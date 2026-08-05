#include <stm32mp135fxx_ca7.h>
#include <stm32mp13xx_ll_pwr.h>
#include <stm32mp13xx_ll_rcc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "sysram.h"
#include "wakeup_sources.h"
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


	printk("entering SYSRAM DDR run\n");
	sysram_init();
	sysram_run();

	printk("returned from SYSRAM DDR run\n");

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
