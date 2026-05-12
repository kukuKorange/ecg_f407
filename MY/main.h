/**
  ******************************************************************************
  * @file    main.h
  * @brief   主程序头文件 - STM32F407 ECG项目
  ******************************************************************************
  */

#ifndef __MAIN_H
#define __MAIN_H

#include "kconfig.h"

#ifndef USE_STDPERIPH_DRIVER
#define USE_STDPERIPH_DRIVER
#endif

#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"

void Error_Handler(void);

#endif /* __MAIN_H */
