#include "main.h"
#include <zephyr/sys/printk.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/state.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include "src/wakeWrap.h"

RTC_HandleTypeDef RTCHandle_BKUP;


int main(void)
{
	printk("Hello World - Landed in user application in DDR\n\r");
	uint32_t count = 0;

	HAL_RTC_Init(&RTCHandle_BKUP);
	printk("step: backup register written\n\r");
	HAL_RTCEx_BKUPWrite(&RTCHandle_BKUP, RTC_BKP_DR0,(uint32_t) (&main));

	while (count < 6) {
		BSP_LED_Toggle(LED_BLUE);
		k_busy_wait(100000); /* 100 ms */
		BSP_LED_Toggle(LED_RED);
		count++;
	}

	BSP_LED_Off(LED_BLUE);
	BSP_LED_Off(LED_RED);

	HAL_PWR_EnableWakeUpPinIT(PWR_WAKEUP_PIN1);
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1_LOW);
	IRQ_Enable(MPU_WAKEUP_PIN_IRQn);
	wokeup();
	HAL_Delay(1000);

	while (1) {
		BSP_LED_Toggle(LED_BLUE);
		k_busy_wait(100000); /* 100 ms */
		BSP_LED_Toggle(LED_RED);
	}
}



