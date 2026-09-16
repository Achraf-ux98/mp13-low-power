#include "main.h"
#include "wakeWrap.h"
#include <zephyr/arch/arm/mmu/arm_mem.h>
#include <zephyr/kernel.h>
#include <zephyr/kernel/mm.h>
#include <zephyr/sys/printk.h>
#include <stm32mp13xx_hal_rtc.h>
#include <stm32mp13xx_hal_rtc_ex.h>

#define FSBL_SYSRAM_BASE 0x2FFE0000U
#define FSBL_SYSRAM_SIZE 0x00020000U

extern RTC_HandleTypeDef RTCHandle_BKUP;
extern void (*p_FsblEntryPoint)(void);
extern void arch_mem_map(void *virt, uintptr_t phys, size_t size, uint32_t flags);

void wokeup(void)
{
	uintptr_t fsbl_entry;

	p_FsblEntryPoint = (void *)(HAL_RTCEx_BKUPRead(&RTCHandle_BKUP, RTC_BKP_DR2));
	printk("PM: BKP_DR2 jump target = 0x%08x\n", (uint32_t)p_FsblEntryPoint);
	printk("step: fsbl entrypoint loaded\n\r");

	arch_mem_map((void *)FSBL_SYSRAM_BASE, FSBL_SYSRAM_BASE, FSBL_SYSRAM_SIZE,
		     K_MEM_ARM_NORMAL_NC | K_MEM_PERM_RW | K_MEM_PERM_EXEC);
	printk("step: fsbl sysram mapped\n\r");

	fsbl_entry = (uintptr_t)p_FsblEntryPoint;
	printk("step: jumping to fsbl entry 0x%08x\n\r", (uint32_t)fsbl_entry);

	__asm__ volatile("bx %0" : : "r"(fsbl_entry) : "memory");
	__builtin_unreachable();
}
