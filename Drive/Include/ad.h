/**
  ******************************************************************************
  * @file    ad.h
  * @brief   ADC驱动头文件 (STM32F407)
  * @note    PA1 (ADC1_Channel_1) 用于AD8232心电信号采集
  ******************************************************************************
  */

#ifndef __AD_H
#define __AD_H

#include <stdint.h>

void AD_Init(void);
uint16_t AD_GetValue(void);

#endif
