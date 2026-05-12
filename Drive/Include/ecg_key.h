/**
  ******************************************************************************
  * @file    ecg_key.h
  * @brief   3按键驱动头文件 (PB12/PB13/PB14)
  ******************************************************************************
  */

#ifndef __ECG_KEY_H
#define __ECG_KEY_H

#include <stdint.h>
#include "kconfig.h"

#define PAGE_HEARTRATE    0
#define PAGE_ECG          1

#ifdef ENABLE_DEBUG_PAGE
#define PAGE_DEBUG        2
#define PAGE_MAX          3
#else
#define PAGE_MAX          2
#endif

extern uint8_t current_page;

void ECG_Key_Init(void);
uint8_t Key_GetNum(void);
void Key_Process(void);

#endif
