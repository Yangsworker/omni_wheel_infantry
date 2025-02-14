#include "board.h"
u8 Init_OK;

/* SystemFrequency / 1000    1ms�ж�һ��
 * SystemFrequency / 100000	 10us�ж�һ��
 * SystemFrequency / 1000000 1us�ж�һ��
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
	Cycle_T[item][OLD] = Cycle_T[item][NOW];	//��һ�ε�ʱ��
	Cycle_T[item][NOW] = GetSysTime_us()/1000000.0f; //���ε�ʱ��
	Cycle_T[item][NEW] = ( ( Cycle_T[item][NOW] - Cycle_T[item][OLD] ) );//�����ʱ�䣨���ڣ�
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
	
	/*�жϷ���*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	/*�δ�ʱ������*/
	SysTick_Init();
	Cycle_Time_Init();
	/*LED��ʼ��*/
	LED_Init();
	/* ����ϵͳ��ʼ�� */
 	Judge_Uart3_Config();

	/*CAN1��ʼ��*/
	Can0_Init();
	/*TFMiniPlus�������ʼ��(USART1��ʼ��)*/
//	SuperPower_USART_Init();
//	TIM3_Int_Init(1,71999);
	/*�Զ�������վ������*/
	//GY53_Init();
	/*��ʼ�����*/
	Init_OK = 1;
}
