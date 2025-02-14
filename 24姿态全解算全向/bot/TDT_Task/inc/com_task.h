#ifndef _COM_ 
#define _COM_

#include "board.h"
#include "can.h"

enum _myPack{
	Jgmt,Local,Ctrl,Display,Offset
};

/*************************底盘->云台的数据包*************************/

#pragma pack(1)
//高频包500HZ
typedef union 
{
	uint8_t data[8];
	struct{
		uint8_t jgmtOfflineFlag:1;		//裁判系统离线标志
		uint16_t shooterHeatRemainA:10;	//当前A枪剩余热量600
		uint16_t underWaringFlag:1;		//受判罚标志位【视野遮挡，此时视觉拥有绝对权限】
		uint16_t bulletSpeed:12;		//当前弹速[m/s]*100(float in judgement)		
		uint8_t bulletCount:6;			//发弹量，仅提供跳变，绝对值意义不大
		uint16_t powerlimitKp:10; //功率限制系数
		int16_t yaw_dps;   //陀螺速度
	};
	//remain_b:24
}JgmtAbout;//0x400
#pragma pack()

#pragma pack(1)
//超低频包10HZ
typedef union 
{
	uint8_t data[8];
	struct{
		uint8_t myColor:1;				//裁判系统读到的当前自己的颜色 0:红 1:蓝
		
		uint16_t shooterSpeedLimitA:2; //弹速上限[m/s] 0:15m/s | 1:18m/s | 2:30m/s
		uint16_t shooterCoolingLimitA:10; //A枪热量上限600
		uint16_t shooterCoolingRateA:10; //A枪每秒冷却值80*5*1.5
		
		uint16_t shooterSpeedLimitB:2; //弹速上限[m/s] 0:15m/s | 1:18m/s | 2:30m/s
		uint16_t shooterCoolingLimitB:10; //A枪热量上限600
		uint16_t shooterCoolingRateB:10; //A枪每秒冷却值80*5*1.5
	};
	//remain_b:19
}LocalAbout;//0x401
#pragma pack()

/*************************云台->底盘的数据包*************************/

#pragma pack(1)
//高频包500HZ
typedef union
{
	uint8_t data[8];
	struct{
    
		uint8_t if_Shift:1;		//是否按下shift
		uint8_t if_Ctrl:1;	  //是否按下ctrl
		uint8_t SpeedUp_level:1; //加速等级		
		uint8_t sysRST:1;		//底盘主控重启
		uint8_t UI_Init:1;
		
	};
	//remain_b:4
}CtrlAbout;//0x500
#pragma pack()

#pragma pack(1)
//低频包50Hz
typedef union
{
	uint8_t data[8];
	struct{	
		int8_t visYawError:7;		//视觉Y轴偏差角度
		uint16_t friLSpd:13;		//左摩擦轮转速
		uint16_t friRSpd:13;		//右摩擦轮转速
		uint16_t pitchCode:13;		//Pitch轴电机编码
		
		uint8_t visionMode:3;		//视觉模式 0:未开启|1:跟随|2:陀螺|3:反符|4:小符|5:大符
		uint8_t visState:2;			//视觉状态:0-视觉离线|1-视觉无目标|2-视觉有目标|3-视觉可开火
		
		uint8_t ready2Fire:1;		//开火许可
		uint8_t feedPos:3;			//拨盘位置
		uint8_t Firefrequency:2;	//0-单发|1-10|2-20
		
		uint8_t enemyID:4;			//瞄准目标的ID
		uint8_t CheckMode:1;		//检录模式
		uint8_t longFocusSwitch:1;	//长焦标记
	};
	//remain_b:1
}DisplayAbout;//0x501
#pragma pack()


/**********************************************************/
	/*TX*/
void Com_Task(_myPack myPack);		//信息从贯通CAN发送
	/*RX*/
void CanInfo_Handle(CanRxMsg ThCanMsg);	//贯通CAN信息解析

/****底盘->云台的联合包****/
extern JgmtAbout JgmtPack;
extern LocalAbout LocalPack;
/****云台->底盘的联合包****/
extern CtrlAbout CtrlPack;
extern DisplayAbout DisplayPack;

extern CAN_TypeDef *ThCan; //贯通Can通道

#endif