#!/usr/bin/env bash
set -euo pipefail

# Generic helper for STM32MP135 Zephyr apps in gettingShitTogether:
# 1) build (west, pristine by default)
# 2) sign image
# 3) flash via CubeProgrammer
# 4) open serial monitor on ttyACM0

APP="cleanTry"
APP_SET=0
BOARD="stm32mp135f_dk"
PORT="/dev/ttyACM0"
BAUD="115200"
PRISTINE=1
OPEN_MONITOR=1
PSCI_EXPERIMENT=0

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"

# Non-interactive shells may miss user-local binaries (west is often here).
export PATH="${HOME}/.local/bin:${PATH}"

PYTHON_BIN="${REPO_ROOT}/.venv/bin/python"
ZEPHYR_BASE_DIR="${REPO_ROOT}/zephyr"
STM32CUBEMP13_ROOT="/local/home/achraftm/STM32CubeMP13"
CUBEPROG_BIN="/local/home/achraftm/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer.sh"
TSP_SRC="${REPO_ROOT}/zephyr/boards/st/stm32mp135f_dk/support/Zephyr.tsv"
EXT_LOADER_DIR="${STM32CUBEMP13_ROOT}/Projects/STM32MP135C-DK/External_Loader/Prebuild_Binaries/SD_Ext_Loader"
SIGN_SCRIPT="${STM32CUBEMP13_ROOT}/Utilities/ImageHeader/Python3/Stm32ImageAddHeader.py"
LAST_APP_FILE="${SCRIPT_DIR}/.last_flash_app"

usage() {
  cat <<'EOF'
Usage:
  ./flash_and_monitor.sh [options]

Options:
  -a, --app <name>        App folder under gettingShitTogether
                          If omitted, reuses last successful app (fallback: cleanTry)
  -b, --board <name>      Zephyr board (default: stm32mp135f_dk)
  -p, --port <device>     Serial device (default: /dev/ttyACM0)
      --baud <rate>       Serial baud (default: 115200)
      --no-pristine       Skip west --pristine
      --no-monitor        Do not open serial monitor after flashing
      --psci-experiment   Apply app-level prj_psci_experiment.conf if present
  -h, --help              Show help

Examples:
  ./flash_and_monitor.sh --app cleanTry
  ./flash_and_monitor.sh --app cleanTry --port /dev/ttyACM0
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -a|--app)
      APP="$2"
      APP_SET=1
      shift 2
      ;;
    -b|--board)
      BOARD="$2"
      shift 2
      ;;
    -p|--port)
      PORT="$2"
      shift 2
      ;;
    --baud)
      BAUD="$2"
      shift 2
      ;;
    --no-pristine)
      PRISTINE=0
      shift
      ;;
    --no-monitor)
      OPEN_MONITOR=0
      shift
      ;;
    --psci-experiment)
      PSCI_EXPERIMENT=1
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage
      exit 2
      ;;
  esac
done

if [[ ${APP_SET} -eq 0 && -f "${LAST_APP_FILE}" ]]; then
  LAST_APP="$(cat "${LAST_APP_FILE}" 2>/dev/null || true)"
  if [[ -n "${LAST_APP}" ]]; then
    APP="${LAST_APP}"
  fi
fi

APP_DIR="${SCRIPT_DIR}/${APP}"
BUILD_DIR="${SCRIPT_DIR}/build/${APP}"
BIN_PATH="${BUILD_DIR}/zephyr/zephyr.bin"
SIGNED_BIN="${EXT_LOADER_DIR}/zephyr_Signed.bin"
TSV_DST="${EXT_LOADER_DIR}/Zephyr.tsv"
EXTRA_CONF_FILE="${APP_DIR}/prj_psci_experiment.conf"

if [[ ! -d "${APP_DIR}" ]]; then
  echo "App directory not found: ${APP_DIR}" >&2
  exit 1
fi

if [[ ! -x "${PYTHON_BIN}" ]]; then
  echo "Python venv not found at ${PYTHON_BIN}" >&2
  echo "Fallback: edit PYTHON_BIN in this script or create .venv in repo root." >&2
  exit 1
fi

if ! "${PYTHON_BIN}" -c "import pykwalify.core" >/dev/null 2>&1; then
  echo "Missing pykwalify in ${PYTHON_BIN}; installing..."
  "${PYTHON_BIN}" -m pip install --quiet pykwalify
fi

if [[ ! -f "${SIGN_SCRIPT}" ]]; then
  echo "Signing script not found: ${SIGN_SCRIPT}" >&2
  exit 1
fi

if [[ ! -x "${CUBEPROG_BIN}" ]]; then
  echo "CubeProgrammer not found or not executable: ${CUBEPROG_BIN}" >&2
  exit 1
fi

if [[ ! -f "${TSP_SRC}" ]]; then
  echo "Zephyr.tsv source not found: ${TSP_SRC}" >&2
  exit 1
fi

USE_CMAKE_FALLBACK=0
if ! command -v west >/dev/null 2>&1; then
  USE_CMAKE_FALLBACK=1
fi

mkdir -p "${BUILD_DIR}"

echo "[1/5] Building ${APP} for ${BOARD}"
if [[ ${USE_CMAKE_FALLBACK} -eq 0 ]]; then
  WEST_EXTRA_ARGS=()
  if [[ ${PSCI_EXPERIMENT} -eq 1 ]]; then
    if [[ ! -f "${EXTRA_CONF_FILE}" ]]; then
      echo "PSCI experiment requested but not found: ${EXTRA_CONF_FILE}" >&2
      exit 1
    fi
    WEST_EXTRA_ARGS+=(-- -DEXTRA_CONF_FILE="${EXTRA_CONF_FILE}")
  fi

  if [[ ${PRISTINE} -eq 1 ]]; then
    west build -b "${BOARD}" "${APP_DIR}" -d "${BUILD_DIR}" --pristine "${WEST_EXTRA_ARGS[@]}"
  else
    west build -b "${BOARD}" "${APP_DIR}" -d "${BUILD_DIR}" "${WEST_EXTRA_ARGS[@]}"
  fi
else
  echo "west not found, using CMake fallback"

  if [[ ! -d "${ZEPHYR_BASE_DIR}" ]]; then
    echo "ZEPHYR_BASE not found: ${ZEPHYR_BASE_DIR}" >&2
    exit 1
  fi

  if [[ ${PRISTINE} -eq 1 ]]; then
    rm -rf "${BUILD_DIR}"
  fi

  EXTRA_CONF_CMAKE_ARGS=()
  if [[ ${PSCI_EXPERIMENT} -eq 1 ]]; then
    if [[ ! -f "${EXTRA_CONF_FILE}" ]]; then
      echo "PSCI experiment requested but not found: ${EXTRA_CONF_FILE}" >&2
      exit 1
    fi
    EXTRA_CONF_CMAKE_ARGS+=( -DEXTRA_CONF_FILE="${EXTRA_CONF_FILE}" )
  fi

  cmake -S "${APP_DIR}" -B "${BUILD_DIR}" -GNinja \
    -DBOARD="${BOARD}" \
    -DZEPHYR_BASE="${ZEPHYR_BASE_DIR}" \
    -DPython3_EXECUTABLE="${PYTHON_BIN}" \
    "${EXTRA_CONF_CMAKE_ARGS[@]}"
  cmake --build "${BUILD_DIR}" -j
fi

if [[ ! -f "${BIN_PATH}" ]]; then
  echo "Build output not found: ${BIN_PATH}" >&2
  exit 1
fi

echo "[2/5] Signing image"
"${PYTHON_BIN}" "${SIGN_SCRIPT}" \
  "${BIN_PATH}" \
  "${SIGNED_BIN}" \
  -bt 10 -la C0000000 -ep C0000000

echo "[3/5] Copying Zephyr.tsv"
cp "${TSP_SRC}" "${TSV_DST}"

echo "[4/5] Flashing"
"${CUBEPROG_BIN}" \
  -c "port=${PORT}" p=even "br=${BAUD}" \
  -d "${TSV_DST}"

echo "[5/5] Done flashing"

echo "${APP}" > "${LAST_APP_FILE}"

if [[ ${OPEN_MONITOR} -eq 1 ]]; then
  echo "Opening serial monitor on ${PORT} @ ${BAUD}"
  echo "Press Ctrl+A then Ctrl+X to exit picocom, or Ctrl+C for fallback mode."

  if command -v picocom >/dev/null 2>&1; then
    exec picocom -b "${BAUD}" "${PORT}"
  fi

  stty -F "${PORT}" "${BAUD}" cs8 -cstopb -parenb -ixon -ixoff -echo
  exec cat "${PORT}"
fi
