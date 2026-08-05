cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(cleanTry)

target_include_directories(app PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/drivers
  ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/include
)

target_sources(app PRIVATE
  src/main.c
  src/sysram.c
  src/wakeup_sources.c
  drivers/pmic_lp.c
  drivers/stpmic1.c
)

zephyr_linker_sources(SECTIONS sysram.ld)
