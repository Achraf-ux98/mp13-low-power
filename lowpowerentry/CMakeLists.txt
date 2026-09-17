cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(lowpowerentry)

set(CUBE_ROOT /local/home/achraftm/zephyrproject/mp13BareMetal/stm32cubemp13-v1-2-0/STM32Cube_FW_MP13_V1.2.0)
set(CUBE_BSP_DIR ${CUBE_ROOT}/Drivers/BSP/STM32MP13xx_DISCO)
set(CUBE_HAL_DIR ${CUBE_ROOT}/Drivers/STM32MP13xx_HAL_Driver/Inc)
set(CUBE_CMSIS_DIR ${CUBE_ROOT}/Drivers/CMSIS/Device/ST/STM32MP13xx/Include)
set(CUBE_BSP_APP_INC ${CUBE_ROOT}/Projects/STM32MP135C-DK/Applications/Low_power_LPLV_Stop2/BSP_BasicTemplates_low_power/Inc)

zephyr_include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CUBE_BSP_DIR}
)

target_sources(app PRIVATE
	src/main.c
	src/wakeWrap.c
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/src/stm32mp13xx_hal_pwr.c
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/src/stm32mp13xx_hal_pwr_ex.c
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/src/stm32mp13xx_hal_gpio.c
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/src/stm32mp13xx_hal_exti.c
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/src/stm32mp13xx_hal_i2c.c
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/src/stm32mp13xx_hal_uart.c
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/src/stm32mp13xx_hal_rtc.c
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/src/stm32mp13xx_hal_rtc_ex.c
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/src/stm32mp13xx_hal_ddr.c
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/src/stm32mp13xx_hal_bsec.c
	${CUBE_ROOT}/Drivers/CMSIS/Core_A/Source/irq_ctrl_gic.c
	${CUBE_BSP_DIR}/stm32mp13xx_disco.c
	${CUBE_BSP_DIR}/stm32mp13xx_disco_bus.c
	${CUBE_BSP_DIR}/stm32mp13xx_disco_stpmic1.c
)

target_include_directories(app PRIVATE
	${CUBE_BSP_APP_INC}
	${CUBE_ROOT}/Drivers/CMSIS/Core_A/Include
	${CUBE_HAL_DIR}
	${CUBE_CMSIS_DIR}
)

target_compile_definitions(app PRIVATE
	USE_STM32MP13XX_DK
	HAL_DDR_MODULE_ENABLED
	DDR_TYPE_DDR3_4Gb
	HAL_BSEC_MODULE_ENABLED
)
