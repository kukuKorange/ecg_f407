/**
  ******************************************************************************
  * @file    esp8266.c
  * @brief   ESP8266 WiFi模块驱动 (STM32F407, LCD版)
  ******************************************************************************
  */

#include "stm32f4xx.h"
#include "esp8266.h"
#include "usart2.h"
#include "delay.h"
#include "lcd_spi_154.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

unsigned char Property_Data[5];
char esp8266_mac[18] = "--:--:--:--:--:--";

static uint8_t prv_is_hex(char c)
{
    return ((c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'f') ||
            (c >= 'A' && c <= 'F'));
}

/**
  * @brief  发送AT+RST并解析MAC地址 (LCD显示调试信息)
  */
static void prv_RST_ParseMAC(void)
{
    uint16_t acc_len, null_cnt, j;
    char *p;
    uint8_t i;

    USART2_RX_STA = 0;
    usart2_raw_mode = 1;
    usart2_raw_len = 0;

    /* LCD显示RST状态 */
    LCD_SetBackColor(LCD_BLACK);
    LCD_Clear();
    LCD_SetColor(LCD_WHITE);
    LCD_SetAsciiFont(&ASCII_Font24);
    LCD_DisplayString(10, 10, "ESP8266 Init");
    LCD_SetAsciiFont(&ASCII_Font16);
    LCD_DisplayString(10, 50, "AT+RST sent, waiting 3s...");

    u2_printf("AT+RST\r\n");
    Delay_ms(3000);

    usart2_raw_mode = 0;
    acc_len = usart2_raw_len;
    if (acc_len < USART2_MAX_RECV_LEN)
        USART2_RX_BUF[acc_len] = 0;
    else
        USART2_RX_BUF[USART2_MAX_RECV_LEN - 1] = 0;

    USART2_RX_STA = 0;

    /* 替换嵌入的NULL字节 */
    null_cnt = 0;
    for (j = 0; j < acc_len; j++)
    {
        if (USART2_RX_BUF[j] == 0x00)
        {
            USART2_RX_BUF[j] = 0xFF;
            null_cnt++;
        }
    }

    LCD_SetColor(LIGHT_CYAN);
    LCD_DisplayString(10, 80, "Searching MAC...");

    p = strstr((char *)USART2_RX_BUF, "wifi_mac:");
    if (p != NULL)
    {
        p += 9;
        for (i = 0; i < 12; i++)
        {
            if (!prv_is_hex(p[i])) break;
        }

        if (i == 12)
        {
            for (i = 0; i < 6; i++)
            {
                esp8266_mac[i * 3]     = p[i * 2]     | 0x20;
                esp8266_mac[i * 3 + 1] = p[i * 2 + 1] | 0x20;
                esp8266_mac[i * 3 + 2] = (i < 5) ? ':' : '\0';
            }
            esp8266_mac[17] = '\0';

            LCD_SetColor(LCD_GREEN);
            LCD_DisplayString(10, 110, "MAC:");
            LCD_DisplayString(60, 110, esp8266_mac);
        }
    }
    else
    {
        LCD_SetColor(LCD_RED);
        LCD_DisplayString(10, 110, "MAC not found");
    }

    Delay_ms(1500);
}

void ESP8266_Init(void)
{
    uint8_t retry;

    Delay_ms(2000);

    for (retry = 0; retry < 3; retry++)
    {
        if (esp8266_send_cmd("AT", "OK", 50) == 0) break;
        Delay_ms(500);
    }

    esp8266_send_cmd("AT+CWMODE=1", "OK", 50);
    prv_RST_ParseMAC();
    Delay_ms(2000);

    /* LCD显示WiFi连接状态 */
    LCD_SetColor(LIGHT_YELLOW);
    LCD_SetAsciiFont(&ASCII_Font16);
    LCD_DisplayString(10, 140, "Connecting WiFi...");

    esp8266_send_cmd("AT+CWJAP=\"" WIFI_NAME "\",\"" WIFI_PASSWORD "\"", "GOT IP", 1500);
    Delay_ms(2000);

    LCD_DisplayString(10, 165, "Connecting MQTT...");
    esp8266_send_cmd(MQTT_USERCFG, "OK", 100);

#if (MQTT_USE_ALIYUN == 1)
    esp8266_send_cmd(MQTT_CLIENTID, "OK", 100);
#endif

    esp8266_send_cmd(MQTT_CONN, "OK", 300);

    LCD_SetColor(LCD_GREEN);
    LCD_DisplayString(10, 190, "Init Complete!");
    Delay_ms(1000);
}

uint8_t esp8266_send_cmd(char *cmd, char *ack, uint16_t waittime)
{
    uint8_t res = 1;

    USART2_RX_STA = 0;
    u2_printf("%s\r\n", cmd);

    if (ack == NULL || waittime == 0) return 0;

    while (waittime--)
    {
        Delay_ms(10);
        if (USART2_RX_STA & 0x8000)
        {
            if (esp8266_check_cmd(ack))
            {
                res = 0;
                break;
            }
            USART2_RX_STA = 0;
        }
    }
    return res;
}

uint8_t esp8266_check_cmd(char *str)
{
    char *strx = NULL;
    if (USART2_RX_STA & 0x8000)
    {
        USART2_RX_BUF[USART2_RX_STA & 0x7FFF] = 0;
        strx = strstr((const char *)USART2_RX_BUF, (const char *)str);
    }
    return (strx != NULL) ? 1 : 0;
}

void ESP8266_SendToTopic(const char *topic, int Data)
{
    USART2_RX_STA = 0;
    u2_printf("AT+MQTTPUB=0,\"%s\",\"%d\",1,0\r\n", topic, Data);
}

void ESP8266_Send(char *property, int Data)
{
    USART2_RX_STA = 0;
    u2_printf("AT+MQTTPUB=0,\"%s\",\"{\\\"%s\\\":%d}\",1,0\r\n", MQTT_TOPIC_POST, property, Data);
}

void ESP8266_SendECGBatch(uint32_t timestamp, uint16_t *data, uint8_t count)
{
    (void)timestamp;
    if (count == 0) return;
    ESP8266_SendToTopic(MQTT_TOPIC_ECG, data[0]);
}

void ESP8266_SendVitalSign(uint16_t heart_rate, uint16_t spo2)
{
    USART2_RX_STA = 0;
    u2_printf("AT+MQTTPUB=0,\"%s\",\"{\\\"heartRate\\\":%d,\\\"oxygenSaturation\\\":%d}\",1,0\r\n",
              MQTT_TOPIC_VITAL, heart_rate, spo2);
}

void ESP8266_SendAlarm(uint8_t alarm_type, uint8_t severity)
{
    USART2_RX_STA = 0;
    u2_printf("AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\":%d,\\\"severity\\\":%d}\",1,0\r\n",
              MQTT_TOPIC_ALARM, alarm_type, severity);
}

void ESP8266_Received(char *PRO)
{
    unsigned char *ret = 0;
    char *property = 0;
    unsigned char i;

    if (PRO == NULL) return;

    if (USART2_RX_STA & 0x8000)
    {
        ret = USART2_RX_BUF;
        if (ret != 0)
        {
            property = strstr((const char *)ret, (const char *)PRO);
            if (property != NULL)
            {
                for (i = 0; i < 5; i++)
                {
                    if ((*(property + 13 + i) >= '0' && *(property + 13 + i) <= '9') ||
                        (*(property + 13 + i) == '}'))
                        Property_Data[i] = *(property + 13 + i);
                }
            }
            USART2_RX_STA = 0;
        }
    }
}
