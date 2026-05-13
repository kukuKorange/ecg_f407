/**
  ******************************************************************************
  * @file    usart2.c
  * @brief   USART2串口驱动 (STM32F407, ESP8266通信)
  * @note    PD5=TX(AF7), PD6=RX(AF7), 中断接收, 帧结束: \r\n
  ******************************************************************************
  */

#include "stm32f4xx.h"
#include "usart2.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

__attribute__((aligned(8))) uint8_t USART2_TX_BUF[USART2_MAX_SEND_LEN];
uint8_t USART2_RX_BUF[USART2_MAX_RECV_LEN];
volatile uint8_t  usart2_raw_mode = 0;
volatile uint16_t usart2_raw_len  = 0;
uint16_t USART2_RX_STA = 0;

void usart2_init(uint32_t bound)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* 使能时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /* PD5(TX), PD6(RX) 复用功能AF7 */
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* NVIC */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* USART2参数 */
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART2, &USART_InitStructure);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART2, ENABLE);
}

#ifdef USART2_RX_EN
void USART2_IRQHandler(void)
{
    uint8_t Res;

    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        Res = USART_ReceiveData(USART2);

        if (usart2_raw_mode)
        {
            if (usart2_raw_len < USART2_MAX_RECV_LEN - 1)
            {
                USART2_RX_BUF[usart2_raw_len] = Res;
                usart2_raw_len++;
            }
        }
        else if ((USART2_RX_STA & 0x8000) == 0)
        {
            if (USART2_RX_STA & 0x4000)
            {
                if (Res != 0x0A)
                    USART2_RX_STA = 0;
                else
                    USART2_RX_STA |= 0x8000;
            }
            else
            {
                if (Res == 0x0D)
                    USART2_RX_STA |= 0x4000;
                else
                {
                    USART2_RX_BUF[USART2_RX_STA & 0x3FFF] = Res;
                    USART2_RX_STA++;
                    if (USART2_RX_STA > (USART2_MAX_RECV_LEN - 1))
                        USART2_RX_STA = 0;
                }
            }
        }
    }
}
#endif

void u2_printf(char *fmt, ...)
{
    uint16_t i, j;
    va_list ap;

    va_start(ap, fmt);
    vsprintf((char *)USART2_TX_BUF, fmt, ap);
    va_end(ap);

    i = strlen((const char *)USART2_TX_BUF);

    for (j = 0; j < i; j++)
    {
        while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
        USART_SendData(USART2, (uint8_t)USART2_TX_BUF[j]);
    }
}
