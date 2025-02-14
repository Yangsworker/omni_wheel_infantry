/*
 * @Date: 2021-12-10 21:26:30
 * @LastEditors: CCsherlock
 * @LastEditTime: 2021-12-10 22:23:30
 * @FilePath: \Projectd:\TDT2022\TDT-sentry\BotGimbal\TDT_Bsp\inc\pwm.h
 * @Description:
 */
#ifndef __PWM_H__
#define __PWM_H__
#include "board.h"
#include "pid.h"
struct Pwm
{
public:
	Pwm(GPIO_TypeDef *_pinPort, uint16_t _pin,TIM_TypeDef *_tim,u8 _ch);
	GPIO_TypeDef *pinPort;
	uint32_t ioRccAHB1Periph;
	uint32_t timRccAHB1Periph;
	uint32_t pinSource;
	uint16_t pin;
	TIM_TypeDef *tim;
	u8 ch;
	uint32_t tempPort;
	uint32_t tempPort1;
	uint32_t tempPort2;
	void pwmInit(u32 arr);
	u32 Arr;
	float result;
	void pwmCalculate(float setVal);
	PidParam pidParam;
	Pid tempPid;
	void loadPidParam();

private:
};
extern Pwm pwmMpuTemp;
#endif