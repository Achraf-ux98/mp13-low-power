#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "wakeup_sources.h"

static void wake_test_enter_idle_window(void)
{
	//while (!wakeup_sources_rtc_fired() && !wakeup_sources_button_fired()) {
		k_cpu_idle();
	//}
}

static void wakeup_sources_manual_wake_test(void)
{
	int ret;

	printk("\n=== Wake source manual wake test ===\n");

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

	printk("entering Zephyr idle\n");
	wake_test_enter_idle_window();
	printk("returned from Zephyr idle\n");

	wakeup_sources_unmask_known_irqs();

	printk("rtc pending after wait = %u\n",
	       (unsigned int)wakeup_sources_rtc_pending());
	printk("rtc fired = %u\n",
	       (unsigned int)wakeup_sources_rtc_fired());
	printk("button fired = %u\n",
	       (unsigned int)wakeup_sources_button_fired());

	wakeup_sources_clear_rtc_alarm();
}

int main(void)
{
	printk("\n=== Wake source test ===\n");

	wakeup_sources_manual_wake_test();

	printk("\n=== Test completed! Continuing normal execution... ===\n");

	while (1) {
		k_msleep(1000);
	}
}