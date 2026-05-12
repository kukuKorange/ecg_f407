/**
  ******************************************************************************
  * @file    kconfig.h
  * @brief   ECG项目全局配置文件 (STM32F407移植版)
  ******************************************************************************
  */

#ifndef __KCONFIG_H
#define __KCONFIG_H

/*============================================================================*/
/*                              调试配置                                       */
/*============================================================================*/

#define ENABLE_DEBUG_PAGE
// #define ENABLE_UART_DEBUG
#define ENABLE_LED_INDICATOR

/*============================================================================*/
/*                              功能配置                                       */
/*============================================================================*/

// #define USE_MAX30102          /* MAX30102暂未移植，注释掉 */
#define USE_ECG_SIM              /* 启用ECG模拟器（无传感器调试） */
#define ECG_SIM_BPM         72

#define ENABLE_HEARTRATE
// #define ENABLE_SPO2          /* 需USE_MAX30102 */
#define ENABLE_ECG
#define ENABLE_ESP8266_UPLOAD

/*============================================================================*/
/*                              参数配置                                       */
/*============================================================================*/

#define HR_CACHE_NUMS           150
#define PPG_DATA_THRESHOLD      100000
#define HR_ALARM_THRESHOLD_LOW      40
#define HR_ALARM_THRESHOLD_HIGH     120
#define SPO2_ALARM_THRESHOLD    95

/*============================================================================*/
/*                              MQTT报警类型                                    */
/*============================================================================*/

#define ALARM_TYPE_SPO2_LOW     0
#define ALARM_TYPE_HR_HIGH      1
#define ALARM_TYPE_HR_LOW       2
#define ALARM_TYPE_TEMP_ABNORMAL 3
#define ALARM_TYPE_ECG_ABNORMAL 4

/*============================================================================*/
/*                              硬件配置 (STM32F407, 168MHz)                   */
/*============================================================================*/

#define SYSTEM_CLOCK_HZ         168000000
#define TIM3_TIMER_CLOCK_HZ    84000000   /* APB1 timer clock */
#define TIM3_COUNTER_FREQ       100000
#define ECG_SAMPLE_FREQ         200
#define DEBUG_PAGE_REFRESH_FREQ 10

/*============================================================================*/
/*                              LCD配置 (ST7789 240x240)                       */
/*============================================================================*/

#define SCREEN_WIDTH            240
#define SCREEN_HEIGHT           240

/*============================================================================*/
/*                              版本信息                                       */
/*============================================================================*/

#define FIRMWARE_VERSION        "2.0.0-f407"
#define BUILD_DATE              __DATE__
#define BUILD_TIME              __TIME__

#endif /* __KCONFIG_H */
