/**
  ******************************************************************************
  * @file    transmit.c
  * @brief   数据传输模块 (MQTT via ESP8266)
  ******************************************************************************
  */

#include "transmit.h"
#include "esp8266.h"
#include "ad8232.h"
#ifdef USE_ECG_SIM
#include "ecg_sim.h"
#endif

static uint16_t transmit_counter = 0;
static uint16_t alarm_counter = 0;
static uint16_t ecg_upload_count = 0;
static uint32_t ecg_batch_timestamp = 0;

volatile uint8_t transmit_flag = 0;
volatile uint8_t alarm_check_flag = 0;
volatile uint8_t ecg_upload_flag = 0;

static void Transmit_ECGUploadProcess(void);

void Transmit_Init(void)
{
    transmit_counter = 0;
    alarm_counter = 0;
    transmit_flag = 0;
    alarm_check_flag = 0;
}

void Transmit_Process(void)
{
#ifdef ENABLE_MQTT_TRANSMIT
    if (transmit_flag)
    {
        transmit_flag = 0;
        Transmit_SendVitalSign();
    }

    if (alarm_check_flag)
    {
        alarm_check_flag = 0;
    }

    Transmit_ECGUploadProcess();
#else
    transmit_flag = 0;
    alarm_check_flag = 0;
#endif
}

void Transmit_SendVitalSign(void)
{
    uint8_t ecg_hr;
#ifdef USE_ECG_SIM
    ecg_hr = ECG_Sim_GetBPM();
#else
    ecg_hr = ECG_GetHeartRate();
#endif
    ESP8266_SendToTopic(MQTT_TOPIC_HEARTRATE, (uint16_t)ecg_hr);
}

void Transmit_CheckAlarm(void)
{
    uint8_t ecg_hr;
#ifdef USE_ECG_SIM
    ecg_hr = ECG_Sim_GetBPM();
#else
    ecg_hr = ECG_GetHeartRate();
#endif

    if (ecg_hr > HR_ALARM_THRESHOLD_HIGH)
        ESP8266_Send("alarm", ALARM_TYPE_HR_HIGH);
    if (ecg_hr > 0 && ecg_hr < HR_ALARM_THRESHOLD_LOW)
        ESP8266_Send("alarm", ALARM_TYPE_HR_LOW);
}

void Transmit_TimerCallback(void)
{
    transmit_counter++;
    if (transmit_counter >= TRANSMIT_INTERVAL_SEC)
    {
        transmit_counter = 0;
        transmit_flag = 1;
    }

    alarm_counter++;
    if (alarm_counter >= ALARM_CHECK_INTERVAL_SEC)
    {
        alarm_counter = 0;
        alarm_check_flag = 1;
    }
}

uint8_t Transmit_StartECGUpload(uint32_t timestamp)
{
    if (ECG_StartUpload(timestamp))
    {
        ecg_batch_timestamp = timestamp;
        ecg_upload_count = 0;
        return 1;
    }
    return 0;
}

static void Transmit_ECGUploadProcess(void)
{
    uint16_t *buffer;

    if (ECG_IsUploadComplete()) return;
    if (!ecg_upload_flag) return;
    ecg_upload_flag = 0;

    if (ecg_upload_count >= 600)
    {
        ECG_StopUpload();
        return;
    }

    buffer = ECG_GetUploadBuffer();
    ESP8266_SendECGBatch(ecg_batch_timestamp, &buffer[ecg_upload_count], 1);
    ecg_upload_count++;
    ecg_batch_timestamp += 5;
}

uint8_t Transmit_GetECGProgress(void)
{
    return ECG_IsUploadComplete() ? 100 : (uint8_t)((ecg_upload_count * 100) / 600);
}

uint8_t Transmit_IsECGUploadComplete(void)
{
    return ECG_IsUploadComplete();
}
