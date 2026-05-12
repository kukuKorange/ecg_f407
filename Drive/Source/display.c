/**
  ******************************************************************************
  * @file    display.c
  * @brief   LCD显示模块 (240x240 ST7789, 替代OLED)
  ******************************************************************************
  */

#include "display.h"
#include "lcd_spi_154.h"
#include "ad8232.h"
#include "ad.h"
#include "ecg_key.h"
#include "esp8266.h"
#ifdef USE_ECG_SIM
#include "ecg_sim.h"
#endif

static uint8_t last_page = 0xFF;
static uint8_t page0_static_drawn = 0;
static uint16_t last_hr = 0xFFFF;

/*============================================================================*/
/*                              显示更新（主入口）                              */
/*============================================================================*/

void Display_Update(void)
{
    if (current_page != last_page)
    {
        LCD_SetBackColor(LCD_BLACK);
        LCD_Clear();
        last_page = current_page;
        page0_static_drawn = 0;
#ifdef ENABLE_DEBUG_PAGE
        extern uint32_t display_loop_time_max_ms;
        display_loop_time_max_ms = 0;
#endif
    }

    switch (current_page)
    {
        case PAGE_HEARTRATE:
            if (display_refresh_flag)
            {
                display_refresh_flag = 0;
                Display_Page0_HeartRate();
            }
            break;

        case PAGE_ECG:
            Display_Page1_ECG();
            break;

#ifdef ENABLE_DEBUG_PAGE
        case PAGE_DEBUG:
            if (debug_refresh_flag)
            {
                debug_refresh_flag = 0;
                Display_Page2_Debug();
            }
            break;
#endif

        default:
            current_page = PAGE_HEARTRATE;
            break;
    }
}

/*============================================================================*/
/*                              页面0: 心率显示                                */
/*============================================================================*/

static void Display_Page0_DrawStatic(void)
{
    LCD_SetBackColor(LCD_BLACK);
    LCD_SetColor(LCD_WHITE);
    LCD_SetAsciiFont(&ASCII_Font24);
    LCD_DisplayString(30, 10, "Heart Rate (ECG)");

    /* 分隔线 */
    LCD_SetColor(LIGHT_GREY);
    LCD_DrawLine(10, 40, 230, 40);

    /* 心率标签 */
    LCD_SetColor(LIGHT_CYAN);
    LCD_SetAsciiFont(&ASCII_Font24);
    LCD_DisplayString(20, 60, "HR:");

    LCD_SetColor(LIGHT_GREY);
    LCD_SetAsciiFont(&ASCII_Font20);
    LCD_DisplayString(160, 65, "bpm");

#ifdef USE_ECG_SIM
    LCD_SetColor(LIGHT_YELLOW);
    LCD_SetAsciiFont(&ASCII_Font24);
    LCD_DisplayString(20, 110, "Sim:");
    LCD_SetColor(LIGHT_GREY);
    LCD_SetAsciiFont(&ASCII_Font20);
    LCD_DisplayString(160, 115, "bpm");
#endif

    /* 页码 */
    LCD_SetColor(DARK_GREY);
    LCD_SetAsciiFont(&ASCII_Font16);
    LCD_DisplayString(10, 220, "<K1");
#ifdef ENABLE_DEBUG_PAGE
    LCD_DisplayString(100, 220, "1/3");
#else
    LCD_DisplayString(100, 220, "1/2");
#endif
    LCD_DisplayString(200, 220, "K3>");

    page0_static_drawn = 1;
}

void Display_Page0_HeartRate(void)
{
    uint16_t current_hr = (uint16_t)ECG_GetHeartRate();

    if (!page0_static_drawn)
    {
        Display_Page0_DrawStatic();
        last_hr = 0xFFFF;
    }

    /* 心率数值 (大字体) */
    if (current_hr != last_hr)
    {
        last_hr = current_hr;

        /* 清除旧数值区域 */
        LCD_SetColor(LCD_BLACK);
        LCD_FillRect(80, 55, 75, 30);

        /* 显示新数值 */
        if (current_hr > 0)
        {
            if (current_hr >= HR_ALARM_THRESHOLD_HIGH || current_hr <= HR_ALARM_THRESHOLD_LOW)
                LCD_SetColor(LCD_RED);
            else
                LCD_SetColor(LCD_GREEN);
        }
        else
        {
            LCD_SetColor(LIGHT_GREY);
        }

        LCD_SetAsciiFont(&ASCII_Font32);
        LCD_ShowNumMode(Fill_Space);
        LCD_DisplayNumber(80, 55, current_hr, 3);
    }

#ifdef USE_ECG_SIM
    {
        static uint8_t last_sim = 0xFF;
        uint8_t cur_sim = ECG_Sim_GetBPM();
        if (cur_sim != last_sim)
        {
            last_sim = cur_sim;
            LCD_SetColor(LCD_BLACK);
            LCD_FillRect(90, 105, 65, 30);
            LCD_SetColor(LIGHT_YELLOW);
            LCD_SetAsciiFont(&ASCII_Font32);
            LCD_ShowNumMode(Fill_Space);
            LCD_DisplayNumber(90, 105, cur_sim, 3);
        }
    }
#endif
}

/*============================================================================*/
/*                              页面1: 心电图显示                              */
/*============================================================================*/

void Display_Page1_ECG(void)
{
    uint8_t ecg_hr;

    /* 标题 */
    LCD_SetColor(LCD_WHITE);
    LCD_SetAsciiFont(&ASCII_Font20);
    LCD_DisplayString(10, 5, "ECG Monitor");

    /* 坐标轴 (在ECG_ClearAndRedraw中绘制) */

    /* 运行时间 */
    LCD_SetColor(LIGHT_GREY);
    LCD_SetAsciiFont(&ASCII_Font16);
    LCD_ShowNumMode(Fill_Space);
    LCD_DisplayNumber(130, 8, test, 5);
    LCD_DisplayString(175, 8, "s");

    /* 心率 */
    LCD_SetColor(LCD_GREEN);
#ifdef USE_ECG_SIM
    LCD_DisplayNumber(200, 8, ECG_Sim_GetBPM(), 3);
#else
    ecg_hr = ECG_GetHeartRate();
    LCD_DisplayNumber(200, 8, ecg_hr, 3);
#endif

    /* 页码 */
    LCD_SetColor(DARK_GREY);
    LCD_SetAsciiFont(&ASCII_Font12);
    LCD_DisplayString(10, 228, "<K1");
#ifdef ENABLE_DEBUG_PAGE
    LCD_DisplayString(110, 228, "2/3");
#else
    LCD_DisplayString(110, 228, "2/2");
#endif
    LCD_DisplayString(210, 228, "K3>");

    /* 底部提示 */
    LCD_SetColor(DARK_GREY);
    LCD_DisplayString(60, 210, "K2:Upload ECG");
}

/*============================================================================*/
/*                              页面2: 调试页面                                */
/*============================================================================*/

#ifdef ENABLE_DEBUG_PAGE
void Display_Page2_Debug(void)
{
    uint16_t adc_raw = AD_GetValue();

    LCD_SetBackColor(LCD_BLACK);
    LCD_SetAsciiFont(&ASCII_Font20);

    /* 标题 */
    LCD_SetColor(LCD_WHITE);
    LCD_DisplayString(10, 5, "[DEBUG]");

    LCD_SetColor(LIGHT_GREY);
    LCD_DrawLine(10, 28, 230, 28);

    LCD_SetAsciiFont(&ASCII_Font16);
    LCD_ShowNumMode(Fill_Space);

    /* 运行时间 */
    LCD_SetColor(LIGHT_CYAN);
    LCD_DisplayString(10, 35, "Time:");
    LCD_DisplayNumber(70, 35, test, 5);
    LCD_DisplayString(120, 35, "s");

    /* 心率 */
    LCD_DisplayString(150, 35, "HR:");
    LCD_DisplayNumber(190, 35, ECG_GetHeartRate(), 3);

    /* 循环时间 */
    LCD_SetColor(LIGHT_YELLOW);
    LCD_DisplayString(10, 60, "Loop:");
    LCD_DisplayNumber(70, 60, display_loop_time_ms, 6);
    LCD_DisplayString(130, 60, "x10us");

    LCD_DisplayString(10, 82, "Max:");
    LCD_DisplayNumber(70, 82, display_loop_time_max_ms, 6);
    LCD_DisplayString(130, 82, "x10us");

    /* ADC */
    LCD_SetColor(LIGHT_GREEN);
    LCD_DisplayString(10, 108, "ADC:");
    LCD_DisplayNumber(60, 108, adc_raw, 4);

#ifdef USE_ECG_SIM
    LCD_DisplayString(120, 108, "Sim:");
    LCD_DisplayNumber(170, 108, ECG_Sim_GetBPM(), 3);
    LCD_DisplayString(200, 108, "bpm");
#endif

    /* MAC地址 */
    LCD_SetColor(LIGHT_GREY);
    LCD_DisplayString(10, 135, "MAC:");
    LCD_DisplayString(50, 135, esp8266_mac);

    /* 版本 */
    LCD_SetColor(DARK_GREY);
    LCD_SetAsciiFont(&ASCII_Font12);
    LCD_DisplayString(10, 160, "FW:" FIRMWARE_VERSION);

    /* 页码 */
    LCD_DisplayString(10, 228, "<K1");
    LCD_DisplayString(110, 228, "3/3");
    LCD_DisplayString(210, 228, "K3>");
}
#endif
