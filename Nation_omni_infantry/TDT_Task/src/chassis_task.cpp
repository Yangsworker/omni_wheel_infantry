/*
*�������ǲ�ͬ/���޸ģ�
*imu.cpp-148
*ptz_task.cpp-send_roll_offset
*ptz_task.cpp-200
*/
/*
*���̸���Ч��:����ʱĬ�ϸ��渺����(����ʱ���̱��ֲ���)����R�ɻָ�����������
*/

#include "chassis_task.h"
#include "dbus.h"
#include "pid.h"
#include "my_math.h"
#include "can.h"
#include "smooth_start.h"
#include "stdlib.h"
#include "state_task.h"


#define random(x) (rand()%x) //����������궨�庯��

 
Chassis chassis ;
PidParam motorpid, followpidOuter; //���̸���pidֻ���������⻷ 


Lpf2p l1, l2, l3;

void Chassis_Task (void)
{
	chassis.chassis_run();
	chassis.motorOutput() ;  //�ٶ�ע��
}

float amp[4] = {7,7,7,7} ;
int8_t drc[3][4] = {{1,-1,-1,1},{1,1,-1,-1},{1,1,1,1}} ;//1˳1��-1˳-1��
//���̳�ʼ��
Chassis::Chassis()
{
  speedUpjudge = new doubleKickJudge;   //����˫��������
	motorpid.kp = 14 ;  //{13, 200, 0, 5, 16384}
	motorpid.ki = 200 ;
	motorpid.kd = 0 ;
	motorpid.integralErrorMax = 0.5 ; //1
	motorpid.resultMax = 16384 ;
	
	followpidOuter.kp = 17 ;  //15 30 0.2 2 2000
	followpidOuter.ki = 40 ;
	followpidOuter.kd = 0.3 ;
	followpidOuter.integralErrorMax = 2 ; 
	followpidOuter.resultMax = 2000 ;
	
	for(u8 i = 0 ; i<4 ; i++)
	{
		Wheel[i] = new Motor(M3508 ,CAN1,0X201+i) ;
		Wheel[i]->pidInner.setPlanNum(1) ;
		Wheel[i]->pidInner.fbValuePtr[0] = &Wheel[i]->canInfo.speed ;
		Wheel[i]->pidInner.paramPtr = &motorpid ;		
		
		Wheel[i]->pidOuter.setPlanNum(1);
		Wheel[i]->pidOuter.fbValuePtr[0] = &angle_poor; //����ֵ��ʵʱˢ��
		Wheel[i]->pidOuter.paramPtr = &followpidOuter;
	}
}

u8 ischanging = 0, ischanging2 = 0, ischanging3 = 0;//��֤�������ֵ���仯һ�Σ���̨��Խǡ�����Ƕȣ�
float gimbalAngle = 0;
//�����̨������������ƫ�ýǣ�360�ƣ���ʵ��180-540��
float Chassis::getGimbalAngle(void)
{
//	gimbalAngle = ptz.Yaw -> canInfo.totalAngle_f - (ptz.Yaw -> canInfo.totalRound - 1) * 360.0f ;
	if(!ptz.direction_change_flag)
	{ 
		ischanging = 0;
		ischanging2 = 0;
		ischanging3 = 0;
		gimbalAngle = ptz.Yaw -> canInfo.totalAngle_f - (ptz.Yaw -> canInfo.totalRound - 1) * 360.0f ;
	}
	if(ptz.direction_change_flag && !ischanging) 
	{
		ischanging = 1;
		gimbalAngle -= 180.0f;
	}
	return gimbalAngle;
//	return ptz.Yaw -> canInfo.totalAngle_f - (ptz.Yaw -> canInfo.totalRound - 1) * 360.0f ;
}

/*���Ƽ�λ/ң��
*��λ���󲦸�����--Ĭ�ϵ��̸���
*      ifChassisNoFollow == 1��������--���̲�����
*      E--С����  ctrl+E--�������ݣ��Զ���ܣ�  E+R--�������
*      yaw��������--���̲�����
*ҡ�ˣ�Ŀǰû��С����ȫĬ�ϵ��̸���
*      
*/
bool Rear_Drive = 0; //����
float follow_error = 0, simulative_angle_now = 0; //����ǰ����������������̨�Ƕȣ�
float sssss = 0;
void Chassis::chassis_run(void)
{
	mode m;
	//ˢ��Ĭ��ֵ
	Rear_Drive = 0;
	Lockchassis = 0;
	//��λѡ��
	if(RC.Key.CH[3] == -660 || canStateStruct_2.e_vserrFlag)//������--�󲦸�����/�����ƫ��ģʽ
	{Lockchassis = 1;}
	if(RC.Key.SW1 == RCS::Mid || RC.Key.SW1 == RCS::Up || RC.Key.SW1 == RCS::Down)   
	{
		power.Check_Mode = 0;
	  	m = ChassisFollow;  //Ĭ�ϵ��̸���
		if(RC.Key.E)       
       {                
			    m = Gyroscope;
				 allow_speeding = RC.Key.R;
				 allow_random_gyro = RC.Key.CTRL;
			 }
	}
  if(RC.Key.SW1 == RCS::Down && deforceFlag)
	{
		m = ChassisFollow;
	 power.Check_Mode = 1;
	}
	if(chassis.ifChassisNoFollow)     //�Ӿ���Ҫ-���̲�����
	{
		m = ChassisNoFollow;
	}
	
	if(ptz.Yaw->canInfo.lostFlag == 1) //��̨����-���߱��� 
	 {m = ChassisProtect;}
	 
	 
if(m != ChassisProtect)
{
	//ǰ�������ƶ�
	Gyro_angle = getGimbalAngle() / 180.0f * 3.1415f;
	double sin_x = 0, sin_y = 0, cos_x = 0, cos_y = 0;
	sin_x = sin(Gyro_angle + 3.1415f/4);
	sin_y = sin(3.1415f/2 - Gyro_angle - 3.1415f/4);	
	cos_x = cos(Gyro_angle + 3.1415f/4);
	cos_y = cos(3.1415f/2 - Gyro_angle - 3.1415f/4);
	chassisSpd[_X] = RC.Key.CH[3] * 1.3 ;   //ǰ��
	chassisSpd[_Y] = RC.Key.CH[2] * 1.3;   //����
	
	chassisSpd[_X] += (RC.Key.W - RC.Key.S) * 420;   //���̿���
  chassisSpd[_Y] += (RC.Key.D - RC.Key.A) * 420;
	
	chassisSpd[_X] = l1.Apply(chassisSpd[_X]);     //��ͨ�˲�ƽ������
	chassisSpd[_Y] = l2.Apply(chassisSpd[_Y]);
	
	motorSpd.data[0] = chassisSpd[_X] * drc[_X][0] * cos_x * amp[0] * Multiplier;
 	motorSpd.data[0] += chassisSpd[_Y] * drc[_Y][0] * cos_y * amp[0] * Multiplier;
	
	motorSpd.data[1] = chassisSpd[_X] * sin_x * drc[_X][1] * amp[1] * Multiplier;                                                                     
 	motorSpd.data[1] += chassisSpd[_Y] * sin_y * drc[_Y][1] * amp[1] * Multiplier;
 
  motorSpd.data[2] = -motorSpd.data[0];
  motorSpd.data[3] = -motorSpd.data[1];
	//ģʽѡ��/R���ƶ�
//Ĭ�ϵ��̸���
if(m == ChassisFollow)
 {
	 //�Զ�����Ƿ�ǰ�ֿ�ת���ֲ�ת���Ǽ��򿪺���ģʽ
	 Rear_Drive = 0;
	 if(ABS(Wheel[0]->pidInner.error) + ABS(Wheel[1]->pidInner.error) - ABS(Wheel[2]->pidInner.error) - ABS(Wheel[3]->pidInner.error) > 1000)
	 {Rear_Drive = 1;}
	 if(Rear_Drive) //���ʷ������
	 {
		 motorSpd.data[0] *= 0.6;
		 motorSpd.data[1] *= 0.6;
		 motorSpd.data[2] *= 1.5;
		 motorSpd.data[3] *= 1.5;
	 }
	 
	 //�޸ı�־λ
	 rotateFlag = 0; flexRotate = 0; ifChassisFollow = 1;
	 //���ڽǶ�Ϊʵ�ʽǶȣ�Ŀ��Ƕ�Ϊ������Ƕ�                                                             
	angle_now = getGimbalAngle();
//	angle_target = 0 ;  
	 	 if(ptz.direction_change_flag)
	 {
		 if(angle_target == 0 && !ischanging2)
		 {
		   angle_target = 180;
			 ischanging2 = 1;
		 }
		 if(angle_target == 180 && !ischanging2)
		 {
			 angle_target = 0;
			 ischanging2 = 1;
		 }
	 }
	 if(RC.Key.R)
	 {
		 angle_target = 0;
	 }
  //���Ƕ�ֵ	   
	angle_poor = angle_target - angle_now ; float dddddd = angle_poor;
	if(angle_poor < -180) {angle_poor = 360 + dddddd;}
  //����pid�⻷�����ٶ�ֵ
	for(u8 a = 0; a<4; a++)
	{
		Wheel[a]->pidOuter.Calculate(0); 
	}  
	//pidǰ�����㣨���趨ֵΪĿ����㣩
	simulative_angle_now = ptz.motorPos2[YAW] ;
//	follow_error = ptz.Yaw->pidOuter.result;
//	follow_error = LIMIT(follow_error, -1400, 1400);
	if(!ptz.direction_change_flag)
	{
	  follow_error = ptz.Yaw->pidOuter.result;
	  follow_error = LIMIT(follow_error, -1400, 1400);
	}
	if(ptz.direction_change_flag)
	{
		follow_error = 0;
	}
	chassisSpd[_R] = follow_error * 8.2; //8.0   //�˴�ʹ����̨�������ֵ������̸���ǰ��
	sssss = Wheel[0]->pidOuter.result;
//	//ԭ��ң�ˡ����ǰ��
//  chassisSpd[_R] = RC.Key.CH[0] * 4.0; 
//	chassisSpd[_R] += RC.Key.CH[6] * 8.8;  //������
	for(u8 i = 0 ; i<4 ;i++) 
  {
		motorSpd.data[i] += (Wheel[i]->pidOuter.result * -2.5 + chassisSpd[_R] * -1.0) * drc[_R][i] ;  //-2.1�� -1.0
  }
 }
//����ģʽ 
if(m == Gyroscope)
{
	rotateFlag = 1; flexRotate = 0; ifChassisFollow = 0;
	if(allow_speeding)  {SpeedingGyro();} //��������
	else if(allow_random_gyro) {randomspeed_gyro();} //�������
	else {speedingmult = 1.0, random_mult = 1.0;}
	
	for(int i = 0; i < 4; i++)
 {
	 motorSpd.data[i] += Gyro_speed * drc[_R][i] * 4 * Gyro_mult * speedingmult * random_mult;
 }
}
//���̲�����ģʽ/����С����
if(m == ChassisNoFollow)
{
	if(RC.Key.E) 
	{
		for(int i = 0; i < 4; i++)
    {
	     motorSpd.data[i] += Gyro_speed * drc[_R][i] * 4.0 * Gyro_mult ;
    }
	}
}

}

//���߱���
if(m == ChassisProtect)
{
	ChNoFollow();
}
}


//���̲����棨�����ù��ֿ���ת��
void Chassis::ChNoFollow(void)  
{
	chassisSpd[_X] = RC.Key.CH[3] ;   
	chassisSpd[_Y] = RC.Key.CH[2] ;   
	chassisSpd[_R] = RC.Key.CH[11] ; 
	
	chassisSpd[_X] += (RC.Key.W - RC.Key.S) * 350;  //���̿���
  chassisSpd[_Y] += (RC.Key.D - RC.Key.A) * 350;	
	if(RC.Key.E)  //���̲�����ʱҲ�������ݣ���δ���ԣ�
	{
		chassisSpd[_R] = 200;
	}
	for(u8 i = 0 ; i<4 ;i++)
 {
		motorSpd.data[i] = chassisSpd[_X] * drc[_X][i] * amp[i] * Multiplier;
 	  motorSpd.data[i] += chassisSpd[_Y] * drc[_Y][i] * amp[i] * Multiplier;
		motorSpd.data[i] += chassisSpd[_R] * drc[_R][i] * amp[i] * Multiplier;
 }
}

//��������
float cc = 0;
int ee = 0, dd = 10;
void Chassis::SpeedingGyro()
{
	
	rotateFlag = 0; flexRotate = 1; ifChassisFollow = 0; //ģʽ��־λ�޸�
	float ptz_angle = ptz.Yaw->canInfo.totalAngle_f - 360.0f * ptz.Yaw->canInfo.totalRound + 180; 
 
	cc = ptz_angle;
	
	    if((ptz_angle>60 && ptz_angle<110)||(ptz_angle>150 && ptz_angle<200)||(ptz_angle>240 && ptz_angle<290)||(ptz_angle>330 || ptz_angle<20))
			{
				dd = 1;
				if(ee<36)
				{ 
					ee++;
					speedingmult = 0.7 + ee/35*0.4;
				}
				else
				{
				  speedingmult = 1.1; 
				}
			}
			else
			{
				ee = 1;
				if(dd < 36)
				{
				  dd++;
			  	speedingmult = 1.1 - dd/35*0.4;
				}
				else
				{
					speedingmult = 0.7 ;
				}
      }
}
//�������
int i = 0;
void Chassis::randomspeed_gyro(void)
{
	if(ifnext_random)
	{
	  provide_random_seeds++;             //�ۼ�
	  random_mult_last = random_mult_set; //�ϴε�ֵΪ�ϴ��趨��ֵ����ʼֵΪ1��
    srand(provide_random_seeds);        //�������������
    random_mult_set = 1 + random(8)/10.0f; //��ȡ������ʣ�1~2��
	  random_mult_error = random_mult_set - random_mult_last; //�������趨���ϴ�
		ifnext_random = 0;
	} 
	i++;
	if(i <= 750)
	{
		random_cycle += 0.12 * 3.1415 / 180; //ÿ2ms����0.12�ȣ�1.5������90��
	  random_mult = random_mult_last + random_mult_error * sin(random_cycle);
	}
	if(i == 750) {ifnext_random = 1; random_cycle = 0, i = 0;} //ѭ��������ʼ��һ�����������������
	
}

//�жϵ����Ƿ��ƶ������ٶȺ��ж�(�����Զ��ص��ո�/���ʱ��1s)
bool Chassis::judgeIfMoving()
{
	static int time;
	if(ABS(motorSpd.data[0])>0||ABS(motorSpd.data[1])>0||ABS(motorSpd.data[2])>0)
	{
		float speedtemp;
		for(u8 i = 0; i<4; i++)
		{
			speedtemp += ABS(Wheel[i]->canInfo.speed);
		}
		if(speedtemp>250)
		{
			time++;
			if(time>500)
			{
				time = 0;
				return 1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			time = 0;
			return 0;
		}
	}
	else
	{
		time = 0;
		return 0;
	}
}

//���ʿ���ˢ��
/*
*CTRL+SHIFT+A+D--����ULTSģʽ 0.4s CD
*SW1->Down--��¼ģʽ
*/
int ULTS_CD = 0;
void Chassis::powerCtrlUpdate()
{
	power.flySloping = RC.Key.V;   //��V�������٣��ɿ�ȡ��
	power.speedUpLevel = power.flySloping?3:speedUpjudge->doubleKickVal(RC.Key.SHIFT);
	slowRun = (RC.Key.CTRL && !RC.Key.SHIFT && !RC.Key.Q);  //����

	//����˫����⡢����Ƿ����ߡ�����ģ�鷢����Ϣ������޷�ϵ��
//  if(RC.Key.CTRL && RC.Key.SHIFT && RC.Key.A && RC.Key.D && !ULTS_CD)	
//  {power.ULTS_Mode = !power.ULTS_Mode; ULTS_CD = 1;}
//	if(ULTS_CD)  {ULTS_CD++;}
//	if(ULTS_CD > 200) {ULTS_CD = 0;}
	
	power.useCap = 1;             //ʹ�õ���/Ĭ�Ͽ���
	power.myPowerOfflineJudge();  //����Ƿ�����
	power.myPowerCanTx();         //2ms��һ��
	power.getPowerKp();           //��ù����޷�ϵ��
}
//�ٶ����
void Chassis::motorOutput()
{
	powerCtrlUpdate();   
	
	if(power.speedUpLevel == 1 ) {Multiplier = 1.6; Gyro_mult = 1.5;} //shift�ֶ�������ֵ(�������ָܻ�)
	else if(power.speedUpLevel == 2 ) {Multiplier = 1.85; Gyro_mult = 1.6;}
	
	else if(power.speedUpLevel == 3 )
  {
		Multiplier = Smooth_1.smooth_cos(1.5, 2.7);  //���һ���
		Gyro_mult = 1.5;
  }
	
  else if(power.capacitance_percentage <= 60) { Multiplier = 0.8; Gyro_mult = 1;} //��������
	else if((power.capacitance_percentage <= 65) && (power.capacitance_percentage >60) ) {Multiplier = 1; Gyro_mult = 1;}
	else {Multiplier = 1.35; Gyro_mult = 1.25;}
	if((ABS(chassisSpd[_X]) + ABS(chassisSpd[_Y])) > 400) {Gyro_mult *= 0.8;}
	if(slowRun)  {Multiplier = 0.1;}
	
	lastSpeedLevel = power.speedUpLevel; //��¼�ϴμ��ٵȼ�
	
  if (Lockchassis)    //�Ƿ������� 
  { 
    memset(motorSpd.data, 0, sizeof(motorSpd.data));//���ÿ⺯��memset��0���Ƶ�����allspeed��ָ����ַ����������ַ�
  }
//	motorSpd.data[0] = Smooth_1.smooth_xy(0, motorSpd.data[0]);
//  motorSpd.data[1] = Smooth_2.smooth_xy(0, motorSpd.data[1]);
//	motorSpd.data[2] = Smooth_3.smooth_xy(0, motorSpd.data[2]);
//	motorSpd.data[3] = Smooth_4.smooth_xy(0, motorSpd.data[3]);
    for (u8 i = 0; i < 4; i++)
    {
//			Wheel[i]->pidInner.Calculate(motorSpd.data[i]);		
//			Wheel[i]->setPowerOutLimit(&power.powerLimitKp); //pid���㷽ʽ
//			Wheel[i]->ctrlCurrent(Wheel[i]->pidInner.result);
			Wheel[i]->setPowerOutLimit(&power.powerLimitKp); //�������ƿ���
			Wheel[i]->ctrlSpeed(motorSpd.data[i]);
    }
}

