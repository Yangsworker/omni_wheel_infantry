#include "board.h"
#include "stm32f10x.h"


//定时器4通道1输入捕获配置
//传感器IIC

	TIM_ICInitTypeDef  TIM4_ICInitStructure;
	TIM_ICInitTypeDef  TIM1_ICInitStructure;

void GY53_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);	//使能TIM4时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);	//使能TIM4时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //使能GPIOA时钟

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6|GPIO_Pin_7;  //PA0 清除之前设置  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0 输入  
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;  //PA0 清除之前设置  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0 输入  
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//初始化定时器4 TIM4	 
	TIM_TimeBaseStructure.TIM_Period = 0xffff; //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =899-1; 	//预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	//初始化定时器4 TIM1 
	TIM_TimeBaseStructure.TIM_Period = 0xffff; //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =899-1; 	//预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	//初始化TIM4输入捕获参数
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	选择输入端 IC1映射到TI1上
  	TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  	TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  	TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  	TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  	TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_2; //CC1S=01 	选择输入端 IC1映射到TI1上
  	TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  	TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  	TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  	TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  	TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	
	//初始化TIM1输入捕获参数
	TIM1_ICInitStructure.TIM_Channel = TIM_Channel_3; //CC1S=01 	选择输入端 IC1映射到TI1上
  	TIM1_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  	TIM1_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  	TIM1_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  	TIM1_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 配置输入滤波器 不滤波
  	TIM_ICInit(TIM1, &TIM1_ICInitStructure);

	
	//中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 
	
	//中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;  //TIM4中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 
	

	TIM_ITConfig(TIM4,TIM_IT_Update|TIM_IT_CC1|TIM_IT_CC2,ENABLE);//允许更新中断 ,允许CC1IE捕获中断	
	
	TIM_ITConfig(TIM1,TIM_IT_Update|TIM_IT_CC3,ENABLE);//允许更新中断 ,允许CC1IE捕获中断	

   	TIM_Cmd(TIM4,ENABLE ); 	//使能定时器4
    TIM_Cmd(TIM1,ENABLE ); 	//使能定时器1




}

u8  TIM4CH1_CAPTURE_STA=0;	//输入捕获状态		    				
u16	TIM4CH1_CAPTURE_VAL;	//输入捕获值
u8  TIM4CH2_CAPTURE_STA=0;	//输入捕获状态		    				
u16	TIM4CH2_CAPTURE_VAL;	//输入捕获值
u8  TIM1CH3_CAPTURE_STA=0;	//输入捕获状态		    				
u16	TIM1CH3_CAPTURE_VAL;	//输入捕获值
 double RC_PPM_CH;
//定时器4中断服务程序	 
int distance[3];

void TIM4_IRQHandler(void)
{
 	if((TIM4CH1_CAPTURE_STA&0X80)==0)//还未成功捕获	
	{	  
		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
		{	    
			if(TIM4CH1_CAPTURE_STA&0X40)//已经捕获到高电平了
			{
				if((TIM4CH1_CAPTURE_STA&0X3F)==0X3F)//高电平太长了
				{
					TIM4CH1_CAPTURE_STA|=0X80;//标记成功捕获了一次
					TIM4CH1_CAPTURE_VAL=0XFFFF;
				}else TIM4CH1_CAPTURE_STA++;
			}
		}
		if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)//捕获1发生捕获事件
		{	

			if(TIM4CH1_CAPTURE_STA&0X40)		//捕获到一个下降沿 		
			{	  			
				TIM4CH1_CAPTURE_STA|=0X80;		//标记成功捕获到一次高电平脉宽
				TIM4CH1_CAPTURE_VAL=TIM_GetCapture1(TIM4);
		   		TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获
			}else  								//还未开始,第一次捕获上升沿
			{
				TIM4CH1_CAPTURE_STA=0;			//清空
				TIM4CH1_CAPTURE_VAL=0;
	 			TIM_SetCounter(TIM4,0);
				TIM4CH1_CAPTURE_STA|=0X40;		//标记捕获到了上升沿
		   		TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Falling);		//CC1P=1 设置为下降沿捕获
			}
		}
 	}
 
	if(TIM4CH1_CAPTURE_STA&0X80)//成功捕获到了一次上升沿
	{
		distance[0]=TIM4CH1_CAPTURE_STA&0X3F;
		distance[0]*=65536;//溢出时间总和
		distance[0]+=TIM4CH1_CAPTURE_VAL;//得到总的高电平时间
		distance[0]=LPF2pApply_1(distance[0]);//			
		TIM4CH1_CAPTURE_STA=0;//开启下一次捕获
	}
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1|TIM_IT_Update); //清除中断标志位
	
		
 	if((TIM4CH2_CAPTURE_STA&0X80)==0)//还未成功捕获	
	{	  
		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
		{	    
			if(TIM4CH2_CAPTURE_STA&0X40)//已经捕获到高电平了
			{
				if((TIM4CH2_CAPTURE_STA&0X3F)==0X3F)//高电平太长了
				{
					TIM4CH2_CAPTURE_STA|=0X80;//标记成功捕获了一次
					TIM4CH2_CAPTURE_VAL=0XFFFF;
				}else TIM4CH2_CAPTURE_STA++;
			}
		}
		if (TIM_GetITStatus(TIM4, TIM_IT_CC2) != RESET)//捕获1发生捕获事件
		{	
			if(TIM4CH2_CAPTURE_STA&0X40)		//捕获到一个下降沿 		
			{	  			
				TIM4CH2_CAPTURE_STA|=0X80;		//标记成功捕获到一次高电平脉宽
				TIM4CH2_CAPTURE_VAL=TIM_GetCapture2(TIM4);
		   		TIM_OC2PolarityConfig(TIM4,TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获
			}
			else  								//还未开始,第一次捕获上升沿
			{
				TIM4CH2_CAPTURE_STA=0;			//清空
				TIM4CH2_CAPTURE_VAL=0;
	 			TIM_SetCounter(TIM4,0);
				TIM4CH2_CAPTURE_STA|=0X40;		//标记捕获到了上升沿
		   	TIM_OC2PolarityConfig(TIM4,TIM_ICPolarity_Falling);		//CC1P=1 设置为下降沿捕获
			}
		}
 	}

	if(TIM4CH2_CAPTURE_STA&0X80)//成功捕获到了一次上升沿
	{
		distance[1]=TIM4CH2_CAPTURE_STA&0X3F;
		distance[1]*=65536;//溢出时间总和
		distance[1]+=TIM4CH2_CAPTURE_VAL;//得到总的高电平时间
		distance[1]=LPF2pApply_2(distance[1]);//		
		TIM4CH2_CAPTURE_STA=0;//开启下一次捕获
	}
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC2|TIM_IT_Update); //清除中断标志位

}




void TIM1_CC_IRQHandler(void)
{
 	if((TIM1CH3_CAPTURE_STA&0X80)==0)//还未成功捕获	
	{	  
		if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
		{	    
			if(TIM1CH3_CAPTURE_STA&0X40)//已经捕获到高电平了
			{
				if((TIM1CH3_CAPTURE_STA&0X3F)==0X3F)//高电平太长了
				{
					TIM1CH3_CAPTURE_STA|=0X80;//标记成功捕获了一次
					TIM1CH3_CAPTURE_VAL=0XFFFF;
				}else TIM1CH3_CAPTURE_STA++;
			}
		}
		if (TIM_GetITStatus(TIM1, TIM_IT_CC3) != RESET)//捕获1发生捕获事件
		{	
			if(TIM1CH3_CAPTURE_STA&0X40)		//捕获到一个下降沿 		
			{	  			
				TIM1CH3_CAPTURE_STA|=0X80;		//标记成功捕获到一次高电平脉宽
				TIM1CH3_CAPTURE_VAL=TIM_GetCapture3(TIM1);
		   		TIM_OC3PolarityConfig(TIM1,TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获
			}
			else  								//还未开始,第一次捕获上升沿
			{
				TIM1CH3_CAPTURE_STA=0;			//清空
				TIM1CH3_CAPTURE_VAL=0;
	 			TIM_SetCounter(TIM1,0);
				TIM1CH3_CAPTURE_STA|=0X40;		//标记捕获到了上升沿
				TIM_OC3PolarityConfig(TIM1,TIM_ICPolarity_Falling);		//CC1P=1 设置为下降沿捕获
			}
		}
 	}

	if(TIM1CH3_CAPTURE_STA&0X80)//成功捕获到了一次上升沿
	{
			distance[2]=TIM1CH3_CAPTURE_STA&0X3F;
			distance[2]*=65536;//溢出时间总和
			distance[2]+=TIM1CH3_CAPTURE_VAL;//得到总的高电平时间
			distance[2]=LPF2pApply_3(distance[2]);//
			TIM1CH3_CAPTURE_STA=0;//开启下一次捕获
	}
    TIM_ClearITPendingBit(TIM1, TIM_IT_CC3|TIM_IT_Update); //清除中断标志位
}
