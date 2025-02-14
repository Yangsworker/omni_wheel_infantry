#include "board.h"
u8 Init_OK;

/* SystemFrequency / 1000    1ms中断一次
 * SystemFrequency / 100000	 10us中断一次
 * SystemFrequency / 1000000 1us中断一次
 */
void SysTick_Init(void)
{
	if (SysTick_Config(SystemCoreClock / 1000))
	{
		/* Capture error */
		while (1);
	}
}
volatile uint32_t sysTickUptime=0;

#define TICK_PER_SECOND 1000
#define TICK_US	(1000000/TICK_PER_SECOND)


uint32_t GetSysTime_us(void)
{
	register uint32_t ms;
	u32 value;
	ms = sysTickUptime;
	value = ms * TICK_US + (SysTick->LOAD - SysTick->VAL) * TICK_US / SysTick->LOAD;
	return value;
}

void delay_ms(unsigned int t)
{
	int i;
	for( i=0; i<t; i++)
	{
		int a=10300;
		while(a--);
	}
}

void delay_us(unsigned int t)
{
	int i;
	for( i=0; i<t; i++)
	{
		int a=9;
		while(a--);
	}
}

volatile float Cycle_T[GET_TIME_NUM][3];
float Get_Cycle_T(u8 item)	
{
	Cycle_T[item][OLD] = Cycle_T[item][NOW];	//上一次的时间
	Cycle_T[item][NOW] = GetSysTime_us()/1000000.0f; //本次的时间
	Cycle_T[item][NEW] = ( ( Cycle_T[item][NOW] - Cycle_T[item][OLD] ) );//间隔的时间（周期）
	return Cycle_T[item][NEW];
}

void Cycle_Time_Init()
{
	u8 i;
	for(i=0;i<GET_TIME_NUM;i++)
	{
		Get_Cycle_T(i);
	}
}

void TDT_Board_ALL_Init(void)
{
	LPF2pSetCutoffFreq_1(50,10);
	LPF2pSetCutoffFreq_2(50,10);
	LPF2pSetCutoffFreq_3(50,10);
	
	/*中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	/*滴答定时器配置*/
	SysTick_Init();
	Cycle_Time_Init();
	/*LED初始化*/
	LED_Init();
	/* 裁判系统初始化 */
 	Judge_Uart3_Config();

	/*CAN1初始化*/
	Can0_Init();
	/*TFMiniPlus激光测距初始化(USART1初始化)*/
//	SuperPower_USART_Init();
//	TIM3_Int_Init(1,71999);
	/*自动进补给站传感器*/
	//GY53_Init();
	/*初始化完成*/
	Init_OK = 1;
}
