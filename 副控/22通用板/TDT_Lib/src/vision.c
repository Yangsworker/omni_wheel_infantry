#include "vision.h"
#include "string.h"
#include "crc.h"

int16_t vision_full_count = - 1;
unsigned char SuperPowerRx_buffer[sizeof(RecvStruct_t)+1];
unsigned char SuperPowerTx_buffer[sizeof(SendStruct_t)];
u8 superPowerOffline,superPowerOfflineCheck;
RecvStruct_t RecvStruct; 
SendStruct_t SendStruct;

vision_Recv_Struct_t vision_RecvStruct;
vision_Send_Struct_t vision_SendStruct;

DMA_InitTypeDef vision_Rx_DMA_InitStructure;
DMA_InitTypeDef vision_Tx_DMA_InitStructure;

//USART1
void SuperPower_USART_Init(void)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
	
	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); //1

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_Init(GPIOA,&GPIO_InitStructure);
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
		GPIO_Init(GPIOA,&GPIO_InitStructure);

		USART_DeInit(USART1);
		USART_InitStructure.USART_BaudRate = 460800;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_Init(USART1,&USART_InitStructure);				
		USART_ITConfig(USART1,USART_IT_IDLE,ENABLE);  
		USART_Cmd(USART1,ENABLE);

		DMA_DeInit(DMA1_Channel5);
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->DR);		//外设基地址
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)SuperPowerRx_buffer; 		//内存基地址
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;		//数据传输方向：外设到内存
		DMA_InitStructure.DMA_BufferSize = sizeof(SuperPowerRx_buffer);			//dma缓存大小
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设地址不变
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;		//内置地址寄存器递增
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//数据宽度为八位
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			//数据宽度为八位
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;					//工作模式为？？？
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;			//dma通道拥有高优先级
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	//非内存到内存传输
		DMA_Init(DMA1_Channel5,&DMA_InitStructure);
		USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);
		DMA_Cmd(DMA1_Channel5,ENABLE);
		
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

}


void USART1_IRQHandler(void)
{
    uint8_t data;
    if(USART_GetITStatus(USART1,USART_IT_IDLE) != RESET)
    {
        data = USART1->SR;
        data = USART1->DR;
        DMA_Cmd(DMA1_Channel5,DISABLE);
        if(SuperPowerRx_buffer[0] == 0xa5 && SuperPowerRx_buffer[sizeof(SuperPowerRx_buffer)-2] == 0xD2)
        {
			memcpy(&RecvStruct,SuperPowerRx_buffer,sizeof(RecvStruct_t));//////
			superPowerOfflineCheck = 0;
			superPowerOffline = 0;
        }
        DMA_SetCurrDataCounter(DMA1_Channel5, sizeof(SuperPowerRx_buffer));
        DMA_Cmd(DMA1_Channel5,ENABLE);
        USART_ClearITPendingBit(USART1,USART_IT_IDLE);
    }
}


extern u8 Check_mode,ULTS_mode;
void USART_Send_to_SuperPower(float chassis_power,uint8_t max_chassis_power,uint16_t chassis_power_buffer,uint8_t powerPath_Switch)
{
    int i;
//    SendStruct.FrameHeader = 0xA5;
//    SendStruct.chassis_power = chassis_power;
//    SendStruct.max_chassis_power = max_chassis_power;
//    SendStruct.chassis_power_buffer = chassis_power_buffer;
//	SendStruct.PowerPath_Switch=powerPath_Switch;
//	SendStruct.Check_Mode = Check_mode;
//	SendStruct.ULTS_Mode = ULTS_mode;
//	SendStruct.FrameTailer = 0xD2;
		vision_SendStruct.frameHeader = 0xA5;
	  vision_SendStruct.enemyColor = 1;
    Append_CRC16_Check_Sum((u8 *)&vision_SendStruct, sizeof(vision_SendStruct));

    memcpy(SuperPowerTx_buffer,&vision_SendStruct,sizeof(vision_Send_Struct_t));
    for(i = 0; i < sizeof(SendStruct_t); i++)
    {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
        USART1->DR = SuperPowerTx_buffer[i];
    }

}





