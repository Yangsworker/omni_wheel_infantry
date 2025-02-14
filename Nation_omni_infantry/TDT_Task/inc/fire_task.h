#ifndef _FIRE_
#define _FIRE_

#include "board.h"
#include "motor.h"
#include "can.h"
#include "chassis_task.h"
#include "ptz_task.h"
#include "vision.h"

enum FireID_t     //2��Ħ����M3508 ���� ����
{
	fire1, fire2, DIAL
};
enum //��δ�õ���
	{
		SILENCE,	//��ֹ����(��Ĭ)
		CLEAN_AMMO, //�嵯 
		FAST_SHOOT, //��Ƶ
		MID_SHOOT,	//����
		SLOW_SHOOT, //��Ƶ
		BUFF_SHOOT	//���
	} fireMode;							//����״̬ö��



class Fire
{
  public:
		Fire(u16 fire1ID, u16 fire2ID, u16 DialID);
	
	Motor *Firemotor[2]; //2*3508Ħ����
	Motor *Dial;    //����
	u8 Robot_Fire_Mode = 0;//�����˷������ѡ��ģʽ 0-��ʼ���� 1-���� 2-��ȴ 3-����
	bool isjudgeover = 0;
	//Ħ���ֳ���
  double friction_spd1 , friction_spd2; //Ħ�����趨�ٶ�
	u8 allowshoot = 0;        //�ֶ���������
	u8 allowshoot_vision = 0; //�Ӿ��������� 
	void friction_ctrl();//Ħ���ֿ���
	//���̳���	
	float Dialpos_set;      //����λ���趨ֵ
	float Dialpos_record;   //��������λ��
	float Dialpos_initial;  //���̳�ʼλ��
	float Dialencouder_record; //�������ڻ�е�Ƕ�ֵ��¼
	bool  Dialpos_change;      

	uint8_t Fire_Mode = 0;       //����ģʽ��0-����/1-10����/2-20������ ͨ��F�����Ƹı䣬Ĭ�ϵ���
	bool unLimitedFired = 0;     //�����������
	
	uint8_t last_key;    //�ϵ������̣�ģʽ�л�
	uint8_t last_key_vision; //�ϵ����Ӿ���
	uint8_t last_fire;   //��¼�ϴη���
	int DialFireTime;    //����ת���ٶ�
	int LastDialFireTime;
	int FireTimes;       //����ʱ��������������ת��Ƶ�ʣ�
	int CheckTimes;      //��ת���ʱ��
	float current_now;   //ʵʱ����ֵ
	void fireload(void);     //����װ��(��ת���)
	
//����Ϊ���١��������� ���󲿷ֺ���Ҳû�õ���
	struct
	{
		uint16_t sprocketShootNum; //����(����)�ж��ķ�������
		uint16_t frictionShootNum; //Ħ�����ж��ķ�������
		uint16_t jgmtShootNum;	   //����ϵͳ�ж��ķ�������
	}thisShootNum,lastShootNum;
	Cycle sprocketHeatTimer;
	void sprocketHeatCalc();	//��������������
	Cycle frictionHeatTimer;
	void frictionHeatCalc();	//Ħ������������
	void jgmtHeatCalc();		//����ϵͳ��������
	void heatCalc();			//��������
	float sprocketHeat;		//�������ж��ı�������
	float frictionHeat;		//Ħ�����ж��ı�������
	float jgmtHeat;			//����ϵͳ����
	float finalHeat;		//���߽�ϵ���������
	float finalHeatAdditionSet[3];
	float finalHeatAddition;
	u8 heatChoose;			//����ѡ��
	int16_t jgmtHeatMax;		//����ϵͳ�������ֵ
	uint16_t jgmtCoolDown;		//����ϵͳ��ȴ
	u8 getSpeedIndex();
	void frictionSpdSwitch();	 //�����ٶ��л�
	void frictionSpdLimit();	 //�����ٶ�����
	float jgmtBulletSpeed;		 //��ǰ�ӵ��ٶ�
	uint16_t jgmtBulletSpeedMax; //����ϵͳ��������
	uint16_t last_SpeedMax;      //��¼����ϵͳֵ�ж����Ƿ�仯
	float lastJgmtBulletSpeed;
	
	float notFireTimer;
	Cycle notFireTimerCycle;
	//����
	void getFireLimit();        //��ÿ�������/��÷���ģʽ
	void fireSpeedSwitch();     //�����ٶ�ѡ��
  void ModeJudge_Fire(void);  //ģʽѡ��������
};

void Fire_Task(void);

extern Fire fire;
#endif