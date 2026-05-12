/**
  ******************************************************************************
  * @file    ad8232.h
  * @brief   AD8232心电模块驱动头文件
  * @note    LO+=PE0, LO-=PE1, 信号输出→PA1(ADC)
  ******************************************************************************
  */

#ifndef __AD8232_H
#define __AD8232_H

#include <stdint.h>
#include "kconfig.h"

extern uint16_t ecg_data[500];
extern uint16_t ecg_index;
extern uint16_t test;
extern uint8_t  ecg_upload_active;

void AD8232Init(void);
uint8_t GetConnect(void);
uint8_t ECG_GetHeartRate(void);
void ECG_SampleAndDraw(void);
void ECG_ClearAndRedraw(void);

/* ECG上传接口 */
uint8_t ECG_StartUpload(uint32_t timestamp);
uint8_t ECG_IsDataReady(void);
uint16_t ECG_GetUploadData(uint16_t index);
uint16_t* ECG_GetUploadBuffer(void);
void ECG_StopUpload(void);
uint16_t ECG_GetUploadDataCount(void);
uint16_t ECG_GetUploadBatch(uint16_t *batch_data, uint16_t batch_size);
uint8_t ECG_GetUploadProgress(void);
uint8_t ECG_IsUploadComplete(void);

#endif
