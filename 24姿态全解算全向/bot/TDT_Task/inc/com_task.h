#ifndef _COM_ 
#define _COM_

#include "board.h"
#include "can.h"

enum _myPack{
	Jgmt,Local,Ctrl,Display,Offset
};

/*************************����->��̨�����ݰ�*************************/

#pragma pack(1)
//��Ƶ��500HZ
typedef union 
{
	uint8_t data[8];
	struct{
		uint8_t jgmtOfflineFlag:1;		//����ϵͳ���߱�־
		uint16_t shooterHeatRemainA:10;	//��ǰAǹʣ������600
		uint16_t underWaringFlag:1;		//���з���־λ����Ұ�ڵ�����ʱ�Ӿ�ӵ�о���Ȩ�ޡ�
		uint16_t bulletSpeed:12;		//��ǰ����[m/s]*100(float in judgement)		
		uint8_t bulletCount:6;			//�����������ṩ���䣬����ֵ���岻��
		uint16_t powerlimitKp:10; //��������ϵ��
		int16_t yaw_dps;   //�����ٶ�
	};
	//remain_b:24
}JgmtAbout;//0x400
#pragma pack()

#pragma pack(1)
//����Ƶ��10HZ
typedef union 
{
	uint8_t data[8];
	struct{
		uint8_t myColor:1;				//����ϵͳ�����ĵ�ǰ�Լ�����ɫ 0:�� 1:��
		
		uint16_t shooterSpeedLimitA:2; //��������[m/s] 0:15m/s | 1:18m/s | 2:30m/s
		uint16_t shooterCoolingLimitA:10; //Aǹ��������600
		uint16_t shooterCoolingRateA:10; //Aǹÿ����ȴֵ80*5*1.5
		
		uint16_t shooterSpeedLimitB:2; //��������[m/s] 0:15m/s | 1:18m/s | 2:30m/s
		uint16_t shooterCoolingLimitB:10; //Aǹ��������600
		uint16_t shooterCoolingRateB:10; //Aǹÿ����ȴֵ80*5*1.5
	};
	//remain_b:19
}LocalAbout;//0x401
#pragma pack()

/*************************��̨->���̵����ݰ�*************************/

#pragma pack(1)
//��Ƶ��500HZ
typedef union
{
	uint8_t data[8];
	struct{
    
		uint8_t if_Shift:1;		//�Ƿ���shift
		uint8_t if_Ctrl:1;	  //�Ƿ���ctrl
		uint8_t SpeedUp_level:1; //���ٵȼ�		
		uint8_t sysRST:1;		//������������
		uint8_t UI_Init:1;
		
	};
	//remain_b:4
}CtrlAbout;//0x500
#pragma pack()

#pragma pack(1)
//��Ƶ��50Hz
typedef union
{
	uint8_t data[8];
	struct{	
		int8_t visYawError:7;		//�Ӿ�Y��ƫ��Ƕ�
		uint16_t friLSpd:13;		//��Ħ����ת��
		uint16_t friRSpd:13;		//��Ħ����ת��
		uint16_t pitchCode:13;		//Pitch��������
		
		uint8_t visionMode:3;		//�Ӿ�ģʽ 0:δ����|1:����|2:����|3:����|4:С��|5:���
		uint8_t visState:2;			//�Ӿ�״̬:0-�Ӿ�����|1-�Ӿ���Ŀ��|2-�Ӿ���Ŀ��|3-�Ӿ��ɿ���
		
		uint8_t ready2Fire:1;		//�������
		uint8_t feedPos:3;			//����λ��
		uint8_t Firefrequency:2;	//0-����|1-10|2-20
		
		uint8_t enemyID:4;			//��׼Ŀ���ID
		uint8_t CheckMode:1;		//��¼ģʽ
		uint8_t longFocusSwitch:1;	//�������
	};
	//remain_b:1
}DisplayAbout;//0x501
#pragma pack()


/**********************************************************/
	/*TX*/
void Com_Task(_myPack myPack);		//��Ϣ�ӹ�ͨCAN����
	/*RX*/
void CanInfo_Handle(CanRxMsg ThCanMsg);	//��ͨCAN��Ϣ����

/****����->��̨�����ϰ�****/
extern JgmtAbout JgmtPack;
extern LocalAbout LocalPack;
/****��̨->���̵����ϰ�****/
extern CtrlAbout CtrlPack;
extern DisplayAbout DisplayPack;

extern CAN_TypeDef *ThCan; //��ͨCanͨ��

#endif