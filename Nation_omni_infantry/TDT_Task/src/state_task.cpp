/*
* 名为状态机，实际为与副控通讯、与视觉通讯、电控方面调整视觉偏置、快速自检及部分不属于模块控制的键位
*/


#include "state_task.h"
#include "ptz_task.h"
#include "chassis_task.h"
#include "ammo_cover_task.h"
#include "fire_task.h"
#include "vision.h"
#include "dbus.h"
#include "can.h"
#include "imu_task.h"
#include "parameter.h"
#include "led_task.h"

FastSelfCheck fastselfcheck;
struct Enemyposition enemyposition;  //敌人位置
extern Can1Feedback can1Feedback;    //接收副控裁判系统消息
struct Enemyradius enemyradius;      //敌人半径数据
struct ROBOTCommunication RobotCommunication;  //机间交互结构体
struct CanStateStruct_2 canStateStruct_2;
StateCtrl stateCtrl;
struct CanStateStruct canStateStruct;//can消息发送结构体
 
 int number = 0;
#define CAN_COUNT 3
int qqq = 0;
//-----------------------------------------------ui状态发送------------------------------------------------
void State_Ctrl()
{
	if(can1Feedback.jgmtOfflineCheck > 200)
	{
		can1Feedback.jgmtOffline = 1;
	}
	else
	{
		can1Feedback.jgmtOfflineCheck++;
	}

	number++;  //2ms发一次
	if (number == 1)
	{
		stateCtrl.lastDeforceFlag = deforceFlag;
		uint32_t tmpGet;
		if (chassis.rotateFlag)  //小陀螺
			canStateStruct.chassisMode = 1;
		else if (chassis.swingFlag)  //摇摆
			canStateStruct.chassisMode = 2;
		else if (chassis.flexRotate)//自动躲避
			canStateStruct.chassisMode = 3;
		else //默认底盘跟随 
			canStateStruct.chassisMode = 0;
	canStateStruct.unLimitedFired = fire.unLimitedFired;    //解除开火限制(还没写)
	canStateStruct.visionLock = !vision_RecvStruct.no_Obj;   //视觉发现目标即锁
	canStateStruct.visionBeat = ptz.autofireFlag;           //视觉允许开火
	canStateStruct.localHeat = fire.jgmtHeat;             //本地热量
	canStateStruct.doubleShift = power.speedUpLevel;         //加速等级
	canStateStruct.visionOffline = visionInfo.offlineFlag;   //视觉离线标志位
	canStateStruct.useCap = power.useCap;               //是否用电容
	canStateStruct.checkMode = power.Check_Mode;        //检录模式
	canStateStruct.ULTSMode = power.ULTS_Mode;          //特殊模式
	canStateStruct.powerDownLimit = power.powerDownLimit;//功率限制下限
		if( fire.Fire_Mode)  //弹频
		{
			if(fire.DialFireTime == 50) //10发/s
			{
			  canStateStruct.fireFrequencyMode = 1;				
			}
			else if(fire.DialFireTime == 25) //20发/s
			{
			  canStateStruct.fireFrequencyMode = 2;				
			}
		} 
		else
				canStateStruct.fireFrequencyMode = 0;  //单发（最后刷新）
		
		canStateStruct.elevationAngle = ABS(ptz.Roll->canInfo.totalAngle); //pitch角度

		if(fire.Firemotor[0]->canInfo.lostFlag || fire.Firemotor[1]->canInfo.lostFlag)
		{
			canStateStruct.frictionState = 0;     //摩擦轮离线为0--黑
		}
		else if(ABS(fire.Firemotor[0]->canInfo.speed) > 3500 && ABS(fire.Firemotor[1]->canInfo.speed) > 3500)
		{
			canStateStruct.frictionState = 2;     //摩擦轮开启为2--白
		}
		else
		{
			canStateStruct.frictionState = 1;     //摩擦轮未开启为1--红
		}
		//视觉识别显示面对者的血量
		if(!qqq)
		{
		 canStateStruct.enemyID = 1; 		
			qqq = 1;
		}
		if(!vision_RecvStruct.no_Obj)
		{
		  canStateStruct.enemyID = vision_RecvStruct.objID ;                                       //敌方ID
		}
   //云台角度
	float tmpyawAngleForChassis;
  tmpyawAngleForChassis=chassis.getGimbalAngle(); //控制角度在-Π到Π
	if(tmpyawAngleForChassis > 180) {tmpyawAngleForChassis = tmpyawAngleForChassis - 360;}
  canStateStruct.chassisAngle = (int8_t)(tmpyawAngleForChassis / 360 * 255);
		
	canStateStruct.ammoCoverOpen = ammoCover.coverOpen;  //弹舱盖是否打开
//	canStateStruct.blockError = (fire.shootErrorFlag && fire.shootErrorCheckTime < fire.shootErrorCheckTimeMax);
	canStateStruct.sprocketMotorOffline = fire.Dial->canInfo.lostFlag;//弹舱盖是否离线
	canStateStruct.flexRotate = chassis.flexRotate;                   //是否开启差速陀螺 -------------------------------


      //视觉模式
      if(VM == rotate)  //青色
			{
				canStateStruct.VisionMode = 2;
			}
			else if(VM == bigBUFF)  //橙色
			{
				canStateStruct.VisionMode = 1;
			}
			else if(VM == smallBUFF)  //绿色
			{
				canStateStruct.VisionMode = 3;
			}
			else if(VM == ANTIBUFF)   //粉色
			{
				canStateStruct.VisionMode = 4;
			}
			else  
			{
				canStateStruct.VisionMode = 0;
			}
			canStateStruct.anti_arch = vision_SendStruct.anti_arch;
			
			canStateStruct.uiinit = RC.Key.C;
			
		canTx((u8 *)&canStateStruct, CAN1, 0x141);
	}
	if(number == 2)
	{
		number = 0;
		canTx((u8 *)&canStateStruct_2, CAN1, 0x151);
	}
}
//---------------------------------------视觉消息发送------------------------------------------------
void State_Ctrl_RC_Info(void) //给视觉发送消息
{
	vision_SendStruct.energyBeatMode = 0;
	vision_SendStruct.SpiningShoot = 0;
	vision_SendStruct.anti_arch = 0;
	vision_SendStruct.EnableAutoAim = 0;
	if(RC.Key.X)  //装甲视觉模式下长按R进入击打建筑模式
	{vision_SendStruct.anti_arch = 1;}
	if((CtrlPlan == Vision) && ((VM == ARMOR) || (VM == rotate)))
	{
		vision_SendStruct.EnableAutoAim = 1;
	}
	switch(VM)
	{
		case ARMOR: //装甲
			break;
		case bigBUFF: //大符
			vision_SendStruct.energyBeatMode = 1;
		  vision_SendStruct.buffbeatmode = 1;
			break;
		case smallBUFF: //小符
			vision_SendStruct.energyBeatMode = 1;
		  vision_SendStruct.buffbeatmode = 0;
			break;
		case rotate:  //小陀螺
			vision_SendStruct.SpiningShoot = 1;
			break;
		case ANTIBUFF: //反符
			vision_SendStruct.energyBeatMode = 2;
			break;
	}
	
}
//------------------------------------------------枪口偏置-----------------------------------------------
int a_record = 0, d_record = 0;
void StateCtrl::changeVisionError() 
{
//	chassis.Lockchassis = 0;
	canStateStruct_2.e_vserrFlag = (RC.Key.CTRL && RC.Key.SHIFT && !RC.Key.Q);
	if(canStateStruct_2.e_vserrFlag)
	{
//		chassis.Lockchassis = 1;
		if(!RC.Key.A) {a_record = 0;}
		if(!RC.Key.D) {d_record = 0;}
		if(RC.Key.A && !a_record)
		{
			if(!canStateStruct_2.e_vserrMode)
				 canStateStruct_2.e_vserrMode=3;
			else if(canStateStruct_2.e_vserrMode>=1)
		     canStateStruct_2.e_vserrMode-=1;
			a_record = 1;
		}
		if(RC.Key.D && !d_record)
		{
			if(canStateStruct_2.e_vserrMode == 3)
				canStateStruct_2.e_vserrMode=0;
			else if(canStateStruct_2.e_vserrMode<3)
		     canStateStruct_2.e_vserrMode+=1;
			d_record = 1;
		}
	}
	if(!RC.Key.W && !RC.Key.S)
	{
		addYawFlag = 0;
		  addPitchFlag = 0;
		  addYawBuffFlag = 0;
		  addPitchBuffFlag = 0;
	switch(canStateStruct_2.e_vserrMode)
	{
		case 0:
			addYawFlag = 1;
		  break;
		case 1:
			addPitchFlag = 1;
		  break;
		case 2:
			addPitchBuffFlag = 1;
		  break;
		case 3:
			addYawBuffFlag = 1;
		  break;
	}
  }
	//自瞄偏置


	if (addYawFlag && canStateStruct_2.e_vserrFlag)
	{
		if(RC.KeyPress.W)
		{canStateStruct_2.visionyawError++;
		ptz_error[vision_yaw] ++;
		addYawFlag = 0;}
		if(RC.KeyPress.S)
		{canStateStruct_2.visionyawError--;
		ptz_error[vision_yaw]--;
		addYawFlag = 0;}
		addYawFlag ==1? (ptz_error[vision_yaw]>200?ptz_error[vision_yaw] = 200:ptz_error[vision_yaw] = ptz_error[vision_yaw])
		       :(ptz_error[vision_yaw] < 1?ptz_error[vision_yaw] = 1:ptz_error[vision_yaw] = ptz_error[vision_yaw]);
	}
	else if (addPitchFlag && canStateStruct_2.e_vserrFlag)
	{if(RC.KeyPress.W)
		{canStateStruct_2.visionpitchError++;
		ptz_error[vision_pitch]++;
		addPitchFlag = 0;}
		if(RC.KeyPress.S)
		{canStateStruct_2.visionpitchError--;
		ptz_error[vision_pitch]--;
		addPitchFlag = 0;}
		addPitchFlag ==1? (ptz_error[vision_pitch]>200?ptz_error[vision_pitch] = 200:ptz_error[vision_pitch] = ptz_error[vision_pitch])
		       :(ptz_error[vision_pitch] < 1?ptz_error[vision_pitch] = 1:ptz_error[vision_pitch] = ptz_error[vision_pitch]);
		}
	//打符偏置
	else if (addYawBuffFlag && canStateStruct_2.e_vserrFlag)
	{if(RC.KeyPress.W)
		{canStateStruct_2.BuffyawError++;
		ptz_error[buff_yaw] ++;
		addYawBuffFlag = 0;}
		if(RC.KeyPress.S)
		{canStateStruct_2.BuffyawError--;
		ptz_error[buff_yaw]--;
		addYawBuffFlag = 0;}
		addYawBuffFlag ==1? (ptz_error[buff_yaw]>200?ptz_error[buff_yaw] = 200:ptz_error[buff_yaw] = ptz_error[buff_yaw])
		       :(ptz_error[buff_yaw] < 1?ptz_error[buff_yaw] = 1:ptz_error[buff_yaw] = ptz_error[buff_yaw]);
		}
	else if (addPitchBuffFlag && canStateStruct_2.e_vserrFlag)
	{if(RC.KeyPress.W)
		{canStateStruct_2.BuffpitchError ++;
		ptz_error[buff_pitch] ++;
		addPitchBuffFlag = 0;}
		if(RC.KeyPress.S)
		{canStateStruct_2.BuffpitchError--;
		ptz_error[buff_pitch]--;
		addPitchBuffFlag = 0;}
		addPitchBuffFlag ==1? (ptz_error[buff_pitch]>200?ptz_error[buff_pitch] = 200:ptz_error[buff_pitch] = ptz_error[buff_pitch])
		       :(ptz_error[buff_pitch] < 1?ptz_error[buff_pitch] = 1:ptz_error[buff_pitch] = ptz_error[buff_pitch]);
		}	
	
}
//--------------------------------------------------快速自检-----------------------------------------------------
u8 iscan2 = 0;
void FastSelfCheck::run()
{
	if(deforceFlag)
	{
		selfCheckFlag = 1;
	}
	if(selfCheckFlag == 0)
	{
		return;
	}
	if(RC.Key.SW1 != RC.LastKey.SW1)
	{
		systime = getSysTimeUs();
		laser.setError(0, LedES_ConstantDark);
	}
	if(timeIntervalFrom(systime) < 500000)
	{
		return;
	}
	if(!deforceFlag)
	{
		selfCheckFlag = 0;
		laser.setError(0,LedES_Disable);
		return;
	}
	if(RC.Key.SW1 == RCS::Mid)  //检查can1
			{
				iscan2 = 0;
			}
	if(RC.Key.SW1 == RCS::Down)  //检查can2
			{
				iscan2 = 1;
			}
	if(RC.Key.SW1 == RCS::Up) //检查其他模块(副控，视觉，超级电容)
	{ 
		if(can1Feedback.jgmtOffline)
		{
			laser.setError(0, 1);
			return;
		}
		if(visionInfo.offlineFlag)
		{
			laser.setError(0,2);
			return;
		}
		if(can1Feedback.SuperPowerOffline)
		{
			laser.setError(0,3);
			return;
		}
	}
	for(int i = 0; i < 8; i++)
			{
				if(motorList[iscan2][i].motorPoint == 0)
				{continue;}
				if(motorList[iscan2][i].motorPoint->getEnableMotor() == 0)
				{continue;}
				if(motorList[iscan2][i].motorPoint->canInfo.lostFlag)
				{
					laser.setError(0,i+1);
					return;
				}
			}
			laser.setError(0, LedES_ConstantDark);
}
//-------------------------------------------------------部分键位-----------------------------------------
//C+SHIFT+CTRL--重启程序
#include "KeyProcess.h"
KeyProcess reset(
	KEY_C | KEY_SHIFT | KEY_CTRL, [](uint32_t *) {
		__set_FAULTMASK(1); //关闭所有中断
		NVIC_SystemReset(); //
		while (1)
		{
		} //重启
	},
	NULL, NULL);
//KeyProcess uiReinit(
//	KEY_C, [](uint32_t *) {
//		canStateStruct.uiinit = 1;
//	},
//	NULL, NULL, 1, 1);
//KeyProcess visionOffsetDown(
//	KEY_CTRL | KEY_SHIFT | KEY_S, [](uint32_t *) {
//		stateCtrl.addPitchFlag = -1;
//	},
//	NULL, NULL, 1, 1);
//KeyProcess visionOffsetLeft(
//	KEY_CTRL | KEY_SHIFT | KEY_A, [](uint32_t *) {
//		stateCtrl.addYawFlag = 1;
//	},
//	NULL, NULL, 1, 1);
//KeyProcess visionOffsetRight(
//	KEY_CTRL | KEY_SHIFT | KEY_D, [](uint32_t *) {
//		stateCtrl.addYawFlag = -1;
//	},
//	NULL, NULL, 1, 1);
	
	
	
//KeyProcess BuffOffsetUp(
//	KEY_G | KEY_SHIFT | KEY_W, [](uint32_t *) {
//		stateCtrl.addPitchBuffFlag = 1;
//	},
//	NULL, NULL, 1, 1);
//KeyProcess BuffOffsetDown(
//	KEY_G | KEY_SHIFT | KEY_S, [](uint32_t *) {
//		stateCtrl.addPitchBuffFlag = -1;
//	},
//	NULL, NULL, 1, 1);

