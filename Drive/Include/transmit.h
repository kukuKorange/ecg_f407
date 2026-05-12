/**
  ******************************************************************************
  * @file    transmit.h
  * @brief   数据传输模块头文件
  ******************************************************************************
  */

#ifndef __TRANSMIT_H
#define __TRANSMIT_H

#include <stdint.h>
#include "kconfig.h"

#define ENABLE_MQTT_TRANSMIT
#define TRANSMIT_INTERVAL_SEC       5
#define ALARM_CHECK_INTERVAL_SEC    10

extern volatile uint8_t transmit_flag;
extern volatile uint8_t alarm_check_flag;
extern volatile uint8_t ecg_upload_flag;

void Transmit_Init(void);
void Transmit_Process(void);
void Transmit_SendVitalSign(void);
void Transmit_CheckAlarm(void);
void Transmit_TimerCallback(void);
uint8_t Transmit_StartECGUpload(uint32_t timestamp);
uint8_t Transmit_GetECGProgress(void);
uint8_t Transmit_IsECGUploadComplete(void);

#endif
