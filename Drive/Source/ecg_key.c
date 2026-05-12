/**
  ******************************************************************************
  * @file    ecg_key.c
  * @brief   3按键驱动 (STM32F407)
  * @note    Key1=PC1(上一页), Key2=PC3(功能), Key3=PC2(下一页)
  ******************************************************************************
  */

#include "stm32f4xx.h"
#include "ecg_key.h"
#include "lcd_spi_154.h"

uint8_t current_page = PAGE_HEARTRATE;

void ECG_Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

uint8_t Key_GetNum(void)
{
    uint8_t KeyNum = 0;

    /* Key1: PC1 */
    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 0)
    {
        while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1) == 0);
        KeyNum = 1;
    }
    /* Key2: PC3 */
    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 0)
    {
        while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3) == 0);
        KeyNum = 2;
    }
    /* Key3: PC2 */
    if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2) == 0)
    {
        while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2) == 0);
        KeyNum = 3;
    }

    return KeyNum;
}

void Key_Process(void)
{
    uint8_t KeyNum = Key_GetNum();

    if (KeyNum == 0) return;

    switch (KeyNum)
    {
        case 1:  /* 上一页 */
            if (current_page > 0)
                current_page--;
            else
                current_page = PAGE_MAX - 1;
            LCD_Clear();
            break;

        case 2:  /* 功能键: ECG上传 */
            if (current_page == PAGE_ECG)
            {
                extern uint16_t test;
                extern uint8_t Transmit_StartECGUpload(uint32_t timestamp);
                Transmit_StartECGUpload(test);
            }
            break;

        case 3:  /* 下一页 */
            if (current_page < PAGE_MAX - 1)
                current_page++;
            else
                current_page = 0;
            LCD_Clear();
            break;

        default:
            break;
    }
}
