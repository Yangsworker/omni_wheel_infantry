#include "board.h"
#include "stm32f10x.h"


//��ʱ��4ͨ��1���벶������
//������IIC

	TIM_ICInitTypeDef  TIM4_ICInitStructure;
	TIM_ICInitTypeDef  TIM1_ICInitStructure;

void GY53_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);	//ʹ��TIM4ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);	//ʹ��TIM4ʱ��
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //ʹ��GPIOAʱ��

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6|GPIO_Pin_7;  //PA0 ���֮ǰ����  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0 ����  
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;  //PA0 ���֮ǰ����  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0 ����  
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//��ʼ����ʱ��4 TIM4	 
	TIM_TimeBaseStructure.TIM_Period = 0xffff; //�趨�������Զ���װֵ 
	TIM_TimeBaseStructure.TIM_Prescaler =899-1; 	//Ԥ��Ƶ��   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	
	//��ʼ����ʱ��4 TIM1 
	TIM_TimeBaseStructure.TIM_Period = 0xffff; //�趨�������Զ���װֵ 
	TIM_TimeBaseStructure.TIM_Prescaler =899-1; 	//Ԥ��Ƶ��   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	//��ʼ��TIM4���벶�����
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  	TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  	TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  	TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  	TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  	TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	TIM4_ICInitStructure.TIM_Channel = TIM_Channel_2; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  	TIM4_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  	TIM4_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  	TIM4_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  	TIM4_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  	TIM_ICInit(TIM4, &TIM4_ICInitStructure);
	
	//��ʼ��TIM1���벶�����
	TIM1_ICInitStructure.TIM_Channel = TIM_Channel_3; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  	TIM1_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  	TIM1_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  	TIM1_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  	TIM1_ICInitStructure.TIM_ICFilter = 0x00;//IC1F=0000 ���������˲��� ���˲�
  	TIM_ICInit(TIM1, &TIM1_ICInitStructure);

	
	//�жϷ����ʼ��
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM4�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ��� 
	
	//�жϷ����ʼ��
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_CC_IRQn;  //TIM4�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ��� 
	

	TIM_ITConfig(TIM4,TIM_IT_Update|TIM_IT_CC1|TIM_IT_CC2,ENABLE);//��������ж� ,����CC1IE�����ж�	
	
	TIM_ITConfig(TIM1,TIM_IT_Update|TIM_IT_CC3,ENABLE);//��������ж� ,����CC1IE�����ж�	

   	TIM_Cmd(TIM4,ENABLE ); 	//ʹ�ܶ�ʱ��4
    TIM_Cmd(TIM1,ENABLE ); 	//ʹ�ܶ�ʱ��1




}

u8  TIM4CH1_CAPTURE_STA=0;	//���벶��״̬		    				
u16	TIM4CH1_CAPTURE_VAL;	//���벶��ֵ
u8  TIM4CH2_CAPTURE_STA=0;	//���벶��״̬		    				
u16	TIM4CH2_CAPTURE_VAL;	//���벶��ֵ
u8  TIM1CH3_CAPTURE_STA=0;	//���벶��״̬		    				
u16	TIM1CH3_CAPTURE_VAL;	//���벶��ֵ
 double RC_PPM_CH;
//��ʱ��4�жϷ������	 
int distance[3];

void TIM4_IRQHandler(void)
{
 	if((TIM4CH1_CAPTURE_STA&0X80)==0)//��δ�ɹ�����	
	{	  
		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
		{	    
			if(TIM4CH1_CAPTURE_STA&0X40)//�Ѿ����񵽸ߵ�ƽ��
			{
				if((TIM4CH1_CAPTURE_STA&0X3F)==0X3F)//�ߵ�ƽ̫����
				{
					TIM4CH1_CAPTURE_STA|=0X80;//��ǳɹ�������һ��
					TIM4CH1_CAPTURE_VAL=0XFFFF;
				}else TIM4CH1_CAPTURE_STA++;
			}
		}
		if (TIM_GetITStatus(TIM4, TIM_IT_CC1) != RESET)//����1���������¼�
		{	

			if(TIM4CH1_CAPTURE_STA&0X40)		//����һ���½��� 		
			{	  			
				TIM4CH1_CAPTURE_STA|=0X80;		//��ǳɹ�����һ�θߵ�ƽ����
				TIM4CH1_CAPTURE_VAL=TIM_GetCapture1(TIM4);
		   		TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
			}else  								//��δ��ʼ,��һ�β���������
			{
				TIM4CH1_CAPTURE_STA=0;			//���
				TIM4CH1_CAPTURE_VAL=0;
	 			TIM_SetCounter(TIM4,0);
				TIM4CH1_CAPTURE_STA|=0X40;		//��ǲ�����������
		   		TIM_OC1PolarityConfig(TIM4,TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
			}
		}
 	}
 
	if(TIM4CH1_CAPTURE_STA&0X80)//�ɹ�������һ��������
	{
		distance[0]=TIM4CH1_CAPTURE_STA&0X3F;
		distance[0]*=65536;//���ʱ���ܺ�
		distance[0]+=TIM4CH1_CAPTURE_VAL;//�õ��ܵĸߵ�ƽʱ��
		distance[0]=LPF2pApply_1(distance[0]);//			
		TIM4CH1_CAPTURE_STA=0;//������һ�β���
	}
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1|TIM_IT_Update); //����жϱ�־λ
	
		
 	if((TIM4CH2_CAPTURE_STA&0X80)==0)//��δ�ɹ�����	
	{	  
		if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
		{	    
			if(TIM4CH2_CAPTURE_STA&0X40)//�Ѿ����񵽸ߵ�ƽ��
			{
				if((TIM4CH2_CAPTURE_STA&0X3F)==0X3F)//�ߵ�ƽ̫����
				{
					TIM4CH2_CAPTURE_STA|=0X80;//��ǳɹ�������һ��
					TIM4CH2_CAPTURE_VAL=0XFFFF;
				}else TIM4CH2_CAPTURE_STA++;
			}
		}
		if (TIM_GetITStatus(TIM4, TIM_IT_CC2) != RESET)//����1���������¼�
		{	
			if(TIM4CH2_CAPTURE_STA&0X40)		//����һ���½��� 		
			{	  			
				TIM4CH2_CAPTURE_STA|=0X80;		//��ǳɹ�����һ�θߵ�ƽ����
				TIM4CH2_CAPTURE_VAL=TIM_GetCapture2(TIM4);
		   		TIM_OC2PolarityConfig(TIM4,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
			}
			else  								//��δ��ʼ,��һ�β���������
			{
				TIM4CH2_CAPTURE_STA=0;			//���
				TIM4CH2_CAPTURE_VAL=0;
	 			TIM_SetCounter(TIM4,0);
				TIM4CH2_CAPTURE_STA|=0X40;		//��ǲ�����������
		   	TIM_OC2PolarityConfig(TIM4,TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
			}
		}
 	}

	if(TIM4CH2_CAPTURE_STA&0X80)//�ɹ�������һ��������
	{
		distance[1]=TIM4CH2_CAPTURE_STA&0X3F;
		distance[1]*=65536;//���ʱ���ܺ�
		distance[1]+=TIM4CH2_CAPTURE_VAL;//�õ��ܵĸߵ�ƽʱ��
		distance[1]=LPF2pApply_2(distance[1]);//		
		TIM4CH2_CAPTURE_STA=0;//������һ�β���
	}
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC2|TIM_IT_Update); //����жϱ�־λ

}




void TIM1_CC_IRQHandler(void)
{
 	if((TIM1CH3_CAPTURE_STA&0X80)==0)//��δ�ɹ�����	
	{	  
		if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
		{	    
			if(TIM1CH3_CAPTURE_STA&0X40)//�Ѿ����񵽸ߵ�ƽ��
			{
				if((TIM1CH3_CAPTURE_STA&0X3F)==0X3F)//�ߵ�ƽ̫����
				{
					TIM1CH3_CAPTURE_STA|=0X80;//��ǳɹ�������һ��
					TIM1CH3_CAPTURE_VAL=0XFFFF;
				}else TIM1CH3_CAPTURE_STA++;
			}
		}
		if (TIM_GetITStatus(TIM1, TIM_IT_CC3) != RESET)//����1���������¼�
		{	
			if(TIM1CH3_CAPTURE_STA&0X40)		//����һ���½��� 		
			{	  			
				TIM1CH3_CAPTURE_STA|=0X80;		//��ǳɹ�����һ�θߵ�ƽ����
				TIM1CH3_CAPTURE_VAL=TIM_GetCapture3(TIM1);
		   		TIM_OC3PolarityConfig(TIM1,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
			}
			else  								//��δ��ʼ,��һ�β���������
			{
				TIM1CH3_CAPTURE_STA=0;			//���
				TIM1CH3_CAPTURE_VAL=0;
	 			TIM_SetCounter(TIM1,0);
				TIM1CH3_CAPTURE_STA|=0X40;		//��ǲ�����������
				TIM_OC3PolarityConfig(TIM1,TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
			}
		}
 	}

	if(TIM1CH3_CAPTURE_STA&0X80)//�ɹ�������һ��������
	{
			distance[2]=TIM1CH3_CAPTURE_STA&0X3F;
			distance[2]*=65536;//���ʱ���ܺ�
			distance[2]+=TIM1CH3_CAPTURE_VAL;//�õ��ܵĸߵ�ƽʱ��
			distance[2]=LPF2pApply_3(distance[2]);//
			TIM1CH3_CAPTURE_STA=0;//������һ�β���
	}
    TIM_ClearITPendingBit(TIM1, TIM_IT_CC3|TIM_IT_Update); //����жϱ�־λ
}
