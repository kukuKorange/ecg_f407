/**
  ******************************************************************************
  * @file    timer.c
  * @brief   TIM3定时器驱动 (STM32F407)
  *
  * @details TIM3 APB1 timer clock = 84MHz
  *          PSC=84-1, ARR=10-1 → 84MHz/84/10 = 100kHz中断
  *          ECG采样: 每500次 = 200Hz
  ******************************************************************************
  */

#include "stm32f4xx.h"
#include "kconfig.h"
#include "timer.h"
#include "ad8232.h"
#include "ecg_key.h"
#include "transmit.h"

static uint32_t tim3_counter = 0;
volatile uint32_t tim3_ms_counter = 0;

void Timer3_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    TIM_InternalClockConfig(TIM3);

    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 10 - 1;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 84 - 1;  /* 84MHz/84=1MHz, /10=100kHz */
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);

    TIM_ClearFlag(TIM3, TIM_FLAG_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM3, ENABLE);
}

/* 外部变量 */
extern volatile uint8_t display_refresh_flag;
extern volatile uint8_t ecg_upload_flag;
#ifdef ENABLE_DEBUG_PAGE
extern volatile uint8_t debug_refresh_flag;
#endif

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
    {
        tim3_counter++;
        tim3_ms_counter++;

        /* 1Hz: 秒计数器 + 传输回调 */
        if (tim3_counter >= 100000)
        {
            test++;
            tim3_counter = 0;
            Transmit_TimerCallback();
        }

        /* 200Hz: ECG采样与绘制（仅ECG页面） */
        if ((tim3_counter % 500 == 0) && (current_page == PAGE_ECG))
        {
            ECG_SampleAndDraw();
        }

        /* 5Hz: 显示刷新 */
        if (tim3_counter % 20000 == 0)
        {
            display_refresh_flag = 1;
        }

        /* 100Hz: ECG上传触发 */
        if (tim3_counter % 1000 == 0)
        {
            ecg_upload_flag = 1;
        }

#ifdef ENABLE_DEBUG_PAGE
        /* 10Hz: 调试页面刷新 */
        if (tim3_counter % 10000 == 0)
        {
            debug_refresh_flag = 1;
        }
#endif

        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}
