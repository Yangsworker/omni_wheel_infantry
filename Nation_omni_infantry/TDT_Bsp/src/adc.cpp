/*
 * @Date: 2021-12-10 20:23:13
 * @LastEditors: CCsherlock
 * @LastEditTime: 2021-12-10 22:23:02
 * @FilePath: \Projectd:\TDT2022\TDT-sentry\BotGimbal\TDT_Bsp\src\adc.cpp
 * @Description:
 */
#include "adc.h"
#include "filter.h"
Lpf2p mpuAdc;
Adc adcMpuTemp(GPIOC,GPIO_Pin_2,ADC1,12);
static uint32_t judge_BitSite(uint16_t port)
{
    return port == 0x00 ? 0x00 : judge_BitSite(port >> 1) + 1;
}
Adc::Adc(GPIO_TypeDef *_pinPort, uint16_t _pin, ADC_TypeDef *_ADCx, uint8_t _ch)
{
	this->pinPort = _pinPort;
	this->pin = _pin;
	this->ADCx = _ADCx;
	this->ch = _ch;
	this->ioRccAHB1Periph = ((0x01 << (((judge_BitSite((uint32_t) this->pinPort - AHB1PERIPH_BASE)) >> 2)))>> 1);
	this->adcAHB2Periph = ((((uint32_t) this->ADCx - APB2PERIPH_BASE - (uint32_t)0x2000)>>8)+0x01) << 8;
}

void Adc::adcInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  ADC_InitTypeDef ADC_InitStructure;

  RCC_AHB1PeriphClockCmd(this->ioRccAHB1Periph, ENABLE); //使能GPIOC时钟
  RCC_APB2PeriphClockCmd(this->adcAHB2Periph, ENABLE);  //使能ADC1时钟

  GPIO_InitStructure.GPIO_Pin = this->pin;             // PC2 通道12
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;          //模拟输入
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;      //不带上下拉
  GPIO_Init(this->pinPort, &GPIO_InitStructure);                //初始化
  RCC_APB2PeriphResetCmd(this->adcAHB2Periph, ENABLE);  // ADC1复位
  RCC_APB2PeriphResetCmd(this->adcAHB2Periph, DISABLE); //复位结束

  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                     //独立模式
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; //两个采样阶段之间的延迟5个时钟
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;      // DMA失能
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;                  //预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz
  ADC_CommonInit(&ADC_CommonInitStructure);                                    //初始化

  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                      // 12位模式
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;                               //非扫描模式
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                         //关闭连续转换
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //禁止触发检测，使用软件触发
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                      //右对齐
  ADC_InitStructure.ADC_NbrOfConversion = 1;                                  // 1个转换在规则序列中 也就是只转换规则序列1
  ADC_Init(this->ADCx, &ADC_InitStructure);                                         // ADC初始化

  ADC_Cmd(this->ADCx, ENABLE); //开启AD转换器

  mpuAdc.SetCutoffFreq(500, 10);
}

void Adc::readAdc()
{
  rawData = ADC_GetConversionValue(this->ADCx);
}
void Adc::Get_Adc()
{
  //设置指定ADC的规则组通道，一个序列，采样时间
  ADC_RegularChannelConfig(this->ADCx, this->ch, 1, ADC_SampleTime_480Cycles); // ADC1,ADC通道,480个周期,提高采样时间可以提高精确度

  ADC_SoftwareStartConv(this->ADCx); //使能指定的ADC1的软件转换启动功能

  while (!ADC_GetFlagStatus(this->ADCx, ADC_FLAG_EOC))
    ; //等待转换结束

  rawData = ADC_GetConversionValue(this->ADCx); //返回最近一次ADC1规则组的转换结果

  temperature = (float)(mpuAdc.Apply((float)rawData) * 0.0371f) - 17.619;
}
