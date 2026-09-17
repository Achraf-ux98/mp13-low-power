################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/local/home/achraftm/STM32CubeMP13/Drivers/CMSIS/Device/ST/STM32MP13xx/Source/Templates/gcc/startup_stm32mp135c_ca7.c 

OBJS += \
./Example/STM32CubeIDE/startup_stm32mp135c_ca7.o 

C_DEPS += \
./Example/STM32CubeIDE/startup_stm32mp135c_ca7.d 


# Each subdirectory must supply rules for building sources it contributes
Example/STM32CubeIDE/startup_stm32mp135c_ca7.o: /local/home/achraftm/STM32CubeMP13/Drivers/CMSIS/Device/ST/STM32MP13xx/Source/Templates/gcc/startup_stm32mp135c_ca7.c Example/STM32CubeIDE/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-a7 -std=gnu11 -g3 -DSTM32MP135Fxx -DGPT_TABLE_PRESENT -DUSE_STM32MP13XX_DK -DCORE_CA7 -DNO_CACHE_USE -DNO_MMU_USE -DUSE_HAL_DRIVER -DUSE_FULL_ASSERT -DDDR_TYPE_DDR3_4Gb -DUSE_FULL_ASSERT -c -I.. -I../.. -I../../Inc -I../../../../../../../Drivers/CMSIS/Core_A/Include -I../../../../../../../Drivers/CMSIS/Device/ST/STM32MP13xx/Include -I../../../../../../../Drivers/STM32MP13xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32MP13xx_DISCO -I../../../../../../../Drivers/BSP/Components/Common -Og -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=vfpv4-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Example-2f-STM32CubeIDE

clean-Example-2f-STM32CubeIDE:
	-$(RM) ./Example/STM32CubeIDE/startup_stm32mp135c_ca7.cyclo ./Example/STM32CubeIDE/startup_stm32mp135c_ca7.d ./Example/STM32CubeIDE/startup_stm32mp135c_ca7.o ./Example/STM32CubeIDE/startup_stm32mp135c_ca7.su

.PHONY: clean-Example-2f-STM32CubeIDE

