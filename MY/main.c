/**
  ******************************************************************************
  * @file    main.c
  * @brief   ECG心电监测系统 - STM32F407ZGT6
  *
  * @details 从ecg_stm(F103)移植，使用240x240 LCD替代OLED
  *          支持设备: AD8232心电, ESP8266 WiFi, RGB LED, 3按键, LCD
  ******************************************************************************
  */

#include "main.h"
#include "stm32f4xx.h"

/* 外设驱动 */
#include "delay.h"
#include "led.h"
#include "usart.h"
#include "lcd_spi_154.h"

/* ECG驱动 */
#include "bsp_led.h"
#include "ad.h"
#include "ad8232.h"
#include "timer.h"
#include "usart2.h"
#include "esp8266.h"
#include "ecg_key.h"
#include "beep.h"

/* 功能模块 */
#ifdef USE_ECG_SIM
#include "ecg_sim.h"
#endif
#include "display.h"
#include "transmit.h"

/*============================ 全局变量 ============================*/

volatile uint8_t display_refresh_flag = 0;

#ifdef ENABLE_DEBUG_PAGE
volatile uint8_t debug_refresh_flag = 0;
static uint32_t loop_start_ms = 0;
uint32_t display_loop_time_ms = 0;
uint32_t display_loop_time_max_ms = 0;
#endif

/*============================ 主函数 ============================*/

int main(void)
{
    /* NVIC优先级分组 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* 基础外设初始化 */
    Delay_Init();
    LED_Init();          /* 板载LED (PC13) */
    LED_RGB_Config();    /* RGB LED (PE2/3/4) */
    Usart_Config();      /* USART1调试串口 */
    SPI_LCD_Init();      /* LCD初始化 */

    /* ESP8266初始化 (USART2, WiFi + MQTT) */
    usart2_init(115200);
    ESP8266_Init();
    Transmit_Init();

    /* 心电图外设 */
    AD_Init();
    AD8232Init();
    Timer3_Init();

#ifdef USE_ECG_SIM
    ECG_Sim_Init(ECG_SIM_BPM);
#endif

    /* 蜂鸣器初始化 */
    Beep_Init();

    /* 按键初始化 */
    ECG_Key_Init();

    /* 清屏准备显示 */
    LCD_SetBackColor(LCD_BLACK);
    LCD_Clear();

    while (1)
    {
#ifdef ENABLE_DEBUG_PAGE
        loop_start_ms = tim3_ms_counter;
#endif

        /* 按键处理 */
        Key_Process();

        /* 页面显示更新 */
        Display_Update();

        /* LED + 蜂鸣器报警 */
#ifdef ENABLE_LED_INDICATOR
        LED_StatusUpdate();
        Beep_AlarmUpdate();
#endif

        /* 数据传输处理 */
        Transmit_Process();

#ifdef ENABLE_DEBUG_PAGE
        display_loop_time_ms = tim3_ms_counter - loop_start_ms;
        if (display_loop_time_ms > display_loop_time_max_ms)
            display_loop_time_max_ms = display_loop_time_ms;
#endif
    }
}

void Error_Handler(void)
{
    while (1);
}
