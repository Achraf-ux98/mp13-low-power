# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(lplvstop2)

target_sources(app PRIVATE src/main.c)

target_sources(app PRIVATE
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/src/stm32mp13xx_hal_pwr.c
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/src/stm32mp13xx_hal_pwr_ex.c
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/src/stm32mp13xx_hal_gpio.c
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/src/stm32mp13xx_hal_i2c.c
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/src/stm32mp13xx_hal_uart.c
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/src/stm32mp13xx_hal_rtc.c
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/src/stm32mp13xx_hal_rtc_ex.c
	${ZEPHYR_BASE}/../mp13BareMetal/stm32cubemp13-v1-2-0/STM32Cube_FW_MP13_V1.2.0/Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_bus.c
	${ZEPHYR_BASE}/../mp13BareMetal/stm32cubemp13-v1-2-0/STM32Cube_FW_MP13_V1.2.0/Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco.c
	${ZEPHYR_BASE}/../mp13BareMetal/stm32cubemp13-v1-2-0/STM32Cube_FW_MP13_V1.2.0/Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_stpmic1.c
	${ZEPHYR_BASE}/../mp13BareMetal/stm32cubemp13-v1-2-0/STM32Cube_FW_MP13_V1.2.0/Drivers/CMSIS/Core_A/Source/irq_ctrl_gic.c
)

target_include_directories(app PRIVATE
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/soc
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/stm32mp13xx/drivers/include
	${ZEPHYR_BASE}/../modules/hal/stm32/stm32cube/common_ll/include
	${ZEPHYR_BASE}/../mp13BareMetal/stm32cubemp13-v1-2-0/STM32Cube_FW_MP13_V1.2.0/Drivers/BSP/STM32MP13xx_DISCO
	${ZEPHYR_BASE}/../mp13BareMetal/stm32cubemp13-v1-2-0/STM32Cube_FW_MP13_V1.2.0/Drivers/BSP/Components
	${ZEPHYR_BASE}/../mp13BareMetal/stm32cubemp13-v1-2-0/STM32Cube_FW_MP13_V1.2.0/Projects/STM32MP135C-DK/Applications/Low_power_LPLV_Stop2/BSP_BasicTemplates_low_power/Inc
)
