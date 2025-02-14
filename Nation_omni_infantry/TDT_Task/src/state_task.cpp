/*
* ��Ϊ״̬����ʵ��Ϊ�븱��ͨѶ�����Ӿ�ͨѶ����ط�������Ӿ�ƫ�á������Լ켰���ֲ�����ģ����Ƶļ�λ
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
struct Enemyposition enemyposition;  //����λ��
extern Can1Feedback can1Feedback;    //���ո��ز���ϵͳ��Ϣ
struct Enemyradius enemyradius;      //���˰뾶����
struct ROBOTCommunication RobotCommunication;  //���佻���ṹ��
struct CanStateStruct_2 canStateStruct_2;
StateCtrl stateCtrl;
struct CanStateStruct canStateStruct;//can��Ϣ���ͽṹ��
 
 int number = 0;
#define CAN_COUNT 3
int qqq = 0;
//-----------------------------------------------ui״̬����------------------------------------------------
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

	number++;  //2ms��һ��
	if (number == 1)
	{
		stateCtrl.lastDeforceFlag = deforceFlag;
		uint32_t tmpGet;
		if (chassis.rotateFlag)  //С����
			canStateStruct.chassisMode = 1;
		else if (chassis.swingFlag)  //ҡ��
			canStateStruct.chassisMode = 2;
		else if (chassis.flexRotate)//�Զ����
			canStateStruct.chassisMode = 3;
		else //Ĭ�ϵ��̸��� 
			canStateStruct.chassisMode = 0;
	canStateStruct.unLimitedFired = fire.unLimitedFired;    //�����������(��ûд)
	canStateStruct.visionLock = !vision_RecvStruct.no_Obj;   //�Ӿ�����Ŀ�꼴��
	canStateStruct.visionBeat = ptz.autofireFlag;           //�Ӿ�������
	canStateStruct.localHeat = fire.jgmtHeat;             //��������
	canStateStruct.doubleShift = power.speedUpLevel;         //���ٵȼ�
	canStateStruct.visionOffline = visionInfo.offlineFlag;   //�Ӿ����߱�־λ
	canStateStruct.useCap = power.useCap;               //�Ƿ��õ���
	canStateStruct.checkMode = power.Check_Mode;        //��¼ģʽ
	canStateStruct.ULTSMode = power.ULTS_Mode;          //����ģʽ
	canStateStruct.powerDownLimit = power.powerDownLimit;//������������
		if( fire.Fire_Mode)  //��Ƶ
		{
			if(fire.DialFireTime == 50) //10��/s
			{
			  canStateStruct.fireFrequencyMode = 1;				
			}
			else if(fire.DialFireTime == 25) //20��/s
			{
			  canStateStruct.fireFrequencyMode = 2;				
			}
		} 
		else
				canStateStruct.fireFrequencyMode = 0;  //���������ˢ�£�
		
		canStateStruct.elevationAngle = ABS(ptz.Roll->canInfo.totalAngle); //pitch�Ƕ�

		if(fire.Firemotor[0]->canInfo.lostFlag || fire.Firemotor[1]->canInfo.lostFlag)
		{
			canStateStruct.frictionState = 0;     //Ħ��������Ϊ0--��
		}
		else if(ABS(fire.Firemotor[0]->canInfo.speed) > 3500 && ABS(fire.Firemotor[1]->canInfo.speed) > 3500)
		{
			canStateStruct.frictionState = 2;     //Ħ���ֿ���Ϊ2--��
		}
		else
		{
			canStateStruct.frictionState = 1;     //Ħ����δ����Ϊ1--��
		}
		//�Ӿ�ʶ����ʾ����ߵ�Ѫ��
		if(!qqq)
		{
		 canStateStruct.enemyID = 1; 		
			qqq = 1;
		}
		if(!vision_RecvStruct.no_Obj)
		{
		  canStateStruct.enemyID = vision_RecvStruct.objID ;                                       //�з�ID
		}
   //��̨�Ƕ�
	float tmpyawAngleForChassis;
  tmpyawAngleForChassis=chassis.getGimbalAngle(); //���ƽǶ���-������
	if(tmpyawAngleForChassis > 180) {tmpyawAngleForChassis = tmpyawAngleForChassis - 360;}
  canStateStruct.chassisAngle = (int8_t)(tmpyawAngleForChassis / 360 * 255);
		
	canStateStruct.ammoCoverOpen = ammoCover.coverOpen;  //���ո��Ƿ��
//	canStateStruct.blockError = (fire.shootErrorFlag && fire.shootErrorCheckTime < fire.shootErrorCheckTimeMax);
	canStateStruct.sprocketMotorOffline = fire.Dial->canInfo.lostFlag;//���ո��Ƿ�����
	canStateStruct.flexRotate = chassis.flexRotate;                   //�Ƿ����������� -------------------------------


      //�Ӿ�ģʽ
      if(VM == rotate)  //��ɫ
			{
				canStateStruct.VisionMode = 2;
			}
			else if(VM == bigBUFF)  //��ɫ
			{
				canStateStruct.VisionMode = 1;
			}
			else if(VM == smallBUFF)  //��ɫ
			{
				canStateStruct.VisionMode = 3;
			}
			else if(VM == ANTIBUFF)   //��ɫ
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
//---------------------------------------�Ӿ���Ϣ����------------------------------------------------
void State_Ctrl_RC_Info(void) //���Ӿ�������Ϣ
{
	vision_SendStruct.energyBeatMode = 0;
	vision_SendStruct.SpiningShoot = 0;
	vision_SendStruct.anti_arch = 0;
	vision_SendStruct.EnableAutoAim = 0;
	if(RC.Key.X)  //װ���Ӿ�ģʽ�³���R���������ģʽ
	{vision_SendStruct.anti_arch = 1;}
	if((CtrlPlan == Vision) && ((VM == ARMOR) || (VM == rotate)))
	{
		vision_SendStruct.EnableAutoAim = 1;
	}
	switch(VM)
	{
		case ARMOR: //װ��
			break;
		case bigBUFF: //���
			vision_SendStruct.energyBeatMode = 1;
		  vision_SendStruct.buffbeatmode = 1;
			break;
		case smallBUFF: //С��
			vision_SendStruct.energyBeatMode = 1;
		  vision_SendStruct.buffbeatmode = 0;
			break;
		case rotate:  //С����
			vision_SendStruct.SpiningShoot = 1;
			break;
		case ANTIBUFF: //����
			vision_SendStruct.energyBeatMode = 2;
			break;
	}
	
}
//------------------------------------------------ǹ��ƫ��-----------------------------------------------
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
	//����ƫ��


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
	//���ƫ��
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
//--------------------------------------------------�����Լ�-----------------------------------------------------
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
	if(RC.Key.SW1 == RCS::Mid)  //���can1
			{
				iscan2 = 0;
			}
	if(RC.Key.SW1 == RCS::Down)  //���can2
			{
				iscan2 = 1;
			}
	if(RC.Key.SW1 == RCS::Up) //�������ģ��(���أ��Ӿ�����������)
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
//-------------------------------------------------------���ּ�λ-----------------------------------------
//C+SHIFT+CTRL--��������
#include "KeyProcess.h"
KeyProcess reset(
	KEY_C | KEY_SHIFT | KEY_CTRL, [](uint32_t *) {
		__set_FAULTMASK(1); //�ر������ж�
		NVIC_SystemReset(); //
		while (1)
		{
		} //����
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

