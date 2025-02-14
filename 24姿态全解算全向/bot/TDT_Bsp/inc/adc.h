/*
 * @Date: 2021-12-10 20:23:21
 * @LastEditors: CCsherlock
 * @LastEditTime: 2021-12-10 22:23:13
 * @FilePath: \Projectd:\TDT2022\TDT-sentry\BotGimbal\TDT_Bsp\inc\adc.h
 * @Description:
 */
#ifndef __ADC_H__
#define __ADC_H__
#include "board.h"
struct Adc
{
private:
	/* data */
public:
	Adc(GPIO_TypeDef *_pinPort, uint16_t _pin, ADC_TypeDef *_ADCx, uint8_t _ch);
	GPIO_TypeDef *pinPort;     ///<SCL�˿�
    ADC_TypeDef *ADCx;     ///<SDA�˿�
    uint16_t pin;           ///<SCL����
    uint32_t ioRccAHB1Periph; ///<SCL�˿�ʱ��
    uint32_t adcAHB2Periph; ///<SDA�˿�ʱ��
	uint8_t ch;
	void adcInit();
	void Get_Adc();
	int rawData;
	float temperature;
	void readAdc();
};
extern Adc adcMpuTemp;
#endif