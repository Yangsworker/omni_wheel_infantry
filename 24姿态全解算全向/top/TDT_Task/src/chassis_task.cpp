//����ע�͹淶
/**
@brief  ����飬�򵥽��ܺ�������
@param  �����ܺ�������
@return��������������˵��
@exception NSException �����׳����쳣.
@author zhangsan��  ����
@date 2023-07-27 22:30:00 ��ʱ��
@version 1.0 ���汾  
@property �����Խ���
*/

#include "chassis_task.h"
#include "imu_task.h"
#include "imu.h"
#include "my_math.h"
#include "dbus.h"
#include "pid.h"
#include "Gimbal_task.h"
#include "com_task.h"
#include "myRC_task.h"
#include "led_task.h"

Chassis chassis;

Pid Followpid, turnSpdpid, BFSpdpid, LRSpdpid;
PidParam MotorpidPr, FollowpidPr, turnSpdpidPr, BFSpdpidPr, LRSpdpidPr; //���̸���pidֻ���������⻷ 
Lpf2p BF_Lp, LR_Lp, turn_Lp, TurnSpdLimit_Lp, Slip1_Lp, Slip2_Lp; 
Kf WheelSpd_kf[4];

#define SPEED_MAX 25   //�ٶ�������ƣ��������������ٶ�

/**
*@brief ��ȡ����Ƕ�
*@note  ��������������
*/
float Chassis::getGimbalAngle(void)
{
	float angle_return = 0;
  angle_return = gimbal.Yaw->canInfo.totalAngle_f - (gimbal.Yaw->canInfo.totalRound - 1) * 360.0f; //���ؽǶȳ�ʼ��
	angle_yaw = gimbal.Yaw->canInfo.totalAngle_f - (gimbal.Yaw->canInfo.totalRound - 1) * 360.0f ;   //���ԽǶ�
//*******************�����������̨�Ƕȣ����ڻ���ui��***********************
	yaw_angle_draw = angle_yaw - YAW_OFFSET;
	if(yaw_angle_draw < 0) yaw_angle_draw += 360.0f;
//*******************��ȡ��̨װ�������ڴ��޸ĽǶ��Ի�������*****************
	angle_return -= YAW_OFFSET; 
//*******************��ȡ���������Ƕ�***********************	
  if(angle_return >= 90 && angle_return <= 180)    //�泯ǰ����תΪ��
	{
		angle_return -= 180.0f;
	}
	else if(angle_return > 180 && angle_return <= 270)  
	{
		angle_return -= 180.0f;
	}
	else if(angle_return > 270 && angle_return <= 360)  
	{
		angle_return -= 360.0f;
	}
	else if(angle_return > 360 && angle_return <= 450)
	{
		angle_return -= 360.0f;
	}
	else if(angle_return >= -180 && angle_return <= -90)
	{
		angle_return += 180.0f;
	}
	else if(angle_return >= -270 && angle_return <= -180)
	{
		angle_return += 180.0f;
	}
//	else if(angle_return >= -360 && angle_return <= -270)
//	{
//		angle_return += 360.0f;
//	}
	return angle_return;	
}

/**
*@brief ����ģʽѡ��(Ĭ�ϸ���)
*@note  ��¼ģʽ����
*/
mode chassis_mode, last_chassis_mode;
void Chassis::Chassis_mode(void)
{
//Ĭ��ģʽ
	chassis_mode = FOLLOW;  
//�����̣���̨�����Զ������棩	
	if(RC.Key.CH[3] == -660 || gimbal.Yaw->canInfo.lostFlag == 1)
	  chassis_mode = NO_FOLLOW;
//����--���ȼ���
  if(RC.Key.E || RC.Key.CH[11])                      
		chassis_mode = GYRO;
//�������ȼ����	
	if(deforceFlag)
	{
		chassis_mode = DEFORCE;
	}
}




/**
*@brief �����ٶȹ۲�(�����˶�ѧ���)
*@note  ������������ٶȿ����������ǽ��ٶ��滻
*@todo  ʹ�þ�����ȷ�������ǽ��ٶ�����
*@note  �����߼����������ٶ��ڼ�ȥ�����ٶȺ󣬶Խ����ϵ�����ٶ�Ӧ�þ���ֵ��ȣ����Ը��ݾ���ֵС���ٶ���Ϊ�öԽ����ϵĵ���ٶ�
*/
float aaa = 0, bbb = 0;
void Chassis::Chassis_speedView(void)
{
//��ȡ���������ǽ��ٶ�
	yaw_dps = JgmtPack.yaw_dps / 500.0f;
	for(int a = 0; a < 3; a++)
	{
    Speed_View[a] = 0;
	}
	for(int i = 0; i < 4; i++)
	{
		WheelSpd_View[i] = Wheel[i]->canInfo.speed / 60.0f / 268.0f * 17.0f * 0.47752f;  //�õ�ÿ����������ٶ�(���ٱȡ����ܳ�)
		WheelSpd_View[i] = WheelSpd_kf[i].KalmanFilter(WheelSpd_View[i], WS_Q, WS_R);

	}
	//����������ʹ�ñ��ؽ���
	if(botCtrlerOffline)  
	{
		for(int b = 0; b < 4; b++)
    {		
		  Speed_View[_X] += WheelSpd_View[b] * drc[_X][b] * 10.0f;
		  Speed_View[_Y] += WheelSpd_View[b] * drc[_Y][b] * 10.0f;
		  Speed_View[_R] += WheelSpd_View[b] * drc[_R][b];
		}
    Speed_View[_X] /= 4.0f * SQRT_TWO;
	  Speed_View[_Y] /= 4.0f * SQRT_TWO;
	  Speed_View[_R] /= 4.0f * CHASSIS_RAD;   //���ٶ�
		//ʹ�ñ��ؼ��㲻���д򻬲���
		Slip_coeff1 = 0;
		Slip_coeff2 = 0;
	}
	else
	{
		float yawspd_offset = yaw_dps * CHASSIS_RAD;
		for(int b = 0; b < 4; b++)
		{
			WheelSpd_View[b] -= yawspd_offset;  //��ȡ�������ٶ���������ٶ�
		}
	//���������ǽ��ٶȲ�������д�ϵ������
		Slip_Check();
	//ѡȡ����ֵС���ٶ���Ϊ�Խǵ���ٶ�
		if(ABS(WheelSpd_View[0]) < ABS(WheelSpd_View[2]))  
			WheelSpd_View[2] = -1.0f * (WheelSpd_View[0] * 0.6f + WheelSpd_View[2] * -0.4f);
		else
			WheelSpd_View[0] = -1.0f * (WheelSpd_View[2] * 0.6f + WheelSpd_View[0] * -0.4f);
		if(ABS(WheelSpd_View[1]) < ABS(WheelSpd_View[3]))  
			WheelSpd_View[3] = -1.0f * (WheelSpd_View[1] * 0.6f + WheelSpd_View[3] * -0.4f);
		else
			WheelSpd_View[1] = -1.0f * (WheelSpd_View[3] * 0.6f + WheelSpd_View[1] * -0.4f);
		for(int c = 0; c < 4; c++)
		{
			Speed_View[_X] += WheelSpd_View[c] * drc[_X][c] * 10.0f;
	  	Speed_View[_Y] += WheelSpd_View[c] * drc[_Y][c] * 10.0f;
		}
		Speed_View[_X] /= 4.0f * SQRT_TWO;
	  Speed_View[_Y] /= 4.0f * SQRT_TWO;
	  Speed_View[_R] = yaw_dps;   //���ٶ�
	}
//��ȡ�ٶȱ���
	float Speed_ture = sqrt(Speed_View[_X] * Speed_View[_X] + Speed_View[_Y] * Speed_View[_Y]);
	Turnspd_limit = 1 - ABS(Speed_ture) / SPEED_MAX;
	Turnspd_limit = TurnSpdLimit_Lp.Apply(Turnspd_limit);
	Turnspd_limit = LIMIT(Turnspd_limit, 0.1, 1);
//	aaa = yaw_dps;
//	bbb = Speed_View[_R];
//	aaa *= 1.0f;
//	bbb *= 1.0f;
}



/**
*@brief �򻬼��
*@note  ���ݶԽ����ټ���򻬲���ϵ��������Сϵ�������ϵ����ֱ���ƣ�
*/
void Chassis::Slip_Check(void)
{
	float Spd_dif1 = ABS(WheelSpd_View[0] + WheelSpd_View[2]);
	float Spd_dif2 = ABS(WheelSpd_View[1] + WheelSpd_View[3]);
	ifSlip = 0;
	Slip_coeff1 = 0; Slip_coeff2 = 0;  //��ʼ���򻬲���ϵ��
//		if(Spd_dif1 > 0.01)
//		{
			Slip_coeff1 = (Spd_dif1) / 1.0f;
			Slip_coeff1 = LIMIT(Slip_coeff1, 0, 1);
			if(ABS(WheelSpd_View[0]) < ABS(WheelSpd_View[2])) 
		  {
			  Slip_coeff1 *= -1;
		  }
	//	}
	//	if(Spd_dif2 > 0.010)
		//{
			Slip_coeff2 = (Spd_dif2) / 1.0f;
			Slip_coeff2 = LIMIT(Slip_coeff2, 0, 1);
			if(ABS(WheelSpd_View[1]) < ABS(WheelSpd_View[3])) 
		  {
			  Slip_coeff2 *= -1;
		  }
		//}
//		Slip_coeff1 = Slip1_Lp.Apply(Slip_coeff1);
//		Slip_coeff2 = Slip2_Lp.Apply(Slip_coeff2);
}

/**
*@brief ���Ƽ��ٶ�+���ݹ��ʵ��ٶ�����
*@note  ����+��ɲ
*@todo  ǰ���ٶ������ֵ�һ��ֵ���丳��ת���ٶ��ϣ�����ʱ�������±�ֱ�����¸������ף�
*/
void Chassis::Speed_Limit(float speed_set)
{
  JgmtPack.powerlimitKp /= 100.0f;
}

/**
*@brief ���̿���
*@todo  ���Կ��Ǿ�ֹ����ʱʹ�����鵥������ʹ����ƽ��
*/
void Chassis::Chassis_ctrl(void)
{
//----�����ٶȹ۲�
	Chassis_speedView();
//----������̨��ԽǶȻ�ȡ
	yaw_angle = getGimbalAngle();
	yaw_angle_deathroom = my_deathzoom(yaw_angle - 0.8,0.8f);
//	yaw_angle = getGimbalAngle();
//----����ģʽ��ȡ
	Chassis_mode();
//----����ֵ��ȡ
	//ƽ���ٶȻ�ȡ��λ��dm/s
	if((angle_yaw < YAW_OFFSET + 90.0f) && (angle_yaw > YAW_OFFSET - 90.0f))
	{
		if(angle_state)
		{
			Speed_set[_X] *= -1.0f;
			Speed_set[_Y] *= -1.0f;
		}
		angle_state = 0;
		if(Speed_set[_X] < -myRC.mySpdSetX)
		{
			Speed_set[_X] += 0.08;
		}
		else if(Speed_set[_X] > -myRC.mySpdSetX)
		{
			Speed_set[_X] -= 0.08;
		}
		if(myRC.mySpdSetX == 0) Speed_set[_X] = 0;
		

		if(Speed_set[_Y] < -myRC.mySpdSetY)
		{
			Speed_set[_Y] += 0.08;
		}
		else if(Speed_set[_Y] > -myRC.mySpdSetY)
		{
			Speed_set[_Y] -= 0.08;
		}
		if(myRC.mySpdSetY == 0) Speed_set[_Y] = 0;
		
//		Speed_set[_X] = myRC.mySpdSetX;   //ƽ���趨ֵ��λ��dm/s
//	  Speed_set[_Y] = myRC.mySpdSetY;
	}
	else
	{
		if(!angle_state)
		{
			Speed_set[_X] *= -1.0f;
			Speed_set[_Y] *= -1.0f;
		}
		angle_state = 1;
		if(Speed_set[_X] < myRC.mySpdSetX)
		{
			Speed_set[_X] += 0.08;
		}
		else if(Speed_set[_X] > myRC.mySpdSetX)
		{
			Speed_set[_X] -= 0.08;
		}
		if(myRC.mySpdSetX == 0) Speed_set[_X] = 0;
		

		if(Speed_set[_Y] < myRC.mySpdSetY)
		{
			Speed_set[_Y] += 0.08;
		}
		else if(Speed_set[_Y] > myRC.mySpdSetY)
		{
			Speed_set[_Y] -= 0.08;
		}
		if(myRC.mySpdSetY == 0) Speed_set[_Y] = 0;
		
//		Speed_set[_X] = -myRC.mySpdSetX;
//	  Speed_set[_Y] = -myRC.mySpdSetY;
	}
	//��ת�ٶȻ�ȡ
	if(chassis_mode == FOLLOW)  //�����趨ֵ�ǽǶȣ��ں���ת��Ϊ����
	{
  //������ģʽ�������ģʽʱ�жϸ��淽�򣬱��ⷴ����
		if(last_chassis_mode == GYRO)
		{
			if(yaw_dps > 0) gyro_dir = 1;
			if(yaw_dps < 0) gyro_dir = 0;
			if(gyro_dir && yaw_angle > 0)
				turn_dir_offset = 1;
			if(!gyro_dir && yaw_angle < 0)
				turn_dir_offset = 2;
		}	
		if(turn_dir_offset == 1)
		{
			if(yaw_angle > 0) Speed_set[_R] = 400.0f;
			else turn_dir_offset = 0;
		}
    else if(turn_dir_offset == 2)
		{
			if(yaw_angle < 0) Speed_set[_R] = -400.0f;
			else turn_dir_offset = 0;
		}
		else
		Speed_set[_R] = Followpid.Calculate(0) - RC.Key.CH[0] / 3.0f;  //ǰ����ʹ����̨�趨ֵ
	}
	else if(chassis_mode == NO_FOLLOW)
	{
		Speed_set[_R] = 0;
	}
	else if(chassis_mode == GYRO) //����ģʽ��ת�ٶ���ʵ���ٶ��������С
	{
    //todo  ����
		Speed_set[_R] = RC.Key.CH[11] * 0.8f + RC.Key.E * 500.0f;  
		Speed_set[_R] *= Turnspd_limit;
	}
	last_chassis_mode = chassis_mode;
//----�趨ֵ������ת��
	float sin_x = my_sin(yaw_angle * RAD_PER_DEG);
	float cos_x = my_cos(yaw_angle * RAD_PER_DEG);
	Speed_real_set[_X] = Speed_set[_X] * cos_x - Speed_set[_Y] * sin_x;
	Speed_real_set[_Y] = Speed_set[_X] * sin_x + Speed_set[_Y] * cos_x;
	Speed_real_set[_R] = Speed_set[_R] / 180.0f * 3.14159f;
  //��ͨ�˲�
	Speed_real_set[_X] = BF_Lp.Apply(Speed_real_set[_X]);
	Speed_real_set[_Y] = LR_Lp.Apply(Speed_real_set[_Y]);
	Speed_real_set[_R] = turn_Lp.Apply(Speed_real_set[_R]);
//�ٶ�����
//	Speed_Limit();
//----�趨ֵ�������ٶȵ�ӳ��
	for(int i = 0; i < 4; i++)
	{
		WheelSpd_Set_BF[i]  = Speed_real_set[_X] * drc[_X][i] * amp[i];
	  WheelSpd_Set_LR[i]  = Speed_real_set[_Y] * drc[_Y][i] * amp[i];
	  WheelSpd_Set_turn[i]= Speed_real_set[_R] * drc[_R][i] * amp[i];
	}
//----�������
	Motor_ctrl();
}

/**
*@brief ������ƺ���
*@note ��ʲô�ط����й������ƣ�û�й���ʱ�趨ֵ���������ͨͬʱ���У�
*/
void Chassis::Motor_ctrl(void)
{
	float Current_set[4] = {0};  //��¼ÿ�����ӵ����趨ֵ
	float max_current = 0;  //��¼�������趨ֵ
	Current_set_BF   =   BFSpdpid.Calculate(Speed_real_set[_X]);
	Current_set_LR   =   LRSpdpid.Calculate(Speed_real_set[_Y]);
	Current_set_turn = turnSpdpid.Calculate(Speed_real_set[_R]);
	
	double drc_t[3][4] = {{-1,1,1,-1},{-1,-1,1,1},{1,1,1,1}} ;  //��ʼ�������ʲ���
//���������ܵ���ֵ����	
	//ѡȡ�������趨ֵ
	for(int i = 0; i < 4; i++)
	{
		Current_set[i] = ABS(Current_set_BF * drc[_X][i] + Current_set_LR * drc[_Y][i] + Current_set_turn * drc[_R][i]);
	}
	max_current = Current_set[0];
	for(int a = 1; a < 4; a++)
	{
		if(max_current < Current_set[a])
			max_current = Current_set[a];
	}
	//�����趨ֵ������ֵ��������
	if(max_current > 17000)  
	{
		float limitKp_turn = 1.0f - (max_current - 17000) / max_current;
		float limitKp_BF = 1.0f - (max_current - 17000) / max_current;
		float limitKp_LR = 1.0f - (max_current - 17000) / max_current;
		limitKp_turn = LIMIT(limitKp_turn, 0.5f, 1.0f);
		limitKp_BF = LIMIT(limitKp_BF, 0.6f, 1.0f);
		limitKp_LR = LIMIT(limitKp_LR, 0.5f, 1.0f);
		for(int b = 0; b < 4; b++)
		{
			drc_t[_X][b] *= limitKp_BF;
			drc_t[_Y][b] *= limitKp_LR;
			drc_t[_R][b] *= limitKp_turn;
		}
	}
//���ݴ򻬽�������
	for(int s = 0; s < 3; s++)
	{
		drc_t[s][0] *= (1.0 - Slip_coeff1);
		drc_t[s][1] *= (1.0 - Slip_coeff2);
		drc_t[s][2] *= (1.0 + Slip_coeff1);
		drc_t[s][3] *= (1.0 + Slip_coeff2);
	}
//	drc_t[_R][0] *= (1.0 - Slip_coeff1 / 1.0 + Slip_coeff2 / 1.0);
//	drc_t[_R][1] *= (1.0 - Slip_coeff2 / 1.0 + Slip_coeff1 / 1.0);
//	drc_t[_R][2] *= (1.0 - Slip_coeff1 / 1.0 + Slip_coeff2 / 1.0);
//	drc_t[_R][3] *= (1.0 - Slip_coeff2 / 1.0 + Slip_coeff1 / 1.0);
//���ݹ��ʵ����������	
	power_limit = JgmtPack.powerlimitKp / 100.0f;
	for(int i = 0; i < 4; i++)
	{
		Current_set_fin[i] = Current_set_BF * drc_t[_X][i] + Current_set_LR * drc_t[_Y][i] + Current_set_turn * drc_t[_R][i];
		Wheel[i]->ctrlCurrent(Current_set_fin[i] * power_limit);
	}
}



/**
*@brief ���̶����ʼ��
*@note ����pid������pid��ʼ��
*/
Chassis::Chassis()
{
	speedUpjudge = new doubleKickJudge;   //����˫��������
	MotorpidPr.kp = 13 ;  //{13, 200, 0, 5, 16384}
	MotorpidPr.ki = 200 ;
	MotorpidPr.kd = 0 ;
	MotorpidPr.integralErrorMax = 0.5 ; //1
	MotorpidPr.resultMax = 16384 ;

	for(u8 i = 0 ; i<4 ; i++)
	{
		Wheel[i] = new Motor(M3508,CAN1,0X201+i) ;
		Wheel[i]->pidInner.setPlanNum(1) ;
		Wheel[i]->pidInner.fbValuePtr[0] = &Wheel[i]->canInfo.speed ;
		Wheel[i]->pidInner.paramPtr = &MotorpidPr ;		
	}
	WS_Q = 0.1; //��������
	WS_R = 1.0; //��������
}


void my_pid_init(void)
{
	FollowpidPr.kp = 10;  //15 30 0.2 2 2000
	FollowpidPr.ki = 0;
	FollowpidPr.kd = 0.5;
	FollowpidPr.integralErrorMax = 0; 
	FollowpidPr.resultMax = 400;   //���ֵΪ�����ٶȣ��Ƕ��ƣ�
	
	BFSpdpidPr.kp = 1300;
	BFSpdpidPr.ki = 0;
	BFSpdpidPr.kd = 10;
	BFSpdpidPr.integralErrorMax = 0;
	BFSpdpidPr.resultMax = 14000;
	
	LRSpdpidPr.kp = 1300;
	LRSpdpidPr.ki = 0;
	LRSpdpidPr.kd = 10;
	LRSpdpidPr.integralErrorMax = 0;
	LRSpdpidPr.resultMax = 12000;
	
	turnSpdpidPr.kp = 1500;  //�˴�����Ϊ������
	turnSpdpidPr.ki = 0;
	turnSpdpidPr.kd = 30;
	turnSpdpidPr.integralErrorMax = 0;
	turnSpdpidPr.resultMax = 8500;  //13000
	
	BF_Lp.SetCutoffFreq(1000, 50);
	LR_Lp.SetCutoffFreq(1000, 50);
	turn_Lp.SetCutoffFreq(1000, 30);
	TurnSpdLimit_Lp.SetCutoffFreq(1000, 20);  //����ʵ��ƽ���ٶ����������ٶ�ϵ��
	Slip1_Lp.SetCutoffFreq(1000, 20);
	Slip2_Lp.SetCutoffFreq(1000, 20);
		
	Followpid.setPlanNum(1);
	Followpid.fbValuePtr[0] = &chassis.yaw_angle; //����ֵΪ��̨����̽ǶȲ�ֵ
	Followpid.paramPtr = &FollowpidPr;
	
	BFSpdpid.setPlanNum(1); 
	BFSpdpid.fbValuePtr[0] = &chassis.Speed_View[0];  //ǰ���ƶ�
	BFSpdpid.paramPtr = &BFSpdpidPr;
	
	LRSpdpid.setPlanNum(1);
	LRSpdpid.fbValuePtr[0] = &chassis.Speed_View[1]; //�����ƶ�
	LRSpdpid.paramPtr = &LRSpdpidPr;
	
	turnSpdpid.setPlanNum(1);
	turnSpdpid.fbValuePtr[0] = &chassis.Speed_View[2]; //����
	turnSpdpid.paramPtr = &turnSpdpidPr;  
}







































