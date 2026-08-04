#include "wakeup_sources.h"

#include <errno.h>
#include <stdbool.h>
#include <time.h>

#include <zephyr/arch/arm/irq.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/timeutil.h>

#define WAKEBTN_NODE DT_ALIAS(sw0)
#define WAKELED_NODE DT_ALIAS(led0)
#define RTC_NODE DT_NODELABEL(rtc)
#define WAKE_TEST_MASKED_IRQ 27U

BUILD_ASSERT(DT_NODE_EXISTS(WAKEBTN_NODE), "sw0 alias must be present");
BUILD_ASSERT(DT_NODE_EXISTS(WAKELED_NODE), "led0 alias must be present");
BUILD_ASSERT(DT_NODE_HAS_STATUS(RTC_NODE, okay), "RTC node must be enabled");

static const struct device *const rtc_dev = DEVICE_DT_GET(RTC_NODE);
static const struct gpio_dt_spec wake_button = GPIO_DT_SPEC_GET(WAKEBTN_NODE, gpios);
static const struct gpio_dt_spec wake_led = GPIO_DT_SPEC_GET(WAKELED_NODE, gpios);
static struct gpio_callback wake_button_cb;
static volatile bool wake_button_event;
static volatile bool rtc_alarm_event;
static bool rtc_alarm_armed;

void wakeup_sources_mask_known_irqs(void)
{
	irq_disable(WAKE_TEST_MASKED_IRQ);
}

void wakeup_sources_unmask_known_irqs(void)
{
	irq_enable(WAKE_TEST_MASKED_IRQ);
}

static void rtc_alarm_cb(const struct device *dev, uint16_t id, void *user_data)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(id);
	ARG_UNUSED(user_data);

	printk("rtc wake interrupt\n");
	if (device_is_ready(wake_led.port)) {
		(void)gpio_pin_toggle_dt(&wake_led);
	}

	rtc_alarm_event = true;
	rtc_alarm_armed = false;
}

static void wake_button_cb_fn(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	ARG_UNUSED(dev);
	ARG_UNUSED(cb);
	ARG_UNUSED(pins);

	if (device_is_ready(wake_led.port)) {
		(void)gpio_pin_toggle_dt(&wake_led);
	}

	printk("button wake interrupt\n");
	wake_button_event = true;
}

static int wake_button_prepare(void)
{
	int ret;

	if (!device_is_ready(wake_button.port)) {
		return -ENODEV;
	}

	if (!device_is_ready(wake_led.port)) {
		return -ENODEV;
	}

	ret = gpio_pin_configure_dt(&wake_button, GPIO_INPUT);
	if (ret != 0) {
		return ret;
	}

	ret = gpio_pin_configure_dt(&wake_led, GPIO_OUTPUT_INACTIVE);
	if (ret != 0) {
		return ret;
	}

	gpio_init_callback(&wake_button_cb, wake_button_cb_fn, BIT(wake_button.pin));
	ret = gpio_add_callback(wake_button.port, &wake_button_cb);
	if (ret != 0) {
		return ret;
	}

	ret = gpio_pin_interrupt_configure_dt(&wake_button, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		(void)gpio_remove_callback(wake_button.port, &wake_button_cb);
		return ret;
	}

	return 0;
}

int wakeup_sources_button_set_enabled(bool enabled)
{
	if (!device_is_ready(wake_button.port)) {
		return -ENODEV;
	}

	return gpio_pin_interrupt_configure_dt(&wake_button,
					      enabled ? GPIO_INT_EDGE_TO_ACTIVE : GPIO_INT_DISABLE);
}

static int rtc_alarm_prepare(void)
{
	int ret;

	if (!device_is_ready(rtc_dev)) {
		return -ENODEV;
	}

	ret = rtc_alarm_set_time(rtc_dev, 0, 0, NULL);
	if (ret != 0) {
		return ret;
	}

	ret = rtc_alarm_set_callback(rtc_dev, 0, rtc_alarm_cb, NULL);
	if (ret != 0) {
		return ret;
	}

	wake_button_event = false;
	rtc_alarm_event = false;
	rtc_alarm_armed = false;

	return 0;
}

static int rtc_seed_time(void)
{
	static const struct rtc_time seed_time = {
		.tm_sec = 0,
		.tm_min = 0,
		.tm_hour = 0,
		.tm_mday = 1,
		.tm_mon = 0,
		.tm_year = 126,
		.tm_wday = 4,
		.tm_yday = 0,
		.tm_isdst = 0,
		.tm_nsec = 0,
	};

	return rtc_set_time(rtc_dev, &seed_time);
}

int wakeup_sources_init(void)
{
	int ret;

	ret = wake_button_prepare();
	if (ret != 0) {
		return ret;
	}

	return rtc_alarm_prepare();
}

int wakeup_sources_arm_rtc_alarm(unsigned int seconds)
{
	struct rtc_time now;
	struct rtc_time alarm;
	struct tm alarm_tm;
	int ret;

	if (!device_is_ready(rtc_dev)) {
		return -ENODEV;
	}

	wake_button_event = false;
	rtc_alarm_event = false;

	ret = rtc_alarm_set_callback(rtc_dev, 0, rtc_alarm_cb, NULL);
	if (ret != 0) {
		return ret;
	}

	ret = rtc_get_time(rtc_dev, &now);
	if (ret == -ENODATA) {
		ret = rtc_seed_time();
		if (ret != 0) {
			return ret;
		}

		ret = rtc_get_time(rtc_dev, &now);
	}
	if (ret != 0) {
		return ret;
	}

	int64_t epoch = timeutil_timegm64(rtc_time_to_tm(&now));
	if (epoch < 0) {
		return -EINVAL;
	}

	time_t alarm_epoch = (time_t)(epoch + seconds);
	if (gmtime_r(&alarm_epoch, &alarm_tm) == NULL) {
		return -EINVAL;
	}

	alarm.tm_sec = alarm_tm.tm_sec;
	alarm.tm_min = alarm_tm.tm_min;
	alarm.tm_hour = alarm_tm.tm_hour;
	alarm.tm_mday = alarm_tm.tm_mday;
	alarm.tm_mon = alarm_tm.tm_mon;
	alarm.tm_year = alarm_tm.tm_year;
	alarm.tm_wday = alarm_tm.tm_wday;
	alarm.tm_yday = alarm_tm.tm_yday;
	alarm.tm_isdst = alarm_tm.tm_isdst;
	alarm.tm_nsec = 0;

	ret = rtc_alarm_set_time(rtc_dev, 0,
				 RTC_ALARM_TIME_MASK_SECOND |
				 RTC_ALARM_TIME_MASK_MINUTE |
				 RTC_ALARM_TIME_MASK_HOUR |
				 RTC_ALARM_TIME_MASK_MONTHDAY,
				 &alarm);
	if (ret == 0) {
		rtc_alarm_armed = true;
		rtc_alarm_event = false;
	}

	return ret;
}

void wakeup_sources_clear_rtc_alarm(void)
{
	if (!rtc_alarm_armed) {
		return;
	}

	(void)rtc_alarm_set_time(rtc_dev, 0, 0, NULL);
	rtc_alarm_armed = false;
}

bool wakeup_sources_button_fired(void)
{
	return wake_button_event;
}

bool wakeup_sources_rtc_fired(void)
{
	return rtc_alarm_event;
}

bool wakeup_sources_rtc_pending(void)
{
	if (!device_is_ready(rtc_dev)) {
		return false;
	}

	return rtc_alarm_is_pending(rtc_dev, 0) > 0;
}