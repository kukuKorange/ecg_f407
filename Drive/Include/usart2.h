/**
  ******************************************************************************
  * @file    usart2.h
  * @brief   USART2串口驱动头文件 (ESP8266通信, STM32F407)
  * @note    PA2=TX, PA3=RX, 115200 baud
  ******************************************************************************
  */

#ifndef __USART2_H
#define __USART2_H

#include <stdint.h>
#include <stdio.h>

#define USART2_MAX_RECV_LEN     2000
#define USART2_MAX_SEND_LEN     600
#define USART2_RX_EN            1

extern uint8_t            USART2_RX_BUF[USART2_MAX_RECV_LEN];
extern uint16_t           USART2_RX_STA;
extern volatile uint8_t   usart2_raw_mode;
extern volatile uint16_t  usart2_raw_len;

void usart2_init(uint32_t bound);
void u2_printf(char *fmt, ...);

#endif
