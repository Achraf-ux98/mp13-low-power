################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Example/STM32CubeIDE/subdir.mk
-include Drivers/STM32MP13xx_HAL_Driver/subdir.mk
-include Drivers/CMSIS/subdir.mk
-include Drivers/BSP/STM32MP13xx_DISCO/subdir.mk
-include Application/User/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := FSBLA_LPLV_Stop2
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
FSBLA_LPLV_Stop2.elf \

MAP_FILES += \
FSBLA_LPLV_Stop2.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
FSBLA_LPLV_Stop2.list \

OBJCOPY_BIN += \
FSBLA_LPLV_Stop2.bin \


# All Target
all:
	+@$(MAKE) --no-print-directory main-build && $(MAKE) --no-print-directory post-build

# Main-build Target
main-build: FSBLA_LPLV_Stop2.elf secondary-outputs

# Tool invocations
FSBLA_LPLV_Stop2.elf FSBLA_LPLV_Stop2.map: $(OBJS) $(USER_OBJS) ../stm32mp13xx_a7_sysram.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "FSBLA_LPLV_Stop2.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-a7 -T"../stm32mp13xx_a7_sysram.ld" --specs=nosys.specs -Wl,-Map="FSBLA_LPLV_Stop2.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=vfpv4-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

FSBLA_LPLV_Stop2.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "FSBLA_LPLV_Stop2.list"
	@echo 'Finished building: $@'
	@echo ' '

FSBLA_LPLV_Stop2.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "FSBLA_LPLV_Stop2.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) FSBLA_LPLV_Stop2.bin FSBLA_LPLV_Stop2.elf FSBLA_LPLV_Stop2.list FSBLA_LPLV_Stop2.map default.size.stdout
	-@echo ' '

post-build:
	arm-none-eabi-objcopy -O binary "FSBLA_LPLV_Stop2.elf" "FSBLA_LPLV_Stop2.bin"
	"../postbuild.sh" "." "FSBLA_LPLV_Stop2.bin" "FSBLA_LPLV_Stop2_Signed.bin" "-bt 10" "-ep 2FFE0000"
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified post-build

-include ../makefile.targets
