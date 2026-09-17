#include <zephyr/kernel.h>
#include <zephyr/pm/pm.h>
#include <zephyr/sys/printk.h>
#include <soc.h>

#include <stm32mp13xx_hal_rtc.h>
#include <stm32mp13xx_hal_rtc_ex.h>
#include "src/wakeWrap.h"

#define FSBL_RESUME_SP_REG RTC_BKP_DR4
#define FSBL_RESUME_R4_REG RTC_BKP_DR6
#define FSBL_RESUME_R5_REG RTC_BKP_DR7
#define FSBL_RESUME_R6_REG RTC_BKP_DR8
#define FSBL_RESUME_LR_REG RTC_BKP_DR9

extern RTC_HandleTypeDef RTCHandle_BKUP;

static bool fsbl_resume_pending;

static void fsbl_zephyr_resume_entry(void)
{
	uintptr_t fsbl_resume_sp = HAL_RTCEx_BKUPRead(&RTCHandle_BKUP, FSBL_RESUME_SP_REG);
	uintptr_t fsbl_resume_r4 = HAL_RTCEx_BKUPRead(&RTCHandle_BKUP, FSBL_RESUME_R4_REG);
	uintptr_t fsbl_resume_r5 = HAL_RTCEx_BKUPRead(&RTCHandle_BKUP, FSBL_RESUME_R5_REG);
	uintptr_t fsbl_resume_r6 = HAL_RTCEx_BKUPRead(&RTCHandle_BKUP, FSBL_RESUME_R6_REG);
	uintptr_t fsbl_resume_lr = HAL_RTCEx_BKUPRead(&RTCHandle_BKUP, FSBL_RESUME_LR_REG);

	printk("PM: FSBL returned to Zephyr resume entry\n");
	printk("PM: returning to PM core SP=0x%08x LR=0x%08x\n",
	       (uint32_t)fsbl_resume_sp, (uint32_t)fsbl_resume_lr);
	__asm__ volatile(
		"mov r4, %0\n"
		"mov r5, %1\n"
		"mov r6, %2\n"
		"mov lr, %3\n"
		"add sp, %4, #16\n"
		"bx lr\n"
		:
		: "r"(fsbl_resume_r4), "r"(fsbl_resume_r5), "r"(fsbl_resume_r6),
		  "r"(fsbl_resume_lr), "r"(fsbl_resume_sp)
		: "memory");
	__builtin_unreachable();
}

void pm_state_set(enum pm_state state, uint8_t substate_id)
{
	printk("PM: entered pm_state_set(state=%d, substate=%u)\n", state, substate_id);

	if (fsbl_resume_pending) {
		fsbl_resume_pending = false;
		return;
	}

	/* Fill in return address in backup register 0 for FSBL-A to come back */
	HAL_RTCEx_BKUPWrite(&RTCHandle_BKUP, RTC_BKP_DR0, (uint32_t)fsbl_zephyr_resume_entry);
	fsbl_resume_pending = true;

	{
		uintptr_t fsbl_resume_sp;
		uintptr_t *fsbl_resume_frame;
		uintptr_t fsbl_resume_lr;

		__asm__ volatile("mov %0, sp" : "=r"(fsbl_resume_sp));
		fsbl_resume_frame = (uintptr_t *)fsbl_resume_sp;
		fsbl_resume_lr = fsbl_resume_frame[3];
		HAL_RTCEx_BKUPWrite(&RTCHandle_BKUP, FSBL_RESUME_SP_REG, (uint32_t)fsbl_resume_sp);
		HAL_RTCEx_BKUPWrite(&RTCHandle_BKUP, FSBL_RESUME_R4_REG, (uint32_t)fsbl_resume_frame[0]);
		HAL_RTCEx_BKUPWrite(&RTCHandle_BKUP, FSBL_RESUME_R5_REG, (uint32_t)fsbl_resume_frame[1]);
		HAL_RTCEx_BKUPWrite(&RTCHandle_BKUP, FSBL_RESUME_R6_REG, (uint32_t)fsbl_resume_frame[2]);
		HAL_RTCEx_BKUPWrite(&RTCHandle_BKUP, FSBL_RESUME_LR_REG, (uint32_t)fsbl_resume_lr);
	}

	arch_irq_unlock(0);
	printk("PM: jumping to FSBL low-power entry\n");
	wokeup();
}



void pm_state_exit_post_ops(enum pm_state state, uint8_t substate_id)
{
	printk("PM: entered pm_state_exit_post_ops(state=%d, substate=%u)\n",
	       state, substate_id);

}

/* Initialize STM32 Power */
void stm32_power_init(void)
{
}
