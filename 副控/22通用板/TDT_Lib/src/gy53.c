#include "gy53.h"
float GYdistance[3] = {2500,2500,2500};
float GYdistance_last[3] = {2500,2500,2500};
float calculateTime_In[3] = {0};
float calculateTime_Out[3] = {0};
GPIO_TypeDef * GY_GPIO[3] = {GPIOA,GPIOB,GPIOA};
u32 GYtime;
uint16_t GY_Pin[3] = {GPIO_Pin_11,GPIO_Pin_6,GPIO_Pin_12};
uint32_t GY_APB[3] = {RCC_APB2Periph_GPIOA,RCC_APB2Periph_GPIOB,RCC_APB2Periph_GPIOA};

void GY53_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(GY_APB[LF], ENABLE );  
	RCC_APB2PeriphClockCmd(GY_APB[LB], ENABLE );  
	RCC_APB2PeriphClockCmd(GY_APB[FF], ENABLE );  

	GPIO_InitStructure.GPIO_Pin = GY_Pin[LF]; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_Init(GY_GPIO[LF], &GPIO_InitStructure); 
	
	GPIO_InitStructure.GPIO_Pin = GY_Pin[LB]; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_Init(GY_GPIO[LB], &GPIO_InitStructure); 
	
	GPIO_InitStructure.GPIO_Pin = GY_Pin[FF]; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
	GPIO_Init(GY_GPIO[FF], &GPIO_InitStructure); 
}
void TIM3_Int_Init(u16 arr,u32 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器


	TIM_Cmd(TIM3, ENABLE);  //使能TIMx					 
}
u8 Inflag[3] = {0};
void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx更新中断标志 
		/*运行一次*/
		Distance_Read();
	}
}
void Distance_Read(void)
{
	for(u8 i = 0;i<3;i++)
	{
		if(GPIO_ReadInputDataBit(GY_GPIO[i],GY_Pin[i]) == 1)//第一次发出测量信号
		{
			if(Inflag[i] == 0)
			{
				calculateTime_In[i] = GetSysTime_us();
				Inflag[i] = 1;
			}
		}
		else	//接收到测量信号
		{
			if(Inflag[i] == 1)
			{
				calculateTime_Out[i] = GetSysTime_us();
				GYdistance_last[i] = GYdistance[i];
				GYdistance[i] = (float)(calculateTime_Out[i] - calculateTime_In[i])/100.0f;//mm
				Inflag[i] = 0;
			}
		}
	}
}
