#ifndef _POWER_
#define _POWER_

#include "board.h"
#include "can.h"

class Power
{
public:
	float powerLimitKp;					//��������ϵ��
	float underVoltageProtectKp = 1;	//Ƿѹ����ϵ��
	float overFlowKp = 1;				//��������ϵ��

	union{	//0X110:����ϵͳ�����������
		u8 txData_110[8];
		struct {
			float chassisPower;				//���̹���
			uint16_t chassisPowerBuffer;	//���̻�������
			uint16_t max_chassis_power;		//�����		
		};
	};
	union{	//0X120:����״̬����
		u8 txData_120[8];
		struct {
			uint8_t useCap:1;			//ʹ�õ���[·���л�]
			uint8_t Check_Mode:1;		//��¼ģʽ
			uint8_t ULTS_Mode:1;		//��������ģʽ	
		};	
	};
	union{	//0X130:����ģ�鷴������
		u8 rxData_130[8];
		struct {
			uint16_t capacitance_percentage_t;	//���������ٷֱ�[��ͨѶ�ã�ģ�ⶨ����]
			uint16_t maxCurrent_t;				//������[��ͨѶ�ã�ģ�ⶨ����]
			uint16_t currentNow_t;				//��ǰ����[��ͨѶ�ã�ģ�ⶨ����]
			uint16_t capHealth:1;				//��ǰ���ݵ�ѹ���ͣ�����ֱͨ״̬
			uint16_t errorCode:3;				//���ʴ���
			uint16_t reserved:12;
		};	
	};
	float capacitance_percentage;	//���ݰٷֱ�
	float currentNow;				//��ǰ����
	float maxCurrent;				//������

	uint8_t powerDownLimit;			//���ʿ�������
	uint8_t powerLimitRange;		//���ʿ��Ʒ�Χ
	
	uint8_t speedUpLevel;	//���ٵȼ� 0:��̬|1:��ͨ����|2:��������|3:���¼���
	
	bool usingBackup;				//"����"ʹ�ñ��ù���[�����������Զ��ر�]
	bool useTooMuchCapFlag;			//����ʹ�ó��ޱ�־λ
	bool flySloping;				//���ڷ���

	enum {
		offline,onCAN1,onCAN2
	}whereIsPower;//����ģ�����������
	void myPowerCanRx(CanRxMsg CanRxMsg);
	void myPowerCanTx(void);			//����CAN��Ϣ����
	
	void myPowerOfflineJudge(void);		//����ģ�����߼��
	void getPowerKp(void);	//���ݼ��ٵȼ��õ���������ϵ��
	
private:
	u8 powerOfflineCnt;
	void getPowerLimit(void);			//���ݼ��ٵȼ�ѡ��������ֵ
	void powerOverFlowCal(void);		//�������ʼ���
	void underVoltageProtection(void);	//Ƿѹ����ϵ������
	void setPowerLimit(float downLimit,float upLimit = 0,uint8_t range = 10);
};

extern Power power;

#endif
