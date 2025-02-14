//函数注释规范
/**
@brief  ：简介，简单介绍函数作用
@param  ：介绍函数参数
@return：函数返回类型说明
@exception NSException 可能抛出的异常.
@author zhangsan：  作者
@date 2023-07-27 22:30:00 ：时间
@version 1.0 ：版本  
@property ：属性介绍
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
PidParam MotorpidPr, FollowpidPr, turnSpdpidPr, BFSpdpidPr, LRSpdpidPr; //底盘跟随pid只用来计算外环 
Lpf2p BF_Lp, LR_Lp, turn_Lp, TurnSpdLimit_Lp, Slip1_Lp, Slip2_Lp; 
Kf WheelSpd_kf[4];

#define SPEED_MAX 25   //速度最大限制，用于限制陀螺速度

/**
*@brief 获取跟随角度
*@note  可能需添加新情况
*/
float Chassis::getGimbalAngle(void)
{
	float angle_return = 0;
  angle_return = gimbal.Yaw->canInfo.totalAngle_f - (gimbal.Yaw->canInfo.totalRound - 1) * 360.0f; //返回角度初始化
	angle_yaw = gimbal.Yaw->canInfo.totalAngle_f - (gimbal.Yaw->canInfo.totalRound - 1) * 360.0f ;   //绝对角度
//*******************底盘相对于云台角度（用于绘制ui）***********************
	yaw_angle_draw = angle_yaw - YAW_OFFSET;
	if(yaw_angle_draw < 0) yaw_angle_draw += 360.0f;
//*******************获取云台装配误差（可在此修改角度以换正方向）*****************
	angle_return -= YAW_OFFSET; 
//*******************获取底盘需跟随角度***********************	
  if(angle_return >= 90 && angle_return <= 180)    //面朝前，左转为正
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
*@brief 底盘模式选择(默认跟随)
*@note  检录模式开闭
*/
mode chassis_mode, last_chassis_mode;
void Chassis::Chassis_mode(void)
{
//默认模式
	chassis_mode = FOLLOW;  
//锁底盘（云台离线自动不跟随）	
	if(RC.Key.CH[3] == -660 || gimbal.Yaw->canInfo.lostFlag == 1)
	  chassis_mode = NO_FOLLOW;
//陀螺--优先级高
  if(RC.Key.E || RC.Key.CH[11])                      
		chassis_mode = GYRO;
//脱力优先级最高	
	if(deforceFlag)
	{
		chassis_mode = DEFORCE;
	}
}




/**
*@brief 底盘速度观测(底盘运动学逆解)
*@note  逆解所得陀螺速度可以用陀螺仪角速度替换
*@todo  使用绝对正确的陀螺仪角速度修正
*@note  修正逻辑：四轮子速度在减去陀螺速度后，对角线上电机的速度应该绝对值相等，可以根据绝对值小的速度作为该对角线上的电机速度
*/
float aaa = 0, bbb = 0;
void Chassis::Chassis_speedView(void)
{
//获取底盘陀螺仪角速度
	yaw_dps = JgmtPack.yaw_dps / 500.0f;
	for(int a = 0; a < 3; a++)
	{
    Speed_View[a] = 0;
	}
	for(int i = 0; i < 4; i++)
	{
		WheelSpd_View[i] = Wheel[i]->canInfo.speed / 60.0f / 268.0f * 17.0f * 0.47752f;  //得到每个电机的线速度(减速比、轮周长)
		WheelSpd_View[i] = WheelSpd_kf[i].KalmanFilter(WheelSpd_View[i], WS_Q, WS_R);

	}
	//下主控离线使用本地解算
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
	  Speed_View[_R] /= 4.0f * CHASSIS_RAD;   //角速度
		//使用本地计算不进行打滑补偿
		Slip_coeff1 = 0;
		Slip_coeff2 = 0;
	}
	else
	{
		float yawspd_offset = yaw_dps * CHASSIS_RAD;
		for(int b = 0; b < 4; b++)
		{
			WheelSpd_View[b] -= yawspd_offset;  //获取除陀螺速度外的所有速度
		}
	//进行陀螺仪角速度补偿后进行打滑系数计算
		Slip_Check();
	//选取绝对值小的速度作为对角电机速度
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
	  Speed_View[_R] = yaw_dps;   //角速度
	}
//获取速度比例
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
*@brief 打滑检测
*@note  根据对角轮速计算打滑补偿系数（以最小系数、最大系数或分别控制）
*/
void Chassis::Slip_Check(void)
{
	float Spd_dif1 = ABS(WheelSpd_View[0] + WheelSpd_View[2]);
	float Spd_dif2 = ABS(WheelSpd_View[1] + WheelSpd_View[3]);
	ifSlip = 0;
	Slip_coeff1 = 0; Slip_coeff2 = 0;  //初始化打滑补偿系数
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
*@brief 限制加速度+根据功率的速度限制
*@note  缓启+急刹
*@todo  前进速度误差积分到一定值后将其赋在转向速度上（上坡时陀螺上坡比直接上坡更加容易）
*/
void Chassis::Speed_Limit(float speed_set)
{
  JgmtPack.powerlimitKp /= 100.0f;
}

/**
*@brief 底盘控制
*@todo  可以考虑静止陀螺时使用轮组单独控制使底盘平稳
*/
void Chassis::Chassis_ctrl(void)
{
//----底盘速度观测
	Chassis_speedView();
//----底盘云台相对角度获取
	yaw_angle = getGimbalAngle();
	yaw_angle_deathroom = my_deathzoom(yaw_angle - 0.8,0.8f);
//	yaw_angle = getGimbalAngle();
//----底盘模式获取
	Chassis_mode();
//----控制值获取
	//平移速度获取单位：dm/s
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
		
//		Speed_set[_X] = myRC.mySpdSetX;   //平移设定值单位是dm/s
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
	//旋转速度获取
	if(chassis_mode == FOLLOW)  //陀螺设定值是角度，在后面转化为弧度
	{
  //由陀螺模式进入跟随模式时判断跟随方向，避免反跟随
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
		Speed_set[_R] = Followpid.Calculate(0) - RC.Key.CH[0] / 3.0f;  //前馈仅使用云台设定值
	}
	else if(chassis_mode == NO_FOLLOW)
	{
		Speed_set[_R] = 0;
	}
	else if(chassis_mode == GYRO) //陀螺模式旋转速度随实际速度增大而减小
	{
    //todo  缓启
		Speed_set[_R] = RC.Key.CH[11] * 0.8f + RC.Key.E * 500.0f;  
		Speed_set[_R] *= Turnspd_limit;
	}
	last_chassis_mode = chassis_mode;
//----设定值正方向转化
	float sin_x = my_sin(yaw_angle * RAD_PER_DEG);
	float cos_x = my_cos(yaw_angle * RAD_PER_DEG);
	Speed_real_set[_X] = Speed_set[_X] * cos_x - Speed_set[_Y] * sin_x;
	Speed_real_set[_Y] = Speed_set[_X] * sin_x + Speed_set[_Y] * cos_x;
	Speed_real_set[_R] = Speed_set[_R] / 180.0f * 3.14159f;
  //低通滤波
	Speed_real_set[_X] = BF_Lp.Apply(Speed_real_set[_X]);
	Speed_real_set[_Y] = LR_Lp.Apply(Speed_real_set[_Y]);
	Speed_real_set[_R] = turn_Lp.Apply(Speed_real_set[_R]);
//速度限制
//	Speed_Limit();
//----设定值到轮组速度的映射
	for(int i = 0; i < 4; i++)
	{
		WheelSpd_Set_BF[i]  = Speed_real_set[_X] * drc[_X][i] * amp[i];
	  WheelSpd_Set_LR[i]  = Speed_real_set[_Y] * drc[_Y][i] * amp[i];
	  WheelSpd_Set_turn[i]= Speed_real_set[_R] * drc[_R][i] * amp[i];
	}
//----电机控制
	Motor_ctrl();
}

/**
*@brief 电机控制函数
*@note 在什么地方进行功率限制（没有功率时设定值与电流限制通同时进行）
*/
void Chassis::Motor_ctrl(void)
{
	float Current_set[4] = {0};  //记录每个轮子电流设定值
	float max_current = 0;  //记录最大电流设定值
	Current_set_BF   =   BFSpdpid.Calculate(Speed_real_set[_X]);
	Current_set_LR   =   LRSpdpid.Calculate(Speed_real_set[_Y]);
	Current_set_turn = turnSpdpid.Calculate(Speed_real_set[_R]);
	
	double drc_t[3][4] = {{-1,1,1,-1},{-1,-1,1,1},{1,1,1,1}} ;  //初始化方向倍率参数
//根据输入总电流值限制	
	//选取最大电流设定值
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
	//电流设定值大于阈值进行限制
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
//根据打滑进行限制
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
//根据功率电容限制输出	
	power_limit = JgmtPack.powerlimitKp / 100.0f;
	for(int i = 0; i < 4; i++)
	{
		Current_set_fin[i] = Current_set_BF * drc_t[_X][i] + Current_set_LR * drc_t[_Y][i] + Current_set_turn * drc_t[_R][i];
		Wheel[i]->ctrlCurrent(Current_set_fin[i] * power_limit);
	}
}



/**
*@brief 底盘对象初始化
*@note 底盘pid及跟随pid初始化
*/
Chassis::Chassis()
{
	speedUpjudge = new doubleKickJudge;   //构造双击检测对象
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
	WS_Q = 0.1; //过程噪声
	WS_R = 1.0; //测量噪声
}


void my_pid_init(void)
{
	FollowpidPr.kp = 10;  //15 30 0.2 2 2000
	FollowpidPr.ki = 0;
	FollowpidPr.kd = 0.5;
	FollowpidPr.integralErrorMax = 0; 
	FollowpidPr.resultMax = 400;   //结果值为陀螺速度（角度制）
	
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
	
	turnSpdpidPr.kp = 1500;  //此处控制为弧度制
	turnSpdpidPr.ki = 0;
	turnSpdpidPr.kd = 30;
	turnSpdpidPr.integralErrorMax = 0;
	turnSpdpidPr.resultMax = 8500;  //13000
	
	BF_Lp.SetCutoffFreq(1000, 50);
	LR_Lp.SetCutoffFreq(1000, 50);
	turn_Lp.SetCutoffFreq(1000, 30);
	TurnSpdLimit_Lp.SetCutoffFreq(1000, 20);  //根据实际平移速度限制陀螺速度系数
	Slip1_Lp.SetCutoffFreq(1000, 20);
	Slip2_Lp.SetCutoffFreq(1000, 20);
		
	Followpid.setPlanNum(1);
	Followpid.fbValuePtr[0] = &chassis.yaw_angle; //反馈值为云台与底盘角度差值
	Followpid.paramPtr = &FollowpidPr;
	
	BFSpdpid.setPlanNum(1); 
	BFSpdpid.fbValuePtr[0] = &chassis.Speed_View[0];  //前后移动
	BFSpdpid.paramPtr = &BFSpdpidPr;
	
	LRSpdpid.setPlanNum(1);
	LRSpdpid.fbValuePtr[0] = &chassis.Speed_View[1]; //左右移动
	LRSpdpid.paramPtr = &LRSpdpidPr;
	
	turnSpdpid.setPlanNum(1);
	turnSpdpid.fbValuePtr[0] = &chassis.Speed_View[2]; //陀螺
	turnSpdpid.paramPtr = &turnSpdpidPr;  
}







































