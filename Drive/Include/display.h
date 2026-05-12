/**
  ******************************************************************************
  * @file    display.h
  * @brief   LCD显示模块头文件 (240x240 ST7789)
  ******************************************************************************
  */

#ifndef __DISPLAY_H
#define __DISPLAY_H

#include <stdint.h>
#include "kconfig.h"

extern uint8_t current_page;
extern volatile uint8_t display_refresh_flag;

#ifdef ENABLE_DEBUG_PAGE
extern uint32_t display_loop_time_ms;
extern uint32_t display_loop_time_max_ms;
extern volatile uint8_t debug_refresh_flag;
#endif

void Display_Update(void);
void Display_Page0_HeartRate(void);
void Display_Page1_ECG(void);

#ifdef ENABLE_DEBUG_PAGE
void Display_Page2_Debug(void);
#endif

#endif
