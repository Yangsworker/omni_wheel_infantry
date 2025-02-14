#include "com_task.h"
#include "led_task.h"
#include "Gimbal_task.h"
#include "fire_task.h"
#include "multi_imu.h"
#include "myRC_task.h"
#include "dbus.h"
#include "vision.h"
#include "judgement.h"
#include "schedule.h"

CAN_TypeDef *ThCan = CAN1;//设定贯通CAN


/****底盘->云台的联合包****/
JgmtAbout JgmtPack;
LocalAbout LocalPack;
/****云台->底盘的联合包****/
CtrlAbout CtrlPack;
DisplayAbout DisplayPack;

/*******************自定义处理函数*******************/

extern MultiImu *boardImu;

//信息从贯通CAN发送
void Com_Task(_myPack myPack)
{
	static int32_t lastCode; 
	if(myPack == Ctrl)
	{
		CtrlPack.if_Shift = myRC.if_Shift;		//是否按下shift
		CtrlPack.if_Ctrl = myRC.if_Ctrl;		  //是否按下ctrl
		CtrlPack.SpeedUp_level = myRC.SpeedUp_level;  //加速等级
		
		CtrlPack.sysRST = myRC.botCtrlRst;		//底盘主控重启

		/*在上方进行信息预处理*/
		canTx(CtrlPack.data,CAN1,0x500);
	}
/***********************************************************************/	
	else if(myPack == Display)
	{
		DisplayPack.visYawError = (uint16_t)(vision_RecvStruct.Yaw-gimbal.WorldAlg[YAW]);//视觉Y轴偏差角度
		DisplayPack.friLSpd = (uint16_t)(ABS(fire.Firemotor[0]->canInfo.speed)); //左摩擦轮转速
		DisplayPack.friRSpd = (uint16_t)(ABS(fire.Firemotor[1]->canInfo.speed)); //右摩擦轮转速
		DisplayPack.pitchCode = gimbal.Pitch->canInfo.encoder; //Pitch轴电机编码
		
		DisplayPack.visionMode = myRC.visionMode;		//视觉模式 0:未开启|1:跟随|2:陀螺|3:反符|4:小符|5:大符
		if(DisplayPack.visionMode == 3) DisplayPack.visionMode += (1+vision_SendStruct.bufftype);   //进入开符模式时判断大小符
//		//视觉状态:0-视觉离线|1-视觉无目标|2-视觉有目标|3-视觉可开火
//		/*| VisState|  beat |no_Obj	|offlineFlag|*/
//		/*|	0		|	0	|	1	|	1		|*/
//		/*|	1		|	0	|	1	|	0		|*/
//		/*|	2		|	0	|	0	|	0		|*/
//		/*|	3		|	1	|	0	|	0		|*/
		DisplayPack.visState  =  (ABS(DisplayPack.visYawError) < 0.3 && !vision_RecvStruct.no_Obj);
		DisplayPack.visState += !vision_RecvStruct.no_Obj;//视觉无目标，标志位置零
	  ++DisplayPack.visState *= !visionInfo.offlineFlag;//视觉离线，标志位置零

		DisplayPack.Firefrequency = myRC.Firefrequency; //0-单发|1-10弹频|2-20弹频
		
		DisplayPack.enemyID = vision_RecvStruct.objID; //瞄准目标的ID
		DisplayPack.CheckMode = myRC.CheckMode;		//检录模式
		
		/*在上方进行信息预处理*/
		canTx(DisplayPack.data,ThCan,0x501);
	}
}

//贯通CAN信息解析
void CanInfo_Handle(CanRxMsg ThCanMsg)
{
	switch(ThCanMsg.StdId)
	{
		case 0X400:
			botCtrlOfflineCnt = 0;
			memcpy(JgmtPack.data,ThCanMsg.Data,8);
			break;
		case 0X401:
			botCtrlOfflineCnt = 0;
			memcpy(LocalPack.data,ThCanMsg.Data,8);
			break;
		default:
			break;
	}
}


