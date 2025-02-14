/*
 * @Date: 2021-12-10 21:26:18
 * @LastEditors: CCsherlock
 * @LastEditTime: 2021-12-11 16:24:55
 * @FilePath: \Projectd:\TDT2022\TDT-sentry\BotGimbal\TDT_Bsp\src\pwm.cpp
 * @Description:
 */
#include "pwm.h"
#include <limits.h>
#include "adc.h"
Pwm pwmMpuTemp(GPIOC, GPIO_Pin_9, TIM3, 4);
static uint32_t judge_BitSite(uint16_t port)
{
	return port == 0x00 ? 0x00 : judge_BitSite(port >> 1) + 1;
}
static uint32_t judge_timBitSite(uint16_t tim)
{
	return tim == 0x00 ? 0x00 : judge_timBitSite(tim - 0x400) + 1;
}
uint8_t selectAF(TIM_TypeDef *_tim)
{
	if (_tim == TIM1 || _tim == TIM2)
	{
		return (uint8_t)0x01;
	}
	else if (_tim == TIM3 || _tim == TIM4 || _tim == TIM5)
	{
		return (uint8_t)0x02;
	}
	else if (_tim == TIM8 || _tim == TIM9 || _tim == TIM10 || _tim == TIM11)
	{
		return (uint8_t)0x03;
	}
	else
	{
		return (uint8_t)0x09;
	}
}
void select_TIM_OC4Init(TIM_TypeDef *TIMx, TIM_OCInitTypeDef *TIM_OCInitStruct, u8 _ch)
{
	switch (_ch)
	{
	case 1:
		TIM_OC1Init(TIMx, TIM_OCInitStruct);			  //根据T指定的参数初始化外设TIM3 OC4
		TIM_OC1PreloadConfig(TIMx, TIM_OCPreload_Enable); //使能TIM3在CCR4上的预装载寄存器
		break;
	case 2:
		TIM_OC2Init(TIMx, TIM_OCInitStruct);			  //根据T指定的参数初始化外设TIM3 OC4
		TIM_OC2PreloadConfig(TIMx, TIM_OCPreload_Enable); //使能TIM3在CCR4上的预装载寄存器
		break;
	case 3:
		TIM_OC3Init(TIMx, TIM_OCInitStruct);			  //根据T指定的参数初始化外设TIM3 OC4
		TIM_OC3PreloadConfig(TIMx, TIM_OCPreload_Enable); //使能TIM3在CCR4上的预装载寄存器
		break;
	case 4:
		TIM_OC4Init(TIMx, TIM_OCInitStruct);			  //根据T指定的参数初始化外设TIM3 OC4
		TIM_OC4PreloadConfig(TIMx, TIM_OCPreload_Enable); //使能TIM3在CCR4上的预装载寄存器
		break;
	default:
		break;
	}
}
Pwm::Pwm(GPIO_TypeDef *_pinPort, uint16_t _pin, TIM_TypeDef *_tim, u8 _ch):tempPid(1)
{
	this->pinPort = _pinPort;
	this->pin = _pin;
	this->pinSource = (uint32_t)judge_BitSite(this->pin) - 0x01;
	this->tim = _tim;
	this->ch = _ch;
	this->ioRccAHB1Periph = ((0x01 << (((judge_BitSite((uint32_t)this->pinPort - AHB1PERIPH_BASE)) >> 2))) >> 1);
	if (this->tim != TIM1 && this->tim != TIM8)
	{
		this->timRccAHB1Periph = (uint32_t)judge_timBitSite((uint32_t)this->tim - APB1PERIPH_BASE) + 1;
	}
}

void Pwm::pwmInit(u32 arr)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	RCC_APB1PeriphClockCmd(this->timRccAHB1Periph, ENABLE);				   // TIM3时钟使能
	RCC_AHB1PeriphClockCmd(this->ioRccAHB1Periph, ENABLE);				   //使能PORTC时钟
	GPIO_PinAFConfig(this->pinPort, this->pinSource, selectAF(this->tim)); // GPIOC9复用为定时器3

	GPIO_InitStructure.GPIO_Pin = this->pin;		   // GPIOC9
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;	   //复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //速度100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	   //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	   //上拉
	GPIO_Init(this->pinPort, &GPIO_InitStructure);	   //初始化PC9

	TIM_TimeBaseStructure.TIM_Prescaler = 2;					//定时器分频
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseStructure.TIM_Period = arr - 1;					//自动重装载值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

	TIM_TimeBaseInit(this->tim, &TIM_TimeBaseStructure); //初始化定时器3

	//初始化TIM3 Channel4 PWM模式
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;			  //选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputState_Disable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性低
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;
	select_TIM_OC4Init(this->tim,&TIM_OCInitStructure,this->ch);

	TIM_ARRPreloadConfig(this->tim, ENABLE); // ARPE使能

	TIM_Cmd(this->tim, ENABLE); //使能TIM3

	this->tim->CCR4 = 100;
	this->Arr = arr;
	loadPidParam();
}
void Pwm::loadPidParam()
{
	pidParam.kp = 80;
	pidParam.ki = 40;
	pidParam.kd = 10;
	pidParam.integralErrorMax = this->Arr / 100;
	pidParam.resultMax = this->Arr;
	tempPid.paramPtr = &pidParam;
	tempPid.fbValuePtr[0] = &adcMpuTemp.temperature;
}
void Pwm::pwmCalculate(float setVal)
{
	result = tempPid.Calculate(setVal);
	if (result <= 0)
	{
		result = 0;
	}
	if(tempPid.error > 2)
	{
		result=this->Arr;
		tempPid.Clear();
		tempPid.integralError = 10;
	}
	if(tempPid.error < -1)
	{
		result=0;
		tempPid.Clear();
		tempPid.integralError = 0;
	}
	tempPid.integralError = LIMIT(tempPid.integralError,0,pidParam.integralErrorMax);
	result = LIMIT(result,0,pidParam.resultMax);

	this->tim->CCR4 = (u32)result;
}