/**
  ******************************************************************************
  * @file    bsp_led.c
  * @brief   RGB LED驱动 (STM32F407)
  ******************************************************************************
  */

#include "bsp_led.h"
#include "kconfig.h"
#include "ad8232.h"
#ifdef USE_ECG_SIM
#include "ecg_sim.h"
#endif

void LED_RGB_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Pin = LED1_PIN | LED2_PIN | LED3_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    LED_RGBOFF;
}

#ifdef ENABLE_LED_INDICATOR
void LED_StatusUpdate(void)
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
        LED1_ON;  /* 红灯报警 */
    }
    else
    {
        LED1_OFF;
    }
}
#endif
