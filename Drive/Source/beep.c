/**
  ******************************************************************************
  * @file    beep.c
  * @brief   蜂鸣器驱动 (STM32F407, PF5)
  * @note    有源蜂鸣器，高电平触发
  ******************************************************************************
  */

#include "beep.h"
#include "kconfig.h"
#include "ad8232.h"
#ifdef USE_ECG_SIM
#include "ecg_sim.h"
#endif

void Beep_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(BEEP_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = BEEP_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(BEEP_PORT, &GPIO_InitStructure);

    BEEP_OFF;
}

#ifdef ENABLE_LED_INDICATOR
void Beep_AlarmUpdate(void)
{
    uint8_t ecg_hr;

#ifdef USE_ECG_SIM
    ecg_hr = ECG_Sim_GetBPM();
#else
    ecg_hr = ECG_GetHeartRate();
#endif

    if (ecg_hr >= HR_ALARM_THRESHOLD_HIGH ||
        (ecg_hr > 0 && ecg_hr <= HR_ALARM_THRESHOLD_LOW))
    {
        BEEP_ON;
    }
    else
    {
        BEEP_OFF;
    }
}
#endif
