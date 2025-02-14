#include "fire_task.h"
#include "dbus.h"
#include "pid.h"
#include "ptz_task.h"

#define FIRE_SPEED_SLOW 4300  //15mps
#define FIRE_SPEED_MID  4650  //18mps
#define FIRE_SPEED_FAST 7450  //30mps
#define DIAL_ONE_POSITION 1620    //����һ���ӵ������ƶ�λ��360 * 36 / 8   1620
#define DIAL_HALF_POSITION 810
//����һ���ӵ�����ʱ��
#define DIAL_ONE_TIME_FAST 20   //25/s         
#define DIAL_ONE_TIME_MID 25    //20/s  //25
#define DIAL_ONE_TIME_SLOW 50  //10/s
#define DIAL_ONE_TIME_SINGLE 500//1/s

#define current_limit 9600      //����ֵ����
#define Dial_offest_k 2.59

int half_fire_time = 0;

    PidParam firepid,  
     DialInner, DialOuter;
	
 Fire fire(0x201, 0x202, 0x203); 
 
 float J_Dial = 0, R_Dial = 0, D_current = 0;
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
Fire::Fire(u16 fire1ID, u16 fire2ID, u16 DialID)  //��pid �����ʼ����
{
	Firemotor[0] = new Motor (M3508, CAN2, fire1ID);
  Firemotor[1] = new Motor (M3508, CAN2, fire2ID);
	Dial    = new Motor (M2006, CAN2, DialID);
	//pid����
	firepid.kp = 15;
	firepid.ki = 100;
	firepid.kd = 0;
	firepid.integralErrorMax = 2;
	firepid.resultMax = 16380;
	
	DialInner.kp = 14;   //18,100,0,0.1
	DialInner.ki = 0;
	DialInner.kd = 0;
	DialInner.integralErrorMax = 2;
	DialInner.resultMax = 9200; //8000
	
	DialOuter.kp = 8; //8,5,0.01,1 
	DialOuter.ki = 100;
	DialOuter.kd = 0.02;
	DialOuter.integralErrorMax = 1;//0.2
	DialOuter.resultMax = Dial -> getMotorSpeedLimit();
	
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
//�����ٶ�ѡ��
int heat_now = 0;        //���в���ϵͳ/��������ѡ��
u8 Heat_limit_mode = 0;  //�������߽�����������ģʽ
int rotate_fire_clock = 0, buff_fire_clock = 0;
void Fire::fireSpeedSwitch()
{ 
	if (can1Feedback.MaxHeat == -1 )  //�����������Ϊ-1�����߽ӳ��������� -> ��Ƶ
	{
		DialFireTime = DIAL_ONE_TIME_MID;
		return;
	}

	switch(Fire_Mode)
	{
		case 0:
			DialFireTime = DIAL_ONE_TIME_SLOW;  //Ĭ��10��Ƶ
		  break;
		case 1:
			DialFireTime = DIAL_ONE_TIME_SLOW;  
			break;
		case 2:
			DialFireTime = DIAL_ONE_TIME_MID;  //20��Ƶ
			break;
		default:
			DialFireTime = DIAL_ONE_TIME_SLOW;  //Ĭ��10��Ƶ
			break;
	}
	if(fire.unLimitedFired)  //������������Կ����л�����ģʽ
	{
		return;
	}

//	if(!can1Feedback.jgmtOffline)
//		heat_now = jgmtHeat;
//	else
		heat_now = sprocketHeat;
	
  LastDialFireTime = DialFireTime;
	float Fire_num = 0;
	if(Robot_Fire_Mode == 1)//--------------------------------------��������------------------------------------------------------------
	{
	Fire_num = fire.jgmtCoolDown / 10;
	if(DialFireTime == DIAL_ONE_TIME_MID)  //20��Ƶ
	{
		if (heat_now >= jgmtHeatMax - 60) //����ϵͳ���������Զ�����
	  {
		  Heat_limit_mode = 1;    //��������
			buff_fire_clock = 1;    //�����������
	  }
	  if(Heat_limit_mode)
	  {
      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//��һֱ��������ߵ���
		  if(heat_now < (jgmtHeatMax - 71))  
	     {DialFireTime = LastDialFireTime; 
	      Heat_limit_mode = 0;}
	  }
	}
	else
	{
		if (heat_now >= jgmtHeatMax - 37) //����/10��Ƶ
	  {
		  Heat_limit_mode = 1;    //��������
			buff_fire_clock = 1;
	  }
	  if(Heat_limit_mode)
	  {
      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//��һֱ��������ߵ���
		  if(heat_now < (jgmtHeatMax - 47))  
	     {DialFireTime = LastDialFireTime; 
	      Heat_limit_mode = 0;}
	  }
  }
  }
	if(Robot_Fire_Mode == 2)//-------------------------------------��ȴ����-------------------------------------------
	{
	Fire_num = fire.jgmtCoolDown / 10;
	if(DialFireTime == DIAL_ONE_TIME_MID)  //20��Ƶ
	{
		if (heat_now >= jgmtHeatMax - 40) //����ϵͳ���������Զ�����
	  {
		  Heat_limit_mode = 1;    //��������
			buff_fire_clock = 1;    //�����������
	  }
	  if(Heat_limit_mode)
	  {
      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//��һֱ��������ߵ���
		  if(heat_now < (jgmtHeatMax - 50))  
	     {DialFireTime = LastDialFireTime; 
	      Heat_limit_mode = 0;}
	  }
	}
	else
	{
		if (heat_now >= jgmtHeatMax - 30) //����/10��Ƶ
	  {
		  Heat_limit_mode = 1;    //��������
			buff_fire_clock = 1;
	  }
	  if(Heat_limit_mode)
	  {
      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//��һֱ��������ߵ���
		  if(heat_now < (jgmtHeatMax - 40))  
	     {DialFireTime = LastDialFireTime; 
	      Heat_limit_mode = 0;}
	  }
  }
  }
  if(Robot_Fire_Mode == 3 || !Robot_Fire_Mode)//---------------------------------------��������---------------------------------------------
	{
	Fire_num = fire.jgmtCoolDown / 10;
		if(Fire_num >= 3)
		{Fire_num = 3;}
	if(DialFireTime == DIAL_ONE_TIME_MID)  //20��Ƶ
	{
		if (heat_now >= jgmtHeatMax - 27) //����ϵͳ���������Զ�����
	  {
		  Heat_limit_mode = 1;    //��������
			buff_fire_clock = 1;    //�����������
	  }
	  if(Heat_limit_mode)
	  {
      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//��һֱ��������ߵ���
		  if(heat_now < (jgmtHeatMax - 37))  
	     {DialFireTime = LastDialFireTime; 
	      Heat_limit_mode = 0;}
	  }
	}
	else
	{
		if (heat_now >= jgmtHeatMax - 23) //����/10��Ƶ
	  {
		  Heat_limit_mode = 1;    //��������
			buff_fire_clock = 1;
	  }
	  if(Heat_limit_mode)
	  {
      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//��һֱ��������ߵ���
		  if(heat_now < (jgmtHeatMax - 33))  
	     {DialFireTime = LastDialFireTime; 
	      Heat_limit_mode = 0;}
	  }
  }
  }	

}

//Ħ���ֿ���(��ȡ�������Ʋ�����)  
int friction_run = 0;
void Fire::friction_ctrl()     
{
	if(RC.Key.SW2 == RCS::Up)
	{
		//�ٶȸı�����Ħ���ֻ���
		if(jgmtBulletSpeedMax != last_SpeedMax)
		{friction_run = 0;}
		if(!friction_run)  //��ȡ����ϵͳ�������ƿ��Ƶ��� �����Ӿ�������Ϣ
	    {if(jgmtBulletSpeedMax == 30) {vision_SendStruct.nominalBulletSpeed = 30; friction_spd1 = FIRE_SPEED_FAST; friction_spd2 = FIRE_SPEED_FAST + 19;}
	     else if(jgmtBulletSpeedMax == 18) {vision_SendStruct.nominalBulletSpeed = 18; friction_spd1 = FIRE_SPEED_MID; friction_spd2 = FIRE_SPEED_MID + 13;}
	     else if(jgmtBulletSpeedMax == 15) {vision_SendStruct.nominalBulletSpeed = 15; friction_spd1 = FIRE_SPEED_SLOW; friction_spd2 = FIRE_SPEED_SLOW + 12;}
			 else                       {friction_spd1 = FIRE_SPEED_SLOW + 15;  friction_spd2 = FIRE_SPEED_SLOW;}
	     friction_run = 1;}
			 last_SpeedMax = jgmtBulletSpeedMax; //��¼�ϴ�Ħ�����ٶ�����
//��ú����Ե���Ӱ��ϴ��Ӱ���Ӿ���Ӧ�����Ż�
		   frictionSpdLimit();  //�ٶ�����
		   allowshoot = 1;  //�������
			 allowshoot_vision = 1;	
	}
	if(RC.Key.SW2 == RCS::Mid)
	{
		friction_spd1 = 0; friction_spd2 = 0; 
		friction_run = 0;
		allowshoot = 0;  //��������� 
		allowshoot_vision = 0;
	}
   
		Firemotor[0]->ctrlSpeed(friction_spd1);
	  Firemotor[1]->ctrlSpeed(-friction_spd2);
	
}
//���̷������ǰ��Ħ����ת��
uint8_t Change_CD;   //����/����ת����ȴ
int singlefireback_time = 0;//������תʱ��
bool allow_back = 1; //�Ƿ��������
int autofire_clock = 0; //�Զ������ʱ
u8 islastfired = 0;   //�ϴη����¼������Զ���
int last_fire_num = 0, jgmt_fire_num = 0;//�����Զ�У׼���������
u8 isDoubleshot = 0; //�Ƿ������
int fire_num_real = 0;
void Fire::fireload()          
{
	 
	J_Dial = Dialpos_set;
	R_Dial = Dialpos_record;
	D_current = Dial->canInfo.trueCurrent;
	J_fireSpd1 = Firemotor[0]->canInfo.speed;
	J_fireSpd2 = Firemotor[1]->canInfo.speed;
	
	jgmtHeatCalc();     //����ϵͳ������ȡ
	sprocketHeatCalc(); //����������ȡ
	fireSpeedSwitch();  //����ת��ѡ��
	current_now = ABS(Dial->canInfo.trueCurrent);       //��¼���̵���ֵ
	Dialpos_record = Dial->canInfo.totalAngle_f;        //��¼����λ��
	Dialencouder_record = Dial->canInfo.encoder;        //��¼����encouderֵ

	if(Dialpos_change) {Dialpos_set = Dialpos_record; Dialpos_initial = Dialpos_record; Dialpos_change = 0;} //�������㲦�̳�ʼλ��

	if(Change_CD ) {Change_CD++;}
	if(Change_CD > 100) {Change_CD = 0;}
	if(!Change_CD && RC.Key.F) 
  {
		Fire_Mode++;
		if(Fire_Mode > 2)
		{Fire_Mode = 0;}
	  Change_CD = 1;
	}  
//	fire.unLimitedFired = RC.Key.C;                     //�����������

//�Ӿ�����ģʽ����������������ֿ��������Զ�����ģʽҲ��������ֿ���
	if(RC.Key.SW1 == RCS::Up && CtrlPlan == Vision)
	{
	//�����Զ����𣨶�ȡһ����������һ��,��һֱ��������5��/s��
	 if(VM == bigBUFF || VM == smallBUFF || VM == ANTIBUFF)
	 {
		 if(VM == ANTIBUFF) {allowshoot = 1;}
		 else         	    {allowshoot = 0;} //��ֹ�����ֿ���
		 if(vision_RecvStruct.beat && allowshoot_vision)
		 {
			  buff_fire_clock++;
				if(last_key_vision == 2 || buff_fire_clock >= 100)
		    {
			     Dialpos_set += DIAL_ONE_POSITION ;
					 buff_fire_clock = 0;
		    }
		    last_key_vision = 3 ; 
		 }
//		 if(vision_RecvStruct.beat && allowshoot_vision && VM == rotate)
//		 {
//			 if(last_key_vision == 2)
//		    {
//			     Dialpos_set += DIAL_ONE_POSITION ;
//		    }
//		    last_key_vision = 3 ; 
//		 }
		 if(!vision_RecvStruct.beat)         
	   {
		    last_key_vision = 2 ;  
				buff_fire_clock = 0;
	   }
		 
	 }
////����Զ��������
//	 if(ptz.autofireFlag && allowshoot_vision)
//	 {
//		 autofire_clock++;
//		 if(islastfired == 2 || autofire_clock > 100)
//		 {
//			 Dialpos_set += DIAL_ONE_POSITION;
//			 autofire_clock = 0;
//		 }
//		 islastfired = 3;
//	 }
//	 if(!ptz.autofireFlag)
//	 {
//		 islastfired = 2;
//		 autofire_clock = 0;
//	 }

	 //�����Զ����𣨷���Ƶ�ʿɵ���
//	 if(VM == rotate)
//	 {
//		 if(vision_RecvStruct.beat && allowshoot_vision && !rotate_fire_clock ) 
//		 {
//			 Dialpos_set += DIAL_ONE_POSITION ; 
//			 	allowshoot = 0; 
//		 }
//		 rotate_fire_clock++;
//		 if(rotate_fire_clock >= DialFireTime) {rotate_fire_clock = 0;}
//	 }
	}
//�ֶ�����ģʽ
	if(RC.Key.CH[8] && allowshoot && Fire_Mode && DialFireTime)  //DialFireTime-��ֹ����ʱ��������
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
 if( (RC.Key.CH[8] || RC.Key.SW1 == RCS::Down)&& allowshoot && !Fire_Mode && !Heat_limit_mode) 
	{
		if(last_key == 2)
		{
			if(RC.Key.CTRL) {Dialpos_set += DIAL_ONE_POSITION / 3; isDoubleshot = 0;}
			else if(isDoubleshot >= 2) {Dialpos_set += DIAL_ONE_POSITION / 4 * 5; isDoubleshot = 0;}
			else            Dialpos_set += DIAL_ONE_POSITION ;
//			Dialpos_set += DIAL_HALF_POSITION;
//			half_fire_time = 1;
			last_fire_num = fire_num_real;
		}
		last_key = 3 ;
	}
//	if(half_fire_time) {half_fire_time++;}
//	if(half_fire_time > 250)
//	{
//		half_fire_time = 0;
//		Dialpos_set += DIAL_HALF_POSITION;
//	}
	if( RC.Key.CH[8] == 0 && RC.Key.SW1 != RCS::Down)                         //���������ʼ��ֵ
	{
		last_key = 2 ;
	}
//��ת���(�������������������λ��)
//	if(!Fire_Mode)  //����/������ת���
//	{
	
		if(current_now > 8400)
		{
			singlefireback_time++;
		}
		else
		{
			singlefireback_time = 0;
		}
		if(singlefireback_time > 120 && allow_back)
		{
			allowshoot = 0;
			Dialpos_set = Dialpos_record - DIAL_ONE_POSITION / 4;
			allow_back = 0;
		}
//	}
//	 else
//	 {
////��������	
	  if ( (ABS(Dialpos_set - Dialpos_record) > DIAL_ONE_POSITION * 3) )    //������ת���/����Ҳ���Դ���           
			{         
//       allowshoot = 0;  //��ת������������
  			Dialpos_set = Dialpos_set - DIAL_ONE_POSITION * 2;
//				allow_back = 0; 
			}
//		}	
	if(!allow_back)
	{if(current_now < current_limit - 3000) {allowshoot = 1; allow_back = 1;}} 
//���������
	if(!Fire_Mode)
	{
		jgmt_fire_num = fire_num_real - last_fire_num;
		if(jgmt_fire_num>1)
		{
			isDoubleshot++;
		}
	}
//���̣����أ�������
	thisShootNum.sprocketShootNum = (Dialpos_record-Dialpos_initial)/DIAL_ONE_POSITION;
//���̿���	
	Dial->ctrlPosition(Dialpos_set) ;  
}
//���������� ���������/���� ϵͳ��ȴ ����ϵͳ���٣�
void Fire::getFireLimit()
{
	if (!can1Feedback.jgmtOffline)//����ϵͳδ����
	{
		fire.jgmtCoolDown = can1Feedback.CoolRate;                        //����ϵͳ��ȴ
		fire.jgmtBulletSpeedMax = can1Feedback.shooterId1_17mmSpeedLimit; //����ϵͳ�����
		fire.jgmtHeatMax = can1Feedback.MaxHeat;                          //����ϵͳ�������
	}
	else //������ϵͳ���ߣ�������һ������
	{
		fire.jgmtCoolDown = 15;
		fire.jgmtBulletSpeedMax = 30;
		fire.jgmtHeatMax = 75;
	}
//����ģʽѡ���ж�
	if(jgmtCoolDown == 10 && jgmtHeatMax == 200 && jgmtBulletSpeedMax == 15 && !isjudgeover)
	{
		Robot_Fire_Mode = 1; 
		isjudgeover = 1;
	}
	else if(jgmtCoolDown == 40 && jgmtHeatMax == 50 && jgmtBulletSpeedMax == 15 && !isjudgeover)
	{
		Robot_Fire_Mode = 2;
		isjudgeover = 1;
	}
	else if(jgmtCoolDown == 15 && jgmtHeatMax == 75 && jgmtBulletSpeedMax == 30 && !isjudgeover)
	{
		Robot_Fire_Mode = 3;
		isjudgeover = 1;
	}
	fire.jgmtBulletSpeed = can1Feedback.Jgmt_OutSpd;  //����ϵͳ���ٴ���
	vision_SendStruct.realBulletSpeed = fire.jgmtBulletSpeed;//���Ʒ��͵���
}
//����ϵͳ��������
void Fire::jgmtHeatCalc()
{
	if (!can1Feedback.jgmtOffline)
		jgmtHeat = can1Feedback.Jgmt_Heat;//��ȡ����ϵͳ����
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

//	if(sprocketHeat > 0)
	sprocketHeat -= fire.jgmtCoolDown * sprocketHeatTimer.getCycleT();  //������ʱ���Լ�
	sprocketHeat = LIMIT(sprocketHeat, 0, jgmtHeatMax + 20);
}
//Ħ�����ٶ�����/ͬʱ��¼������ 
void Fire::frictionSpdLimit()
{
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
  if(!can1Feedback.jgmtOffline)	//���ٵ������ڲ���ϵͳδ���������
	{
	 if(jgmtBulletSpeed > (jgmtBulletSpeedMax - 0.8))//�ٶȳ����Զ�����
	 {
		friction_spd1 -= 60; 
		 friction_spd2 -= 60; 
	 }
	 else if (jgmtBulletSpeed < jgmtBulletSpeedMax - 5 && jgmtBulletSpeedMax<32 ) //�ٶ�С����ߵ�����ߵ���
	 { 
			friction_spd1 += 100; 
		 friction_spd2 += 100; 
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





