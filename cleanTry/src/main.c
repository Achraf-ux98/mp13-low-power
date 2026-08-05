#include <stm32mp135fxx_ca7.h>
#include <stm32mp13xx_ll_pwr.h>
#include <stm32mp13xx_ll_rcc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "wakeup_sources.h"

static void wait_for_wakeup_event(void)
{
	while (!wakeup_sources_button_fired() && !wakeup_sources_rtc_fired()) {
		k_cpu_idle();
	}
}

static void configure_lpstop(void)
{
	PWR->MPUCR |= PWR_MPUCR_CSSF;     /* clear old flags */
	PWR->MPUCR &= ~PWR_MPUCR_PDDS;    /* Stop, not Standby */

	PWR->CR1 |= PWR_CR1_LPDS;         /* low-power Stop family */
	PWR->CR1 &= ~PWR_CR1_STOP2;       /* not Stop2 */
	PWR->CR1 &= ~PWR_CR1_LVDS;        /* exact LP-Stop, not LPLV-Stop */

	RCC->MP_SREQSETR |= RCC_MP_SREQSETR_STPREQ_P0;
}
static void run_wake_test(void)
{
	int ret;

	printk("\n=== Wake source test ===\n");

	ret = wakeup_sources_init();
	printk("wakeup_sources_init() = %d\n", ret);
	if (ret != 0) {
		return;
	}

	ret = wakeup_sources_arm_rtc_alarm(2U);
	printk("wakeup_sources_arm_rtc_alarm(2) = %d\n", ret);
	if (ret != 0) {
		return;
	}

	printk("rtc pending before wait = %u\n",
	       (unsigned int)wakeup_sources_rtc_pending());

	wakeup_sources_mask_known_irqs();
	configure_lpstop();

	printk("entering Zephyr idle\n");
	wait_for_wakeup_event();
	printk("returned from Zephyr idle\n");

	wakeup_sources_unmask_known_irqs();

	printk("rtc pending after wait = %u\n",
	       (unsigned int)wakeup_sources_rtc_pending());
	printk("rtc fired = %u\n",
	       (unsigned int)wakeup_sources_rtc_fired());
	printk("button fired = %u\n",
	       (unsigned int)wakeup_sources_button_fired());

	wakeup_sources_clear_rtc_alarm();

	printk("\n=== Test completed! Continuing normal execution... ===\n");
}

int main(void)
{
	run_wake_test();

	while (1) {
		k_msleep(1000);
	}

	return 0;
}
