#include "myRC_task.h"
#include "dbus.h"
#include "Gimbal_task.h"
#include "com_task.h"
#include "my_math.h"
#include "vision.h"
#include "fire_task.h"

MyRC myRC;

MyRC::MyRC()
{
	visionKick = new doubleKickJudge;
}
/**
 * @brief 控制值获取
*/
int v_time = 1;
bool static_rotate = 0, static_armor = 0, static_buff = 0; //遥控切换模式
uint8_t Change_CD;   //单发/连发转化冷却
int AutoFire_time = 0;
u8 last_fireMode = 2, last_visionMode = 0; //默认10弹频
bool last_RHL = 0; //记录上次高低陀螺是否开启
void MyRC::myRC_Update(void)
{	
/*********************底盘控制***************************/
	mySpdSetX = RC.Key.CH[3] / 35.0f;
	mySpdSetX += (RC.Key.W-RC.Key.S) * 10.0f;//键鼠控制量
	mySpdSetX *= !(RC.Key.CTRL&&RC.Key.SHIFT);//同时按Ctrl+Shift时禁用移动
	mySpdSetX *= RC.Key.CH[3]>-660;//防止遥控器挂住后后冲起步
	mySpdSetX = LIMIT(mySpdSetX,-20,20);//前后设定
	
	mySpdSetY = RC.Key.CH[2] / 40.0f;
	mySpdSetY += (RC.Key.D-RC.Key.A) * 9.0f;//键鼠控制量
	mySpdSetY *= !(RC.Key.CTRL&&RC.Key.SHIFT);//同时按Ctrl+Shift时禁用移动
	mySpdSetY = LIMIT(mySpdSetY,-20,20);//前后设定
	
//是否按Shift
	if_Shift = RC.Key.SHIFT; 
//是否按Ctrl
	if_Ctrl = RC.Key.CTRL;
//加速等级
	if(if_Shift)
	{
		SpeedUp_level = 1;
	}
//是否取消底盘跟随(开启底盘躲避/开启打符模式)
	defollowFlag = (AvoidMode > 0) || (visionMode == 3);
//检录模式开启（电容）
  CheckMode = (RC.Key.SW1 == RCS::Up && deforceFlag);   //脱力时左拨杆置上进入检录模式
/*********************云台控制***************************/
	mySpdSetZ = RC.Key.CH[0] *-0.0009f; //yaw
	mySpdSetP = -RC.Key.CH[1] *0.0005f; //pitch
	mySpdSetZ += RC.Key.CH[6] *-0.0024f;
	mySpdSetP += RC.Key.CH[7] *0.0008f;
	mySpdSetZ = LIMIT(mySpdSetZ,-2.0,2.0);//Yaw轴控制  //2.044
	mySpdSetP = LIMIT(mySpdSetP,-1.022,1.022);//Pitch轴控制
	
//**********视觉**************
visionMode = visionKick->doubleKickVal(RC.Key.CH[9]);
	if((RC.Key.SW1 == RCS::Up && deforceFlag) || (RC.Key.SHIFT && RC.Key.CTRL && RC.Key.Q) || static_buff)
	{
     visionMode = 3;
	  static_buff = 1;	
		static_armor = 0;
		static_rotate = 0;	
	}
	if((RC.Key.SW1 == RCS::Down && deforceFlag) || static_armor)
	{
     visionMode = 1;
	  static_armor = 1;	
		static_rotate = 0;	
		static_buff = 0;
	}
  if(((RC.Key.CH[11] < -400)&& deforceFlag) || static_rotate)
	{
     visionMode = 2;
	  static_rotate = 1;	
		static_armor = 0;
		static_buff = 0;
	}
	if((deforceFlag && RC.Key.SW1 == RCS::Mid && (RC.Key.CH[11] == 0)) || RC.Key.Z)  //按Z或脱力置中上力推退出所有视觉模式
	{
		visionMode = 0;
		static_armor = 0;
		static_rotate = 0;	
		static_buff = 0;
	}
	//发送
	vision_SendStruct.SpiningShoot = (visionMode == 2);
	vision_SendStruct.baseShootMode = RC.Key.X;  //建筑击打模式
	if(visionMode == 3)
	{
		vision_SendStruct.energyBeatMode = 1;
		vision_SendStruct.bufftype = 0;   //小符
		if(RC.Key.CH[9])
		{
			vision_SendStruct.bufftype = 1; //大符
		}
	}
	else
	{
		vision_SendStruct.energyBeatMode = 0;
	}
	vision_SendStruct.EnableAutoAim = (visionMode == 1);
/*********************发射控制***************************/	
	//修改弹频(默认10弹频)
	if(!Change_CD && RC.Key.F)   //0-单发 1-10弹频 2-20弹频
  {
		fire.Fire_Mode++;		
		if(fire.Fire_Mode > 2)
		{fire.Fire_Mode = 1;}
	  Change_CD = 1;
	} 
	if(Change_CD ) {Change_CD++;}
	if(Change_CD > 100) {Change_CD = 0;}
	//反陀螺默认自动10弹频
	if(visionMode == 2) {fire.Fire_Mode = 3;}  
	Firefrequency = fire.Fire_Mode;
	if(visionMode != 2 && last_visionMode == 2) {fire.Fire_Mode = last_fireMode; last_fireMode = fire.Fire_Mode;} //脱离反陀螺模式时恢复原弹频
	if(visionMode == 3) {fire.Fire_Mode = 0;}  //开符模式默认1弹频
	if(visionMode != 3 && last_visionMode == 3) {fire.Fire_Mode = 1;}  //退出开符模式自动切换10弹频
//	Firefrequency = fire.Fire_Mode;
//	if(visionMode != 2 && last_visionMode == 2) {fire.Fire_Mode = last_fireMode; last_fireMode = fire.Fire_Mode;} //脱离反陀螺模式时恢复原弹频
	
//	if(visionMode == 2 && (ABS(gimbal.eulerAlgSet[YAW]-gimbal.WorldAlg[YAW]))<0.3 && !vision_RecvStruct.no_Obj && !visionInfo.offlineFlag)
//	{
//		AutoFire = 1;
//	}
//	else AutoFire = 0;
	
	
	
	if(!vision_RecvStruct.no_Obj && !visionInfo.offlineFlag && vision_RecvStruct.beat && visionMode == 2 && RC.Key.SW1 == RCS::Mid)
	{
		AutoFire = 1;
	}
	else AutoFire = 0;
  //打符自动开火(需要左拨杆置中)
	if(!vision_RecvStruct.no_Obj && !visionInfo.offlineFlag && vision_RecvStruct.beat && visionMode == 3 && RC.Key.SW1 == RCS::Mid)
	{
		AutoFire_buff = 1;
	}
	else AutoFire_buff = 0;
	last_visionMode = visionMode;
/*********************系统相关***************************/		
	topCtrlRst 	= RC.Key.CTRL*RC.Key.SHIFT*RC.Key.C*deforceFlag;	//上主控重启
	botCtrlRst 	= RC.Key.SHIFT*RC.Key.C*deforceFlag;			//下主控重启
	
		if(myRC.topCtrlRst)
	{
		__disable_irq(); //关闭所有中断
		NVIC_SystemReset(); //复位
		while (1)
		{
		} //仅等待复位
	}
	
	
}





