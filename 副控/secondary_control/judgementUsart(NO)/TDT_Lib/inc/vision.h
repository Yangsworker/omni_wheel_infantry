#ifndef __VISION_H
#define	__VISION_H

#include "board.h"

typedef __packed struct
{
    uint8_t FrameHeader;
    float chassis_power;
    uint8_t max_chassis_power;
    uint16_t chassis_power_buffer;
	uint8_t PowerPath_Switch;	    //是否开启电容
	uint8_t Check_Mode;
	uint8_t ULTS_Mode;
	uint8_t FrameTailer;
}SendStruct_t;

typedef __packed struct
{
    uint8_t FrameHeader;
    float capacitance_percentage;//电容容量百分比
    float voltage;//电容电压
    float local_Power;//功率模块计算功率
	uint8_t FrameTailer;
}RecvStruct_t;

extern RecvStruct_t RecvStruct; 
extern SendStruct_t SendStruct;


void SuperPower_USART_Init(void);
void USART_Send_to_SuperPower(float chassis_power,uint8_t max_chassis_power,uint16_t chassis_power_buffer,uint8_t powerPath_Switch);

#endif /* __USART1_H */
