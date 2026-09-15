/* USER CODE BEGIN Header */
/**
	******************************************************************************
	* @file    main.h
	* @brief   Header for main.c file.
	******************************************************************************
	*/
/* USER CODE END Header */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32mp13xx_hal.h"

/* USER CODE BEGIN Includes */
#include "stm32mp13xx_disco.h"
#include "stm32mp13xx_disco_stpmic1.h"
/* USER CODE END Includes */

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
