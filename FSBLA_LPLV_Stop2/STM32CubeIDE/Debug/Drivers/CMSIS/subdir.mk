################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/local/home/achraftm/STM32CubeMP13/Drivers/CMSIS/Core_A/Source/irq_ctrl_gic.c \
/local/home/achraftm/STM32CubeMP13/Drivers/CMSIS/Device/ST/STM32MP13xx/Source/Templates/mmu_stm32mp13xx.c \
/local/home/achraftm/STM32CubeMP13/Drivers/CMSIS/Device/ST/STM32MP13xx/Source/Templates/system_stm32mp13xx_A7.c 

OBJS += \
./Drivers/CMSIS/irq_ctrl_gic.o \
./Drivers/CMSIS/mmu_stm32mp13xx.o \
./Drivers/CMSIS/system_stm32mp13xx_A7.o 

C_DEPS += \
./Drivers/CMSIS/irq_ctrl_gic.d \
./Drivers/CMSIS/mmu_stm32mp13xx.d \
./Drivers/CMSIS/system_stm32mp13xx_A7.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/CMSIS/irq_ctrl_gic.o: /local/home/achraftm/STM32CubeMP13/Drivers/CMSIS/Core_A/Source/irq_ctrl_gic.c Drivers/CMSIS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-a7 -std=gnu11 -g3 -DSTM32MP135Fxx -DGPT_TABLE_PRESENT -DUSE_STM32MP13XX_DK -DCORE_CA7 -DNO_CACHE_USE -DNO_MMU_USE -DUSE_HAL_DRIVER -DUSE_FULL_ASSERT -DDDR_TYPE_DDR3_4Gb -DUSE_FULL_ASSERT -c -I.. -I../.. -I../../Inc -I../../../../../../../Drivers/CMSIS/Core_A/Include -I../../../../../../../Drivers/CMSIS/Device/ST/STM32MP13xx/Include -I../../../../../../../Drivers/STM32MP13xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32MP13xx_DISCO -I../../../../../../../Drivers/BSP/Components/Common -Og -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=vfpv4-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/CMSIS/mmu_stm32mp13xx.o: /local/home/achraftm/STM32CubeMP13/Drivers/CMSIS/Device/ST/STM32MP13xx/Source/Templates/mmu_stm32mp13xx.c Drivers/CMSIS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-a7 -std=gnu11 -g3 -DSTM32MP135Fxx -DGPT_TABLE_PRESENT -DUSE_STM32MP13XX_DK -DCORE_CA7 -DNO_CACHE_USE -DNO_MMU_USE -DUSE_HAL_DRIVER -DUSE_FULL_ASSERT -DDDR_TYPE_DDR3_4Gb -DUSE_FULL_ASSERT -c -I.. -I../.. -I../../Inc -I../../../../../../../Drivers/CMSIS/Core_A/Include -I../../../../../../../Drivers/CMSIS/Device/ST/STM32MP13xx/Include -I../../../../../../../Drivers/STM32MP13xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32MP13xx_DISCO -I../../../../../../../Drivers/BSP/Components/Common -Og -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=vfpv4-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/CMSIS/system_stm32mp13xx_A7.o: /local/home/achraftm/STM32CubeMP13/Drivers/CMSIS/Device/ST/STM32MP13xx/Source/Templates/system_stm32mp13xx_A7.c Drivers/CMSIS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-a7 -std=gnu11 -g3 -DSTM32MP135Fxx -DGPT_TABLE_PRESENT -DUSE_STM32MP13XX_DK -DCORE_CA7 -DNO_CACHE_USE -DNO_MMU_USE -DUSE_HAL_DRIVER -DUSE_FULL_ASSERT -DDDR_TYPE_DDR3_4Gb -DUSE_FULL_ASSERT -c -I.. -I../.. -I../../Inc -I../../../../../../../Drivers/CMSIS/Core_A/Include -I../../../../../../../Drivers/CMSIS/Device/ST/STM32MP13xx/Include -I../../../../../../../Drivers/STM32MP13xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32MP13xx_DISCO -I../../../../../../../Drivers/BSP/Components/Common -Og -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=vfpv4-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-CMSIS

clean-Drivers-2f-CMSIS:
	-$(RM) ./Drivers/CMSIS/irq_ctrl_gic.cyclo ./Drivers/CMSIS/irq_ctrl_gic.d ./Drivers/CMSIS/irq_ctrl_gic.o ./Drivers/CMSIS/irq_ctrl_gic.su ./Drivers/CMSIS/mmu_stm32mp13xx.cyclo ./Drivers/CMSIS/mmu_stm32mp13xx.d ./Drivers/CMSIS/mmu_stm32mp13xx.o ./Drivers/CMSIS/mmu_stm32mp13xx.su ./Drivers/CMSIS/system_stm32mp13xx_A7.cyclo ./Drivers/CMSIS/system_stm32mp13xx_A7.d ./Drivers/CMSIS/system_stm32mp13xx_A7.o ./Drivers/CMSIS/system_stm32mp13xx_A7.su

.PHONY: clean-Drivers-2f-CMSIS

