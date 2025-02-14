#ifndef __MOTOR_MT_DLC__
#define __MOTOR_MT_DLC__

#include "schedule.h"
#include "board.h"
#include "motor.h"
#include "can.h"

class MT_Can
{
private:
	CanInfo *Info;
	void Motor_Offset(u8 can_x, CanRxMsg *_CanRxMsg);
public:
	void Motor_Information_Calculate(u8 can_x, CanRxMsg *_CanRxMsg);
	static uint8_t IsDJIMotorCheck(CanRxMsg _CanRxMsg);
};

extern MT_Can MT_can1;
extern MT_Can MT_can2;
extern MotorList MT_motorlist[2][32];


void MT_Motor_Init(Motor* MT_Motor,CAN_TypeDef *_Canx,uint32_t _Std_ID);
void MT_GetInfomation(Motor * MT_Motor);
void MT_ctrlCurrent(float current,Motor * MT_Motor);



#endif