################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/local/home/achraftm/STM32CubeMP13/Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco.c \
/local/home/achraftm/STM32CubeMP13/Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_bus.c \
/local/home/achraftm/STM32CubeMP13/Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_stpmic1.c 

OBJS += \
./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco.o \
./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_bus.o \
./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_stpmic1.o 

C_DEPS += \
./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco.d \
./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_bus.d \
./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_stpmic1.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco.o: /local/home/achraftm/STM32CubeMP13/Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco.c Drivers/BSP/STM32MP13xx_DISCO/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-a7 -std=gnu11 -g3 -DSTM32MP135Fxx -DGPT_TABLE_PRESENT -DUSE_STM32MP13XX_DK -DCORE_CA7 -DNO_CACHE_USE -DNO_MMU_USE -DUSE_HAL_DRIVER -DUSE_FULL_ASSERT -DDDR_TYPE_DDR3_4Gb -DUSE_FULL_ASSERT -c -I.. -I../.. -I../../Inc -I../../../../../../../Drivers/CMSIS/Core_A/Include -I../../../../../../../Drivers/CMSIS/Device/ST/STM32MP13xx/Include -I../../../../../../../Drivers/STM32MP13xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32MP13xx_DISCO -I../../../../../../../Drivers/BSP/Components/Common -Og -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=vfpv4-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_bus.o: /local/home/achraftm/STM32CubeMP13/Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_bus.c Drivers/BSP/STM32MP13xx_DISCO/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-a7 -std=gnu11 -g3 -DSTM32MP135Fxx -DGPT_TABLE_PRESENT -DUSE_STM32MP13XX_DK -DCORE_CA7 -DNO_CACHE_USE -DNO_MMU_USE -DUSE_HAL_DRIVER -DUSE_FULL_ASSERT -DDDR_TYPE_DDR3_4Gb -DUSE_FULL_ASSERT -c -I.. -I../.. -I../../Inc -I../../../../../../../Drivers/CMSIS/Core_A/Include -I../../../../../../../Drivers/CMSIS/Device/ST/STM32MP13xx/Include -I../../../../../../../Drivers/STM32MP13xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32MP13xx_DISCO -I../../../../../../../Drivers/BSP/Components/Common -Og -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=vfpv4-d16 -mfloat-abi=hard -mthumb -o "$@"
Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_stpmic1.o: /local/home/achraftm/STM32CubeMP13/Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_stpmic1.c Drivers/BSP/STM32MP13xx_DISCO/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-a7 -std=gnu11 -g3 -DSTM32MP135Fxx -DGPT_TABLE_PRESENT -DUSE_STM32MP13XX_DK -DCORE_CA7 -DNO_CACHE_USE -DNO_MMU_USE -DUSE_HAL_DRIVER -DUSE_FULL_ASSERT -DDDR_TYPE_DDR3_4Gb -DUSE_FULL_ASSERT -c -I.. -I../.. -I../../Inc -I../../../../../../../Drivers/CMSIS/Core_A/Include -I../../../../../../../Drivers/CMSIS/Device/ST/STM32MP13xx/Include -I../../../../../../../Drivers/STM32MP13xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/STM32MP13xx_DISCO -I../../../../../../../Drivers/BSP/Components/Common -Og -ffunction-sections -Wall -Wno-strict-aliasing -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=vfpv4-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-STM32MP13xx_DISCO

clean-Drivers-2f-BSP-2f-STM32MP13xx_DISCO:
	-$(RM) ./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco.cyclo ./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco.d ./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco.o ./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco.su ./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_bus.cyclo ./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_bus.d ./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_bus.o ./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_bus.su ./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_stpmic1.cyclo ./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_stpmic1.d ./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_stpmic1.o ./Drivers/BSP/STM32MP13xx_DISCO/stm32mp13xx_disco_stpmic1.su

.PHONY: clean-Drivers-2f-BSP-2f-STM32MP13xx_DISCO

