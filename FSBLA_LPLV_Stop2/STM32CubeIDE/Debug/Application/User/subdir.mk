################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/local/home/achraftm/STM32CubeMP13/Projects/STM32MP135C-DK/Applications/Low_power_LPLV_Stop2/FSBLA_LPLV_Stop2/Src/main.c \
/local/home/achraftm/STM32CubeMP13/Projects/STM32MP135C-DK/Applications/Low_power_LPLV_Stop2/FSBLA_LPLV_Stop2/Src/stm32mp13xx_hal_msp.c \
/local/home/achraftm/STM32CubeMP13/Projects/STM32MP135C-DK/Applications/Low_power_LPLV_Stop2/FSBLA_LPLV_Stop2/Src/stm32mp13xx_it.c 

OBJS += \
./Application/User/main.o \
./Application/User/stm32mp13xx_hal_msp.o \
./Application/User/stm32mp13xx_it.o 

C_DEPS += \
./Application/User/main.d \
./Application/User/stm32mp13xx_hal_msp.d \
./Application/User/stm32mp13xx_it.d 


# Each subdirectory must supply rules for building sources it contributes
Application/User/main.o: /local/home/achraftm/STM32CubeMP13/Projects/STM32MP135C-DK/Applications/Low_power_LPLV_Stop2/FSBLA_LPLV_Stop2/Src/main.c Application/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-a7 -std=gnu11 -g3 -DSTM32MP135Fxx -DGPT_TABLE_PRESENT -DUSE_STM32MP13XX_DK -DCORE_CA7 -DNO_CACHE_USE -DNO_MMU_USE -DUSE_HAL_DRIVER -DUSE_FULL_ASSERT -DDDR_TYPE_DDR3_4Gb -DUSE_FULL_ASSERT -c -I.. -I../.. -I../../Inc -I../../../../../../../Drivers/CMSIS/Core_A/Include -I../../../../../../../Drivers/CMSIS/Device/ST/STM32MP13xx/Include -I../../../../../../../Drivers/STM32MP13xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32MP13xx_DISCO -I../../../../../../../Drivers/BSP/Components/Common -Og -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=vfpv4-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/stm32mp13xx_hal_msp.o: /local/home/achraftm/STM32CubeMP13/Projects/STM32MP135C-DK/Applications/Low_power_LPLV_Stop2/FSBLA_LPLV_Stop2/Src/stm32mp13xx_hal_msp.c Application/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-a7 -std=gnu11 -g3 -DSTM32MP135Fxx -DGPT_TABLE_PRESENT -DUSE_STM32MP13XX_DK -DCORE_CA7 -DNO_CACHE_USE -DNO_MMU_USE -DUSE_HAL_DRIVER -DUSE_FULL_ASSERT -DDDR_TYPE_DDR3_4Gb -DUSE_FULL_ASSERT -c -I.. -I../.. -I../../Inc -I../../../../../../../Drivers/CMSIS/Core_A/Include -I../../../../../../../Drivers/CMSIS/Device/ST/STM32MP13xx/Include -I../../../../../../../Drivers/STM32MP13xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32MP13xx_DISCO -I../../../../../../../Drivers/BSP/Components/Common -Og -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=vfpv4-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/User/stm32mp13xx_it.o: /local/home/achraftm/STM32CubeMP13/Projects/STM32MP135C-DK/Applications/Low_power_LPLV_Stop2/FSBLA_LPLV_Stop2/Src/stm32mp13xx_it.c Application/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-a7 -std=gnu11 -g3 -DSTM32MP135Fxx -DGPT_TABLE_PRESENT -DUSE_STM32MP13XX_DK -DCORE_CA7 -DNO_CACHE_USE -DNO_MMU_USE -DUSE_HAL_DRIVER -DUSE_FULL_ASSERT -DDDR_TYPE_DDR3_4Gb -DUSE_FULL_ASSERT -c -I.. -I../.. -I../../Inc -I../../../../../../../Drivers/CMSIS/Core_A/Include -I../../../../../../../Drivers/CMSIS/Device/ST/STM32MP13xx/Include -I../../../../../../../Drivers/STM32MP13xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32MP13xx_DISCO -I../../../../../../../Drivers/BSP/Components/Common -Og -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=vfpv4-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Application-2f-User

clean-Application-2f-User:
	-$(RM) ./Application/User/main.cyclo ./Application/User/main.d ./Application/User/main.o ./Application/User/main.su ./Application/User/stm32mp13xx_hal_msp.cyclo ./Application/User/stm32mp13xx_hal_msp.d ./Application/User/stm32mp13xx_hal_msp.o ./Application/User/stm32mp13xx_hal_msp.su ./Application/User/stm32mp13xx_it.cyclo ./Application/User/stm32mp13xx_it.d ./Application/User/stm32mp13xx_it.o ./Application/User/stm32mp13xx_it.su

.PHONY: clean-Application-2f-User

