/**
  ******************************************************************************
  * @file    ad.c
  * @brief   ADC驱动 (STM32F407)
  * @note    PF6 → ADC3_Channel_4, 12位分辨率, 软件触发单次转换
  ******************************************************************************
  */

#include "stm32f4xx.h"
#include "ad.h"

void AD_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;

    /* 使能时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

    /* PF6 配置为模拟输入 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    /* ADC通用配置 */
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;  /* 84/4=21MHz */
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

    /* ADC3配置 */
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_Init(ADC3, &ADC_InitStructure);

    /* 配置规则通道: PF6 = ADC3_Channel_4 */
    ADC_RegularChannelConfig(ADC3, ADC_Channel_4, 1, ADC_SampleTime_56Cycles);

    /* 使能ADC3 */
    ADC_Cmd(ADC3, ENABLE);
}

uint16_t AD_GetValue(void)
{
    ADC_SoftwareStartConv(ADC3);
    while (ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC) == RESET);
    return ADC_GetConversionValue(ADC3);
}
