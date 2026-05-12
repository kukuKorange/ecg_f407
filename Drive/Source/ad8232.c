/**
  ******************************************************************************
  * @file    ad8232.c
  * @brief   AD8232心电模块驱动 (STM32F407, LCD版)
  * @note    波形绘制适配240x240 LCD (ST7789)
  ******************************************************************************
  */

#include "stm32f4xx.h"
#include "ad8232.h"
#include "ad.h"
#include "lcd_spi_154.h"
#ifdef USE_ECG_SIM
#include "ecg_sim.h"
#endif

/*============================ 全局变量 ============================*/

uint16_t ecg_data[500] = {0};
uint16_t ecg_index = 1;
uint16_t test = 0;

/*============================ ECG上传缓存（双缓冲） ============================*/

#define ECG_UPLOAD_BUFFER_SIZE  600
#define ECG_UPLOAD_BATCH_SIZE   1

static uint16_t ecg_buffer_a[ECG_UPLOAD_BUFFER_SIZE];
static uint16_t ecg_buffer_b[ECG_UPLOAD_BUFFER_SIZE];
static uint16_t *ecg_fill_buffer = ecg_buffer_a;
static uint16_t *ecg_upload_buffer = ecg_buffer_b;
static uint16_t ecg_fill_idx = 0;
static uint8_t  ecg_buffer_ready = 0;

uint16_t ecg_upload_read_idx = 0;
uint8_t  ecg_upload_active = 0;
uint32_t ecg_upload_timestamp = 0;

/*============================ 私有变量 ============================*/

static uint16_t draw_x = 0;
static float last_filtered = 2048;

/*============================ 心率检测变量 ============================*/

#define ECG_SAMPLE_RATE     200
#define ECG_HR_MIN          30
#define ECG_HR_MAX          220
#define ECG_PEAK_THRESHOLD  2300
#define ECG_REFRACTORY_MS   200
#define ECG_HR_FILTER_SIZE  4

static float ecg_prev_value = 2048;
static float ecg_prev_prev_value = 2048;
static uint32_t ecg_sample_count = 0;
static uint32_t ecg_last_peak_sample = 0;
static uint8_t ecg_heart_rate = 0;
static uint8_t ecg_peak_detected = 0;

static uint8_t ecg_hr_buffer[ECG_HR_FILTER_SIZE] = {0};
static uint8_t ecg_hr_buffer_idx = 0;
static uint8_t ecg_hr_buffer_count = 0;

/*============================ LCD波形区域定义 ============================*/

#define ECG_WAVE_X_START    10
#define ECG_WAVE_X_END      235
#define ECG_WAVE_Y_TOP      45
#define ECG_WAVE_Y_BOTTOM   200
#define ECG_WAVE_WIDTH      (ECG_WAVE_X_END - ECG_WAVE_X_START)
#define ECG_WAVE_HEIGHT     (ECG_WAVE_Y_BOTTOM - ECG_WAVE_Y_TOP)

/*============================ 函数实现 ============================*/

void AD8232Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    /* PE4 (LO+), PE6 (LO-) 浮空输入 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

uint8_t GetConnect(void)
{
    uint8_t LO_plus  = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4);
    uint8_t LO_minus = GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_6);

    return ((LO_plus == 0) && (LO_minus == 0)) ? 1 : 0;
}

uint8_t ECG_GetHeartRate(void)
{
    return ecg_heart_rate;
}

/**
  * @brief  ECG数据采集与绘制 (LCD版)
  * @note   200Hz调用，波形绘制到240x240 LCD
  */
void ECG_SampleAndDraw(void)
{
    uint16_t adc_raw;
    float filtered;

#ifdef USE_ECG_SIM
    adc_raw = ECG_Sim_GetSample();
#else
    adc_raw = AD_GetValue();
#endif

    /* 低通滤波 */
    filtered = last_filtered + ((float)adc_raw - last_filtered) * 0.4f;
    last_filtered = filtered;

    /* 保存到上传缓冲区 */
    ecg_fill_buffer[ecg_fill_idx] = (uint16_t)filtered;
    ecg_fill_idx++;
    ecg_sample_count++;

    /* R波峰值检测 */
    {
        uint32_t refractory_samples = (ECG_REFRACTORY_MS * ECG_SAMPLE_RATE) / 1000;

        if ((ecg_sample_count > refractory_samples + ecg_last_peak_sample) &&
            (ecg_prev_value > ecg_prev_prev_value) &&
            (ecg_prev_value > filtered) &&
            (ecg_prev_value > ECG_PEAK_THRESHOLD))
        {
            ecg_peak_detected = 1;

            if (ecg_last_peak_sample > 0)
            {
                uint32_t rr_samples = ecg_sample_count - ecg_last_peak_sample - 1;
                uint16_t hr = (60 * ECG_SAMPLE_RATE) / rr_samples;

                if (hr >= ECG_HR_MIN && hr <= ECG_HR_MAX)
                {
                    uint16_t sum = 0;
                    uint8_t i;

                    ecg_hr_buffer[ecg_hr_buffer_idx] = (uint8_t)hr;
                    ecg_hr_buffer_idx = (ecg_hr_buffer_idx + 1) % ECG_HR_FILTER_SIZE;
                    if (ecg_hr_buffer_count < ECG_HR_FILTER_SIZE)
                        ecg_hr_buffer_count++;

                    for (i = 0; i < ecg_hr_buffer_count; i++)
                        sum += ecg_hr_buffer[i];
                    ecg_heart_rate = (uint8_t)(sum / ecg_hr_buffer_count);
                }
            }
            ecg_last_peak_sample = ecg_sample_count - 1;
        }
        else
        {
            ecg_peak_detected = 0;
        }

        ecg_prev_prev_value = ecg_prev_value;
        ecg_prev_value = filtered;
    }

    /* 缓冲区满交换 */
    if (ecg_fill_idx >= ECG_UPLOAD_BUFFER_SIZE)
    {
        uint16_t *temp;
        ecg_fill_idx = 0;
        if (!ecg_upload_active)
        {
            temp = ecg_fill_buffer;
            ecg_fill_buffer = ecg_upload_buffer;
            ecg_upload_buffer = temp;
            ecg_buffer_ready = 1;
        }
    }

    /* 绘制波形到LCD */
    if (ecg_index < ECG_WAVE_WIDTH)
    {
        int16_t y_pos;

        /* 映射ADC值到LCD Y坐标 */
        y_pos = ECG_WAVE_Y_BOTTOM - (int16_t)(filtered * ECG_WAVE_HEIGHT / 4096);
        if (y_pos < ECG_WAVE_Y_TOP) y_pos = ECG_WAVE_Y_TOP;
        if (y_pos > ECG_WAVE_Y_BOTTOM) y_pos = ECG_WAVE_Y_BOTTOM;

        ecg_data[ecg_index] = (uint16_t)y_pos;
        ecg_data[0] = ecg_data[1];

        /* 绘制绿色波形线段 */
        LCD_SetColor(LCD_GREEN);
        LCD_DrawLine(ECG_WAVE_X_START + draw_x,     ecg_data[ecg_index - 1],
                     ECG_WAVE_X_START + draw_x + 1, ecg_data[ecg_index]);

        ecg_index++;
        draw_x += 1;
    }
    else
    {
        ECG_ClearAndRedraw();
        ecg_index = 1;
        draw_x = 0;
    }
}

void ECG_ClearAndRedraw(void)
{
    /* 清除波形区域 */
    LCD_SetColor(LCD_BLACK);
    LCD_FillRect(ECG_WAVE_X_START, ECG_WAVE_Y_TOP,
                 ECG_WAVE_WIDTH, ECG_WAVE_HEIGHT + 1);

    /* 重绘坐标轴 */
    LCD_SetColor(LCD_WHITE);
    LCD_DrawLine(ECG_WAVE_X_START, ECG_WAVE_Y_BOTTOM, ECG_WAVE_X_END, ECG_WAVE_Y_BOTTOM);  /* X轴 */
    LCD_DrawLine(ECG_WAVE_X_START, ECG_WAVE_Y_TOP,    ECG_WAVE_X_START, ECG_WAVE_Y_BOTTOM); /* Y轴 */
}

/*============================ ECG上传功能 ============================*/

uint8_t ECG_StartUpload(uint32_t timestamp)
{
    if (!ecg_buffer_ready) return 0;
    ecg_upload_timestamp = timestamp;
    ecg_upload_read_idx = 0;
    ecg_upload_active = 1;
    ecg_buffer_ready = 0;
    return 1;
}

void ECG_StopUpload(void)
{
    ecg_upload_active = 0;
    ecg_upload_read_idx = 0;
}

uint16_t ECG_GetUploadDataCount(void)
{
    return (ecg_upload_active || ecg_buffer_ready) ? ECG_UPLOAD_BUFFER_SIZE : 0;
}

uint8_t ECG_IsDataReady(void)
{
    return ecg_buffer_ready;
}

uint16_t ECG_GetUploadData(uint16_t index)
{
    return (index < ECG_UPLOAD_BUFFER_SIZE) ? ecg_upload_buffer[index] : 0;
}

uint16_t* ECG_GetUploadBuffer(void)
{
    return ecg_upload_buffer;
}

uint16_t ECG_GetUploadBatch(uint16_t *batch_data, uint16_t batch_size)
{
    uint16_t i, count = 0, available;

    if (!ecg_upload_active) return 0;

    available = ECG_UPLOAD_BUFFER_SIZE - ecg_upload_read_idx;
    if (available == 0) { ecg_upload_active = 0; return 0; }

    if (batch_size > available) batch_size = available;
    if (batch_size > ECG_UPLOAD_BATCH_SIZE) batch_size = ECG_UPLOAD_BATCH_SIZE;

    for (i = 0; i < batch_size; i++)
    {
        batch_data[i] = ecg_upload_buffer[ecg_upload_read_idx + i];
        count++;
    }
    ecg_upload_read_idx += count;
    return count;
}

uint8_t ECG_GetUploadProgress(void)
{
    return ecg_upload_active ? (uint8_t)((ecg_upload_read_idx * 100) / ECG_UPLOAD_BUFFER_SIZE) : 100;
}

uint8_t ECG_IsUploadComplete(void)
{
    return !ecg_upload_active;
}
