/**
  ******************************************************************************
  * @file    ecg_sim.h
  * @brief   ECG信号模拟器头文件
  ******************************************************************************
  */

#ifndef __ECG_SIM_H
#define __ECG_SIM_H

#include <stdint.h>
#include "kconfig.h"

#ifdef USE_ECG_SIM

void ECG_Sim_Init(uint8_t bpm);
uint16_t ECG_Sim_GetSample(void);
void ECG_Sim_SetBPM(uint8_t bpm);
uint8_t ECG_Sim_GetBPM(void);

#endif
#endif
