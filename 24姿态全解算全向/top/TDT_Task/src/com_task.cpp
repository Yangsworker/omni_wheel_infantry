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

CAN_TypeDef *ThCan = CAN1;//�趨��ͨCAN


/****����->��̨�����ϰ�****/
JgmtAbout JgmtPack;
LocalAbout LocalPack;
/****��̨->���̵����ϰ�****/
CtrlAbout CtrlPack;
DisplayAbout DisplayPack;

/*******************�Զ��崦����*******************/

extern MultiImu *boardImu;

//��Ϣ�ӹ�ͨCAN����
void Com_Task(_myPack myPack)
{
	static int32_t lastCode; 
	if(myPack == Ctrl)
	{
		CtrlPack.if_Shift = myRC.if_Shift;		//�Ƿ���shift
		CtrlPack.if_Ctrl = myRC.if_Ctrl;		  //�Ƿ���ctrl
		CtrlPack.SpeedUp_level = myRC.SpeedUp_level;  //���ٵȼ�
		
		CtrlPack.sysRST = myRC.botCtrlRst;		//������������

		/*���Ϸ�������ϢԤ����*/
		canTx(CtrlPack.data,CAN1,0x500);
	}
/***********************************************************************/	
	else if(myPack == Display)
	{
		DisplayPack.visYawError = (uint16_t)(vision_RecvStruct.Yaw-gimbal.WorldAlg[YAW]);//�Ӿ�Y��ƫ��Ƕ�
		DisplayPack.friLSpd = (uint16_t)(ABS(fire.Firemotor[0]->canInfo.speed)); //��Ħ����ת��
		DisplayPack.friRSpd = (uint16_t)(ABS(fire.Firemotor[1]->canInfo.speed)); //��Ħ����ת��
		DisplayPack.pitchCode = gimbal.Pitch->canInfo.encoder; //Pitch��������
		
		DisplayPack.visionMode = myRC.visionMode;		//�Ӿ�ģʽ 0:δ����|1:����|2:����|3:����|4:С��|5:���
		if(DisplayPack.visionMode == 3) DisplayPack.visionMode += (1+vision_SendStruct.bufftype);   //���뿪��ģʽʱ�жϴ�С��
//		//�Ӿ�״̬:0-�Ӿ�����|1-�Ӿ���Ŀ��|2-�Ӿ���Ŀ��|3-�Ӿ��ɿ���
//		/*| VisState|  beat |no_Obj	|offlineFlag|*/
//		/*|	0		|	0	|	1	|	1		|*/
//		/*|	1		|	0	|	1	|	0		|*/
//		/*|	2		|	0	|	0	|	0		|*/
//		/*|	3		|	1	|	0	|	0		|*/
		DisplayPack.visState  =  (ABS(DisplayPack.visYawError) < 0.3 && !vision_RecvStruct.no_Obj);
		DisplayPack.visState += !vision_RecvStruct.no_Obj;//�Ӿ���Ŀ�꣬��־λ����
	  ++DisplayPack.visState *= !visionInfo.offlineFlag;//�Ӿ����ߣ���־λ����

		DisplayPack.Firefrequency = myRC.Firefrequency; //0-����|1-10��Ƶ|2-20��Ƶ
		
		DisplayPack.enemyID = vision_RecvStruct.objID; //��׼Ŀ���ID
		DisplayPack.CheckMode = myRC.CheckMode;		//��¼ģʽ
		
		/*���Ϸ�������ϢԤ����*/
		canTx(DisplayPack.data,ThCan,0x501);
	}
}

//��ͨCAN��Ϣ����
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


