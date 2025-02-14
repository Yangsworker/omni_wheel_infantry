#include "fire_task.h"
#include "dbus.h"
#include "pid.h"
#include "Gimbal_task.h"
#include "com_task.h"
#include "myRC_task.h"

#define DIAL_ONE_POSITION 1310  //����һ���ӵ������ƶ�λ��360 * 36 / 8   1620  360*36*53/26/10 2641  //1294
#define DIAL_HALF_POSITION 650
//����һ���ӵ�����ʱ��
#define DIAL_ONE_TIME_FAST 20   //25/s         
#define DIAL_ONE_TIME_MID 25    //20/s  //25
#define DIAL_ONE_TIME_SLOW 50  //10/s
#define DIAL_ONE_TIME_GYRO 70
#define DIAL_ONE_TIME_SINGLE 500//1/s

#define current_limit 7000      //����ֵ����

float fire_speed_fast =7400;  //�ߵ���  //7400 


int half_fire_time = 0;

    PidParam firepid,  
     DialInner, DialOuter;
	
 Fire fire(0x201, 0x202, 0x205); 
 
 float S_Dial = 0, R_Dial = 0, D_current = 0;
 float J_fireSpd1 = 0, J_fireSpd2 = 0;
 
 void Fire_Task(void)
{
	fire.Dialpos_record = fire.Dial->canInfo.totalAngle_f;
	if(deforceFlag)   //������¼����λ��
	{
    fire.Dialpos_change = 1;
	}
	fire.ModeJudge_Fire();
}

//�����ٶ�ѡ��
int heat_now = 0;        //���в���ϵͳ/��������ѡ��
u8 Heat_limit_mode = 0;  //�������߽�����������ģʽ
int rotate_fire_clock = 0, buff_fire_clock = 0;
void Fire::fireSpeedSwitch()
{ 

	switch(Fire_Mode)  //Ĭ��20��Ƶ
	{
		case 1:
			DialFireTime = DIAL_ONE_TIME_SLOW;  //10��Ƶ
			break;
		case 2:
			DialFireTime = DIAL_ONE_TIME_FAST;  //20��Ƶ
			break;
		case 3:
			DialFireTime = DIAL_ONE_TIME_GYRO; //���ݵ͵�Ƶ
		default:
			DialFireTime = DIAL_ONE_TIME_SLOW; 
			break;
	}
	if(fire.unLimitedFired)  //������������Կ����л�����ģʽ
	{
		return;
	}

  LastDialFireTime = DialFireTime;
	float Fire_num = 0;
 
	Fire_num = fire.jgmtCoolDown / 10;
		if(Fire_num >= 5)
		{Fire_num = 5;}
/********************************ʹ�ò���ϵͳ�ж�����********************************/   //��¼ʣ������
	if(DialFireTime == DIAL_ONE_TIME_MID)  //20��Ƶ    
	{
		if (jgmtHeat <=  68) //����ϵͳʣ������
	  {
		  Heat_limit_mode = 1;    //��������
			buff_fire_clock = 1;    //�����������
	  }
	  if(Heat_limit_mode)
	  {
      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//��һֱ��������ߵ���
		  if(jgmtHeat >=  80)  
	     {DialFireTime = LastDialFireTime; 
	      Heat_limit_mode = 0;}
	  }
	}
	else
	{
		if (jgmtHeat <=  35) //����ϵͳʣ������
	  {
		  Heat_limit_mode = 1;    //��������
			buff_fire_clock = 1;    //�����������
	  }
	  if(Heat_limit_mode)
	  {
      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//��һֱ��������ߵ���
		  if(jgmtHeat >=  50)  
	     {DialFireTime = LastDialFireTime; 
	      Heat_limit_mode = 0;}
	  }
  }
/********************************ʹ�ò���λ���ж�����********************************/	//��¼��ǰ����
//	heat_now = sprocketHeat;
//if(DialFireTime == DIAL_ONE_TIME_MID)  //20��Ƶ
//	{
//		if (heat_now >= jgmtHeatMax - 50) //����ϵͳ���������Զ�����
//	  {
//		  Heat_limit_mode = 1;    //��������
//			buff_fire_clock = 1;    //�����������
//	  }
//	  if(Heat_limit_mode)
//	  {
//      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//��һֱ��������ߵ���
//		  if(heat_now < (jgmtHeatMax - 60))  
//	     {DialFireTime = LastDialFireTime; 
//	      Heat_limit_mode = 0;}
//	  }
//	}
//	else
//	{
//		if (heat_now >= jgmtHeatMax - 40) //����/10��Ƶ
//	  {
//		  Heat_limit_mode = 1;    //��������
//			buff_fire_clock = 1;
//	  }
//	  if(Heat_limit_mode)
//	  {
//      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//��һֱ��������ߵ���
//		  if(heat_now < (jgmtHeatMax - 50))  
//	     {DialFireTime = LastDialFireTime; 
//	      Heat_limit_mode = 0;}
//	  }
//  }
 

}

//Ħ���ֿ���(��ȡ�������Ʋ�����)  
float Dial_speed_set = 0, Dial_current_set = 0;
bool allow_offset = 0;
void Fire::friction_ctrl()     
{
	allow_offset = 1;
	if(RC.Key.SW2 == RCS::Up)
	{
     friction_spd1 = fire_speed_fast; friction_spd2 = fire_speed_fast;	
//��ú����Ե���Ӱ��ϴ��Ӱ���Ӿ���Ӧ�����Ż�
//		 frictionSpdLimit();  //�ٶ�����
		 allowshoot = 1;  //�������
		allow_offset = 0;
	}
	if(RC.Key.SW2 == RCS::Mid || Firemotor[0]->canInfo.lostFlag || Firemotor[1]->canInfo.lostFlag)
		if(RC.Key.SW2 == RCS::Mid)
	{
		friction_spd1 = 0; friction_spd2 = 0; 
		allowshoot = 0;  //��������� 
		allowshoot_vision = 0;
	}
   
		Firemotor[0]->ctrlSpeed(friction_spd1);
	  Firemotor[1]->ctrlSpeed(-friction_spd2);
}
//���̷������ǰ��Ħ����ת��
int singlefireback_time = 0;//������תʱ��
bool allow_back = 1; //�Ƿ��������
int last_fire_num = 0, jgmt_fire_num = 0;//�����Զ�У׼���������
u8 isDoubleshot = 0; //�Ƿ������
int fire_num_real = 0;
void Fire::fireload()          
{
	S_Dial = Dialpos_set / 100.0f;
	R_Dial = Dialpos_record / 100.0f;
	S_Dial*=1;
	R_Dial*=1;
	D_current = Dial->canInfo.trueCurrent;
	J_fireSpd1 = Firemotor[0]->canInfo.speed;
	J_fireSpd2 = Firemotor[1]->canInfo.speed;
	
	jgmtHeatCalc();     //����ϵͳ������ȡ
	sprocketHeatCalc(); //����������ȡ
	fireSpeedSwitch();  //����ת��ѡ��
	current_now = ABS(Dial->canInfo.trueCurrent);       //��¼���̵���ֵ
	Dialpos_record = Dial->canInfo.totalAngle_f;        //��¼����λ��

	if(Dialpos_change) {Dialpos_set = Dialpos_record; Dialpos_initial = Dialpos_record; Dialpos_change = 0;} //�������㲦�̳�ʼλ��



	 //�����Զ����𣨷���Ƶ�ʿɵ���
//	 if(myRC.visionMode == 2)
//	 {
//		 if(vision_RecvStruct.beat && allowshoot_vision && !rotate_fire_clock ) 
//		 {
//			 Dialpos_set += DIAL_ONE_POSITION ; 
//			 	allowshoot = 0; 
//		 }
//		 rotate_fire_clock++;
//		 if(rotate_fire_clock >= DialFireTime) {rotate_fire_clock = 0;}
//	 }
//	}
//�ֶ�����ģʽ
if((RC.Key.CH[8] || myRC.AutoFire) && allowshoot && DialFireTime && Fire_Mode)  //DialFireTime-��ֹ����ʱ��������
	{
		if(!(FireTimes % DialFireTime))           
		{
			Dialpos_set += DIAL_ONE_POSITION ;
		  FireTimes = 0 ;
		}
		last_key = 1 ;
	}
 FireTimes++ ; if(FireTimes > 1000) {FireTimes = 1;}    //���ƴ�С�����    
//�ֶ�����ģʽ 
 if( ((RC.Key.CH[8] && !Fire_Mode)|| RC.Key.SW1 == RCS::Down || myRC.AutoFire_buff )&& allowshoot && !Heat_limit_mode) 
	{
		if(last_key == 2)
		{
		  if(isDoubleshot >= 1) {Dialpos_set += DIAL_ONE_POSITION / 4 * 5; isDoubleshot = 0;}
			else            Dialpos_set += DIAL_ONE_POSITION ;
//			Dialpos_set = Dialpos_record + 1500;
//			Dialpos_set += DIAL_ONE_POSITION;
			last_fire_num = fire_num_real;
		}
		last_key = 3 ;
	}
	if(RC.Key.CH[8] == 0 && RC.Key.SW1 != RCS::Down && !myRC.AutoFire_buff && !myRC.AutoFire)                         //���������ʼ��ֵ
	{
		last_key = 2 ;
	}
//��ת���(������/�������λ��)
	 //������ת���
		if(current_now > 10000)
		{
			singlefireback_time++;
		}
		else
		{
			singlefireback_time = 0;
		}
		if(singlefireback_time > 100 && allow_back)
		{
			allowshoot = 0;
			Dialpos_set = Dialpos_record - DIAL_ONE_POSITION / 2.0;
			allow_back = 0;
		}
  //λ�ö�ת���	
	  if ( (ABS(Dialpos_set - Dialpos_record) > DIAL_ONE_POSITION * 3) )    //������ת���/����Ҳ���Դ���           
			{         
        allowshoot = 0;  //��ת������������
  			Dialpos_set = Dialpos_record - DIAL_ONE_POSITION / 2.0;
				allow_back = 0; 
			}
    if(!allow_back)
	  {if(current_now < current_limit - 5000) {allowshoot = 1; allow_back = 1;}} 
//���������
//	if(!Fire_Mode)
//	{
		jgmt_fire_num = fire_num_real - last_fire_num;
		if(jgmt_fire_num >1)
		{
			isDoubleshot++;
			last_fire_num = fire_num_real;
		}
//	}
//���̣����أ�������
	thisShootNum.sprocketShootNum = (Dialpos_record-Dialpos_initial)/DIAL_ONE_POSITION;
//���̿���	
//		if(allow_offset)
//		{
//			Dial->ctrlCurrent(1000) ;  
//		}
//		else
			Dial->ctrlPosition(Dialpos_set);
//		if(Dialpos_record < Dialpos_set)
//		{
//	    Dial_speed_set = Dial->pidOuter.Calculate(Dialpos_set);
////			Dial_current_set = Dial->pidInner.Calculate(Dial_speed_set);
//		}
//		else
//		{
//			Dial_speed_set = 0;
////			Dial_current_set = Dial->pidInner.Calculate(Dial_speed_set);
//		}		
////		Dial->ctrlCurrent(Dial_current_set);
//		Dial->ctrlSpeed(Dial_speed_set);
	  
}
 //���������� ���������/���� ϵͳ��ȴ ����ϵͳ���٣�
void Fire::getFireLimit()
{
	if (!!JgmtPack.jgmtOfflineFlag)//����ϵͳδ����
	{
		fire.jgmtCoolDown = LocalPack.shooterCoolingRateA;                        //����ϵͳ��ȴ
		fire.jgmtBulletSpeedMax = 30; //����ϵͳ�����
		fire.jgmtHeatMax = LocalPack.shooterCoolingLimitA;                      //����ϵͳ�������
	}
	else //������ϵͳ���ߣ�������һ������
	{
		fire.jgmtCoolDown = 30;
		fire.jgmtBulletSpeedMax = 30;
		fire.jgmtHeatMax = 200;
	}
}
 //����ϵͳ��������
void Fire::jgmtHeatCalc()
{
	if (!JgmtPack.jgmtOfflineFlag)
		jgmtHeat = JgmtPack.shooterHeatRemainA;//����ϵͳʣ������
	else
		jgmtHeat = jgmtHeatMax;//ֱ���趨�����ﵽ����
}
 //���֣����أ���������
void Fire::sprocketHeatCalc()
{
	if (thisShootNum.sprocketShootNum != lastShootNum.sprocketShootNum)   //������ּ�⵽���䣬����������10
	{
		sprocketHeat += 10 * (thisShootNum.sprocketShootNum - lastShootNum.sprocketShootNum);
	}
	lastShootNum.sprocketShootNum = thisShootNum.sprocketShootNum;

	sprocketHeat -= fire.jgmtCoolDown * sprocketHeatTimer.getCycleT();  //������ʱ���Լ�
	sprocketHeat = LIMIT(sprocketHeat, 0, jgmtHeatMax + 20);
}
 //Ħ�����ٶ�����/ͬʱ��¼������ 
void Fire::frictionSpdLimit()
{
	jgmtBulletSpeed = JgmtPack.bulletSpeed / 100.0f;
	if (jgmtBulletSpeed == lastJgmtBulletSpeed) 
		return;
	else
		fire_num_real++;
	if (lastJgmtBulletSpeed == 0)//��֡����
	{
		lastJgmtBulletSpeed = jgmtBulletSpeed;
		return;
	}
	if(jgmtBulletSpeed < 10)//��Ϊ0mps
	return;
  if(!JgmtPack.jgmtOfflineFlag)	//���ٵ������ڲ���ϵͳδ���������
	{
	 if(jgmtBulletSpeed > (jgmtBulletSpeedMax - 1.0))//�ٶȳ����Զ�����
	 {
		friction_spd1 -= 30; 
		 friction_spd2 -= 30; 
		 fire_speed_fast -= 30;
	 }
	 else if (jgmtBulletSpeed < jgmtBulletSpeedMax - 5 && jgmtBulletSpeedMax<32 ) //�ٶ�С����ߵ�����ߵ���
	 { 
			friction_spd1 += 10; 
		 friction_spd2 += 10; 
		  fire_speed_fast += 10;
	 }
  }
	 lastJgmtBulletSpeed = jgmtBulletSpeed;
}

void Fire::ModeJudge_Fire(void)
{
	getFireLimit();   //�����������
	friction_ctrl();  //Ħ���ֿ���
	fireload();       //���̿���
}  

/*
 * ���캯��
*/
Fire::Fire(u16 fire1ID, u16 fire2ID, u16 DialID)  //��pid �����ʼ����
{
	Firemotor[0] = new Motor (M3508, CAN2, fire1ID);
  Firemotor[1] = new Motor (M3508, CAN2, fire2ID);
	Dial    = new Motor (M3508, CAN1, DialID);
	//pid����
	firepid.kp = 13;
	firepid.ki = 100;
	firepid.kd = 0.05;
	firepid.integralErrorMax = 0.1;
	firepid.resultMax = 16380;
	
	DialInner.kp = 10;  
	DialInner.ki = 100;
	DialInner.kd = 0;
	DialInner.integralErrorMax = 10;
	DialInner.resultMax = 15000; //8000
	
	DialOuter.kp = 15; //8,5,0.01,1 
	DialOuter.ki = 100;
	DialOuter.kd = 0;
	DialOuter.integralErrorMax = 10;//0.2
	DialOuter.resultMax = 5000;
	
	for(u8 i = 0; i < 2; i++)
	{
	  Firemotor[i]->pidInner.setPlanNum(1);
	  Firemotor[i]->pidInner.fbValuePtr[0] = &Firemotor[i]->canInfo.speed;
	  Firemotor[i]->pidInner.paramPtr = &firepid;
	}
	//pid�������趨
	Dial   ->pidInner.setPlanNum(1);
	Dial   ->pidOuter.setPlanNum(1);
	//pid������Դ
	Dial  -> pidInner.paramPtr = &DialInner;
	Dial  -> pidOuter.paramPtr = &DialOuter;
	//pid����ֵ��Դ
	Dial   ->pidInner.fbValuePtr[0] = &Dial ->canInfo.speed;
	Dial   ->pidOuter.fbValuePtr[0] = &Dial ->canInfo.totalAngle_f;
}
