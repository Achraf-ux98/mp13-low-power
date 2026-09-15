#include "main.h"
#include <zephyr/sys/printk.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/state.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>

#include "sysram.h"

#define USE_DDR
static volatile bool user_button_pressed;
static const struct gpio_dt_spec user_button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
void BSP_PB_Callback(Button_TypeDef Button);

RTC_HandleTypeDef RTCHandle_BKUP;
void (*p_FsblEntryPoint)(void);


void wokeup(void)
{

{
		uint32_t current_sp;

		__asm volatile ("mov %0, sp" : "=r"(current_sp));
		*(volatile uint32_t *)0x5c00a110U = current_sp;
		printk("LR = 0x%08x\n", current_sp);
	}
        
	

		uint32_t bkp_dr2_value = HAL_RTCEx_BKUPRead(&RTCHandle_BKUP, RTC_BKP_DR2);
	printk("PM: BKP_DR2 raw value = 0x%08x\n", bkp_dr2_value);
	p_FsblEntryPoint = (void *)(HAL_RTCEx_BKUPRead(&RTCHandle_BKUP, RTC_BKP_DR2));
	printk("PM: BKP_DR2 jump target = 0x%08x\n", (uint32_t)p_FsblEntryPoint);
	p_FsblEntryPoint();
        
}

int main(void)
{
	uint32_t count = 0;
	unsigned int irq_key;

	SystemClock_Config();

	HAL_RTC_Init(&RTCHandle_BKUP);

#if (USE_STPMIC1x) && !defined(USE_DDR)
	BSP_PMIC_Init();
	BSP_PMIC_InitRegulators();
#endif

	MX_GPIO_Init();
	


	HAL_RTCEx_BKUPWrite(&RTCHandle_BKUP, RTC_BKP_DR0, (uint32_t)(&main));

	while (count < 6) {
		BSP_LED_Toggle(LED_BLUE);
		k_busy_wait(1000000); /* 100 ms */
		BSP_LED_Toggle(LED_RED);
		count++;
	}

	BSP_LED_Off(LED_BLUE);
	BSP_LED_Off(LED_RED);

	HAL_PWR_EnableWakeUpPinIT(PWR_WAKEUP_PIN1);
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1_LOW);
	IRQ_Enable(MPU_WAKEUP_PIN_IRQn);

		{
		uint32_t current_sp;

		__asm volatile ("mov %0, sp" : "=r"(current_sp));
		*(volatile uint32_t *)0x5c00a110U = current_sp;
	}
        
	
	p_FsblEntryPoint = (void *)(HAL_RTCEx_BKUPRead(&RTCHandle_BKUP, RTC_BKP_DR2));
	printk("PM: BKP_DR2 jump target = 0x%08x\n", (uint32_t)p_FsblEntryPoint);
	sysram_init();
	//irq_key = irq_lock();
	//sysram_run();
	//irq_unlock(irq_key);
	HAL_Delay(10000);
printk("out of sysram_run()\n");

	//	p_FsblEntryPoint();
	
/*
	while (count < 30) {
		BSP_LED_Toggle(LED_BLUE);
		HAL_Delay(30);
		BSP_LED_Toggle(LED_RED);
		count++;
		printf("End of application\n\r");
		if (user_button_pressed) {
			user_button_pressed = false;
			printf("user button interrupt seen\n\r");
		}
	}

*/
	while (1) {
	}
}

void SystemClock_Config(void)
{
#if !defined(USE_DDR)
	HAL_RCC_DeInit();
	RCC_ClkInitTypeDef RCC_ClkInitStructure;
	RCC_OscInitTypeDef RCC_OscInitStructure;

	RCC_OscInitStructure.OscillatorType = (RCC_OSCILLATORTYPE_HSI |
									 RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_CSI |
									 RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE);
	RCC_OscInitStructure.HSIState = RCC_HSI_ON;
	RCC_OscInitStructure.HSEState = RCC_HSE_ON;
	RCC_OscInitStructure.LSEState = RCC_LSE_ON;
	RCC_OscInitStructure.LSIState = RCC_LSI_ON;
	RCC_OscInitStructure.CSIState = RCC_CSI_ON;
	RCC_OscInitStructure.HSICalibrationValue = 0x00;
	RCC_OscInitStructure.CSICalibrationValue = 0x10;
	RCC_OscInitStructure.HSIDivValue = RCC_HSI_DIV1;
	RCC_OscInitStructure.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStructure.PLL.PLLSource = RCC_PLL12SOURCE_HSE;
	RCC_OscInitStructure.PLL.PLLM = 3;
	RCC_OscInitStructure.PLL.PLLN = 81;
	RCC_OscInitStructure.PLL.PLLP = 1;
	RCC_OscInitStructure.PLL.PLLQ = 2;
	RCC_OscInitStructure.PLL.PLLR = 2;
	RCC_OscInitStructure.PLL.PLLFRACV = 0x800;
	RCC_OscInitStructure.PLL.PLLMODE = RCC_PLL_FRACTIONAL;
	RCC_OscInitStructure.PLL2.PLLState = RCC_PLL_ON;
	RCC_OscInitStructure.PLL2.PLLSource = RCC_PLL12SOURCE_HSE;
	RCC_OscInitStructure.PLL2.PLLM = 3;
	RCC_OscInitStructure.PLL2.PLLN = 66;
	RCC_OscInitStructure.PLL2.PLLP = 2;
	RCC_OscInitStructure.PLL2.PLLQ = 2;
	RCC_OscInitStructure.PLL2.PLLR = 1;
	RCC_OscInitStructure.PLL2.PLLFRACV = 0x1400;
	RCC_OscInitStructure.PLL2.PLLMODE = RCC_PLL_FRACTIONAL;
	RCC_OscInitStructure.PLL3.PLLState = RCC_PLL_ON;
	RCC_OscInitStructure.PLL3.PLLSource = RCC_PLL3SOURCE_HSE;
	RCC_OscInitStructure.PLL3.PLLM = 2;
	RCC_OscInitStructure.PLL3.PLLN = 34;
	RCC_OscInitStructure.PLL3.PLLP = 2;
	RCC_OscInitStructure.PLL3.PLLQ = 17;
	RCC_OscInitStructure.PLL3.PLLR = 2;
	RCC_OscInitStructure.PLL3.PLLRGE = RCC_PLL3IFRANGE_1;
	RCC_OscInitStructure.PLL3.PLLFRACV = 0x1a04;
	RCC_OscInitStructure.PLL3.PLLMODE = RCC_PLL_FRACTIONAL;
	RCC_OscInitStructure.PLL4.PLLState = RCC_PLL_ON;
	RCC_OscInitStructure.PLL4.PLLSource = RCC_PLL4SOURCE_HSE;
	RCC_OscInitStructure.PLL4.PLLM = 2;
	RCC_OscInitStructure.PLL4.PLLN = 50;
	RCC_OscInitStructure.PLL4.PLLP = 12;
	RCC_OscInitStructure.PLL4.PLLQ = 60;
	RCC_OscInitStructure.PLL4.PLLR = 6;
	RCC_OscInitStructure.PLL4.PLLRGE = RCC_PLL4IFRANGE_1;
	RCC_OscInitStructure.PLL4.PLLFRACV = 0;
	RCC_OscInitStructure.PLL4.PLLMODE = RCC_PLL_INTEGER;

	SET_BIT(PWR->CR1, PWR_CR1_DBP);
	__HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMHIGH);

	if (HAL_RCC_OscConfig(&RCC_OscInitStructure) != HAL_OK) {
		Error_Handler();
	}

	RCC_ClkInitStructure.ClockType = (RCC_CLOCKTYPE_MPU   | RCC_CLOCKTYPE_ACLK  |
									 RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_PCLK4 |
									 RCC_CLOCKTYPE_PCLK5 | RCC_CLOCKTYPE_PCLK1 |
									 RCC_CLOCKTYPE_PCLK6 |
									 RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK3);

	RCC_ClkInitStructure.MPUInit.MPU_Clock = RCC_MPUSOURCE_PLL1;
	RCC_ClkInitStructure.MPUInit.MPU_Div = RCC_MPU_DIV2;
	RCC_ClkInitStructure.AXISSInit.AXI_Clock = RCC_AXISSOURCE_PLL2;
	RCC_ClkInitStructure.AXISSInit.AXI_Div = RCC_AXI_DIV1;
	RCC_ClkInitStructure.MLAHBInit.MLAHB_Clock = RCC_MLAHBSSOURCE_PLL3;
	RCC_ClkInitStructure.MLAHBInit.MLAHB_Div = RCC_MLAHB_DIV1;
	RCC_ClkInitStructure.APB1_Div = RCC_APB1_DIV2;
	RCC_ClkInitStructure.APB2_Div = RCC_APB2_DIV2;
	RCC_ClkInitStructure.APB3_Div = RCC_APB3_DIV2;
	RCC_ClkInitStructure.APB4_Div = RCC_APB4_DIV2;
	RCC_ClkInitStructure.APB5_Div = RCC_APB5_DIV4;
	RCC_ClkInitStructure.APB6_Div = RCC_APB6_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStructure) != HAL_OK) {
		Error_Handler();
	}
#endif
}

static void MX_GPIO_Init(void)
{
}

void Error_Handler(void)
{
}

void BSP_PB_Callback(Button_TypeDef Button)
{
	ARG_UNUSED(Button);
	user_button_pressed = true;
}
