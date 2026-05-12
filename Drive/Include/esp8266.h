/**
  ******************************************************************************
  * @file    esp8266.h
  * @brief   ESP8266 WiFi模块驱动头文件
  ******************************************************************************
  */

#ifndef __ESP8266_H
#define __ESP8266_H

#include <stdint.h>
#include <string.h>

/*============================ 服务器模式 ============================*/

#define MQTT_USE_ALIYUN     0

/*============================ WiFi配置 ============================*/

#define WIFI_NAME           "5132"
#define WIFI_PASSWORD       "ttst4qrj"

/*============================ MQTT主题 ============================*/

#define MQTT_TOPIC_HEARTRATE    "health/heartrate"
#define MQTT_TOPIC_SPO2         "health/spo2"
#define MQTT_TOPIC_TEMPERATURE  "health/temperature"
#define MQTT_TOPIC_ECG          "health/ecg"
#define MQTT_TOPIC_ALARM        "health/alarm"
#define MQTT_TOPIC_VITAL    MQTT_TOPIC_HEARTRATE
#define MQTT_TOPIC_POST     MQTT_TOPIC_HEARTRATE

/*============================ MQTT服务器配置 ============================*/

#if (MQTT_USE_ALIYUN == 1)
#define MQTT_USERCFG    "AT+MQTTUSERCFG=0,1,\"NULL\",\"esp01s&k16e93sVlLN\",\"4ac4f12ef4b322f3212b281e7addd35477f3a2ebf27fccf80dac6678201d4b2c\",0,0,\"\""
#define MQTT_CLIENTID   "AT+MQTTCLIENTID=0,\"k16e93sVlLN.esp01s|securemode=2\\,signmethod=hmacsha256\\,timestamp=1744624949805|\""
#define MQTT_CONN       "AT+MQTTCONN=0,\"iot-06z00dazkl34gjg.mqtt.iothub.aliyuncs.com\",1883,1"
#else
#define MQTT_BROKER_HOST    "47.115.148.200"
#define MQTT_BROKER_PORT    "1883"
#define MQTT_CLIENT_ID      "ecg_stm32_f407"
#define MQTT_USERCFG    "AT+MQTTUSERCFG=0,1,\"" MQTT_CLIENT_ID "\",\"\",\"\",0,0,\"\""
#define MQTT_CLIENTID   ""
#define MQTT_CONN       "AT+MQTTCONN=0,\"" MQTT_BROKER_HOST "\"," MQTT_BROKER_PORT ",1"
#endif

/*============================ 外部变量 ============================*/

extern unsigned char Property_Data[];
extern char esp8266_mac[18];

/*============================ 函数声明 ============================*/

void ESP8266_Init(void);
uint8_t esp8266_send_cmd(char *cmd, char *ack, uint16_t waittime);
uint8_t esp8266_check_cmd(char *str);
void ESP8266_SendToTopic(const char *topic, int Data);
void ESP8266_Send(char *property, int Data);
void ESP8266_SendECGBatch(uint32_t timestamp, uint16_t *data, uint8_t count);
void ESP8266_SendVitalSign(uint16_t heart_rate, uint16_t spo2);
void ESP8266_SendAlarm(uint8_t alarm_type, uint8_t severity);
void ESP8266_Received(char *PRO);

#endif
