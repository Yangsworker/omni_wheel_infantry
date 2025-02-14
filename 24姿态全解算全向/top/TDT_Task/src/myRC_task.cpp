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
 * @brief ����ֵ��ȡ
*/
int v_time = 1;
bool static_rotate = 0, static_armor = 0, static_buff = 0; //ң���л�ģʽ
uint8_t Change_CD;   //����/����ת����ȴ
int AutoFire_time = 0;
u8 last_fireMode = 2, last_visionMode = 0; //Ĭ��10��Ƶ
bool last_RHL = 0; //��¼�ϴθߵ������Ƿ���
void MyRC::myRC_Update(void)
{	
/*********************���̿���***************************/
	mySpdSetX = RC.Key.CH[3] / 35.0f;
	mySpdSetX += (RC.Key.W-RC.Key.S) * 10.0f;//���������
	mySpdSetX *= !(RC.Key.CTRL&&RC.Key.SHIFT);//ͬʱ��Ctrl+Shiftʱ�����ƶ�
	mySpdSetX *= RC.Key.CH[3]>-660;//��ֹң������ס������
	mySpdSetX = LIMIT(mySpdSetX,-20,20);//ǰ���趨
	
	mySpdSetY = RC.Key.CH[2] / 40.0f;
	mySpdSetY += (RC.Key.D-RC.Key.A) * 9.0f;//���������
	mySpdSetY *= !(RC.Key.CTRL&&RC.Key.SHIFT);//ͬʱ��Ctrl+Shiftʱ�����ƶ�
	mySpdSetY = LIMIT(mySpdSetY,-20,20);//ǰ���趨
	
//�Ƿ�Shift
	if_Shift = RC.Key.SHIFT; 
//�Ƿ�Ctrl
	if_Ctrl = RC.Key.CTRL;
//���ٵȼ�
	if(if_Shift)
	{
		SpeedUp_level = 1;
	}
//�Ƿ�ȡ�����̸���(�������̶��/�������ģʽ)
	defollowFlag = (AvoidMode > 0) || (visionMode == 3);
//��¼ģʽ���������ݣ�
  CheckMode = (RC.Key.SW1 == RCS::Up && deforceFlag);   //����ʱ�󲦸����Ͻ����¼ģʽ
/*********************��̨����***************************/
	mySpdSetZ = RC.Key.CH[0] *-0.0009f; //yaw
	mySpdSetP = -RC.Key.CH[1] *0.0005f; //pitch
	mySpdSetZ += RC.Key.CH[6] *-0.0024f;
	mySpdSetP += RC.Key.CH[7] *0.0008f;
	mySpdSetZ = LIMIT(mySpdSetZ,-2.0,2.0);//Yaw�����  //2.044
	mySpdSetP = LIMIT(mySpdSetP,-1.022,1.022);//Pitch�����
	
//**********�Ӿ�**************
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
	if((deforceFlag && RC.Key.SW1 == RCS::Mid && (RC.Key.CH[11] == 0)) || RC.Key.Z)  //��Z�����������������˳������Ӿ�ģʽ
	{
		visionMode = 0;
		static_armor = 0;
		static_rotate = 0;	
		static_buff = 0;
	}
	//����
	vision_SendStruct.SpiningShoot = (visionMode == 2);
	vision_SendStruct.baseShootMode = RC.Key.X;  //��������ģʽ
	if(visionMode == 3)
	{
		vision_SendStruct.energyBeatMode = 1;
		vision_SendStruct.bufftype = 0;   //С��
		if(RC.Key.CH[9])
		{
			vision_SendStruct.bufftype = 1; //���
		}
	}
	else
	{
		vision_SendStruct.energyBeatMode = 0;
	}
	vision_SendStruct.EnableAutoAim = (visionMode == 1);
/*********************�������***************************/	
	//�޸ĵ�Ƶ(Ĭ��10��Ƶ)
	if(!Change_CD && RC.Key.F)   //0-���� 1-10��Ƶ 2-20��Ƶ
  {
		fire.Fire_Mode++;		
		if(fire.Fire_Mode > 2)
		{fire.Fire_Mode = 1;}
	  Change_CD = 1;
	} 
	if(Change_CD ) {Change_CD++;}
	if(Change_CD > 100) {Change_CD = 0;}
	//������Ĭ���Զ�10��Ƶ
	if(visionMode == 2) {fire.Fire_Mode = 3;}  
	Firefrequency = fire.Fire_Mode;
	if(visionMode != 2 && last_visionMode == 2) {fire.Fire_Mode = last_fireMode; last_fireMode = fire.Fire_Mode;} //���뷴����ģʽʱ�ָ�ԭ��Ƶ
	if(visionMode == 3) {fire.Fire_Mode = 0;}  //����ģʽĬ��1��Ƶ
	if(visionMode != 3 && last_visionMode == 3) {fire.Fire_Mode = 1;}  //�˳�����ģʽ�Զ��л�10��Ƶ
//	Firefrequency = fire.Fire_Mode;
//	if(visionMode != 2 && last_visionMode == 2) {fire.Fire_Mode = last_fireMode; last_fireMode = fire.Fire_Mode;} //���뷴����ģʽʱ�ָ�ԭ��Ƶ
	
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
  //����Զ�����(��Ҫ�󲦸�����)
	if(!vision_RecvStruct.no_Obj && !visionInfo.offlineFlag && vision_RecvStruct.beat && visionMode == 3 && RC.Key.SW1 == RCS::Mid)
	{
		AutoFire_buff = 1;
	}
	else AutoFire_buff = 0;
	last_visionMode = visionMode;
/*********************ϵͳ���***************************/		
	topCtrlRst 	= RC.Key.CTRL*RC.Key.SHIFT*RC.Key.C*deforceFlag;	//����������
	botCtrlRst 	= RC.Key.SHIFT*RC.Key.C*deforceFlag;			//����������
	
		if(myRC.topCtrlRst)
	{
		__disable_irq(); //�ر������ж�
		NVIC_SystemReset(); //��λ
		while (1)
		{
		} //���ȴ���λ
	}
	
	
}





