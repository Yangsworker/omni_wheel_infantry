/*
*因陀螺仪不同/需修改：
*imu.cpp-148
*ptz_task.cpp-send_roll_offset
*ptz_task.cpp-200
*/
/*
*底盘跟随效果:换向时默认跟随负方向(即此时底盘保持不动)，按R可恢复跟随正方向
*/

#include "chassis_task.h"
#include "dbus.h"
#include "pid.h"
#include "my_math.h"
#include "can.h"
#include "smooth_start.h"
#include "stdlib.h"
#include "state_task.h"


#define random(x) (rand()%x) //产生随机数宏定义函数

 
Chassis chassis ;
PidParam motorpid, followpidOuter; //底盘跟随pid只用来计算外环 


Lpf2p l1, l2, l3;

void Chassis_Task (void)
{
	chassis.chassis_run();
	chassis.motorOutput() ;  //速度注入
}

float amp[4] = {7,7,7,7} ;
int8_t drc[3][4] = {{1,-1,-1,1},{1,1,-1,-1},{1,1,1,1}} ;//1顺1逆-1顺-1逆
//底盘初始化
Chassis::Chassis()
{
  speedUpjudge = new doubleKickJudge;   //构造双击检测对象
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
		Wheel[i]->pidOuter.fbValuePtr[0] = &angle_poor; //反馈值需实时刷新
		Wheel[i]->pidOuter.paramPtr = &followpidOuter;
	}
}

u8 ischanging = 0, ischanging2 = 0, ischanging3 = 0;//保证换向相关值仅变化一次（云台相对角、跟随角度）
float gimbalAngle = 0;
//获得云台与底盘正方向的偏置角（360制）（实际180-540）
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

/*控制键位/遥杆
*键位：左拨杆置中--默认底盘跟随
*      ifChassisNoFollow == 1（开符）--底盘不跟随
*      E--小陀螺  ctrl+E--差速陀螺（自动躲避）  E+R--随机陀螺
*      yaw轴电机离线--底盘不跟随
*摇杆：目前没有小陀螺全默认底盘跟随
*      
*/
bool Rear_Drive = 0; //后驱
float follow_error = 0, simulative_angle_now = 0; //跟随前馈参数（误差、虚拟云台角度）
float sssss = 0;
void Chassis::chassis_run(void)
{
	mode m;
	//刷新默认值
	Rear_Drive = 0;
	Lockchassis = 0;
	//键位选择
	if(RC.Key.CH[3] == -660 || canStateStruct_2.e_vserrFlag)//锁底盘--左拨杆锁下/进入调偏置模式
	{Lockchassis = 1;}
	if(RC.Key.SW1 == RCS::Mid || RC.Key.SW1 == RCS::Up || RC.Key.SW1 == RCS::Down)   
	{
		power.Check_Mode = 0;
	  	m = ChassisFollow;  //默认底盘跟随
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
	if(chassis.ifChassisNoFollow)     //视觉需要-底盘不跟随
	{
		m = ChassisNoFollow;
	}
	
	if(ptz.Yaw->canInfo.lostFlag == 1) //云台离线-离线保护 
	 {m = ChassisProtect;}
	 
	 
if(m != ChassisProtect)
{
	//前后左右移动
	Gyro_angle = getGimbalAngle() / 180.0f * 3.1415f;
	double sin_x = 0, sin_y = 0, cos_x = 0, cos_y = 0;
	sin_x = sin(Gyro_angle + 3.1415f/4);
	sin_y = sin(3.1415f/2 - Gyro_angle - 3.1415f/4);	
	cos_x = cos(Gyro_angle + 3.1415f/4);
	cos_y = cos(3.1415f/2 - Gyro_angle - 3.1415f/4);
	chassisSpd[_X] = RC.Key.CH[3] * 1.3 ;   //前后
	chassisSpd[_Y] = RC.Key.CH[2] * 1.3;   //左右
	
	chassisSpd[_X] += (RC.Key.W - RC.Key.S) * 420;   //键盘控制
  chassisSpd[_Y] += (RC.Key.D - RC.Key.A) * 420;
	
	chassisSpd[_X] = l1.Apply(chassisSpd[_X]);     //低通滤波平滑启动
	chassisSpd[_Y] = l2.Apply(chassisSpd[_Y]);
	
	motorSpd.data[0] = chassisSpd[_X] * drc[_X][0] * cos_x * amp[0] * Multiplier;
 	motorSpd.data[0] += chassisSpd[_Y] * drc[_Y][0] * cos_y * amp[0] * Multiplier;
	
	motorSpd.data[1] = chassisSpd[_X] * sin_x * drc[_X][1] * amp[1] * Multiplier;                                                                     
 	motorSpd.data[1] += chassisSpd[_Y] * sin_y * drc[_Y][1] * amp[1] * Multiplier;
 
  motorSpd.data[2] = -motorSpd.data[0];
  motorSpd.data[3] = -motorSpd.data[1];
	//模式选择/R轴移动
//默认底盘跟随
if(m == ChassisFollow)
 {
	 //自动检测是否前轮空转后轮不转，是即打开后驱模式
	 Rear_Drive = 0;
	 if(ABS(Wheel[0]->pidInner.error) + ABS(Wheel[1]->pidInner.error) - ABS(Wheel[2]->pidInner.error) - ABS(Wheel[3]->pidInner.error) > 1000)
	 {Rear_Drive = 1;}
	 if(Rear_Drive) //功率分配后驱
	 {
		 motorSpd.data[0] *= 0.6;
		 motorSpd.data[1] *= 0.6;
		 motorSpd.data[2] *= 1.5;
		 motorSpd.data[3] *= 1.5;
	 }
	 
	 //修改标志位
	 rotateFlag = 0; flexRotate = 0; ifChassisFollow = 1;
	 //现在角度为实际角度，目标角度为正方向角度                                                             
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
  //换角度值	   
	angle_poor = angle_target - angle_now ; float dddddd = angle_poor;
	if(angle_poor < -180) {angle_poor = 360 + dddddd;}
  //计算pid外环反馈速度值
	for(u8 a = 0; a<4; a++)
	{
		Wheel[a]->pidOuter.Calculate(0); 
	}  
	//pid前馈计算（以设定值为目标计算）
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
	chassisSpd[_R] = follow_error * 8.2; //8.0   //此处使用云台跟随误差值计算底盘跟随前馈
	sssss = Wheel[0]->pidOuter.result;
//	//原本遥杆、鼠标前馈
//  chassisSpd[_R] = RC.Key.CH[0] * 4.0; 
//	chassisSpd[_R] += RC.Key.CH[6] * 8.8;  //鼠标控制
	for(u8 i = 0 ; i<4 ;i++) 
  {
		motorSpd.data[i] += (Wheel[i]->pidOuter.result * -2.5 + chassisSpd[_R] * -1.0) * drc[_R][i] ;  //-2.1， -1.0
  }
 }
//陀螺模式 
if(m == Gyroscope)
{
	rotateFlag = 1; flexRotate = 0; ifChassisFollow = 0;
	if(allow_speeding)  {SpeedingGyro();} //差速陀螺
	else if(allow_random_gyro) {randomspeed_gyro();} //随机陀螺
	else {speedingmult = 1.0, random_mult = 1.0;}
	
	for(int i = 0; i < 4; i++)
 {
	 motorSpd.data[i] += Gyro_speed * drc[_R][i] * 4 * Gyro_mult * speedingmult * random_mult;
 }
}
//底盘不跟随模式/可以小陀螺
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

//离线保护
if(m == ChassisProtect)
{
	ChNoFollow();
}
}


//底盘不跟随（可以用滚轮控制转向）
void Chassis::ChNoFollow(void)  
{
	chassisSpd[_X] = RC.Key.CH[3] ;   
	chassisSpd[_Y] = RC.Key.CH[2] ;   
	chassisSpd[_R] = RC.Key.CH[11] ; 
	
	chassisSpd[_X] += (RC.Key.W - RC.Key.S) * 350;  //键盘控制
  chassisSpd[_Y] += (RC.Key.D - RC.Key.A) * 350;	
	if(RC.Key.E)  //底盘不跟随时也可以陀螺（暂未尝试）
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

//差速陀螺
float cc = 0;
int ee = 0, dd = 10;
void Chassis::SpeedingGyro()
{
	
	rotateFlag = 0; flexRotate = 1; ifChassisFollow = 0; //模式标志位修改
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
//随机陀螺
int i = 0;
void Chassis::randomspeed_gyro(void)
{
	if(ifnext_random)
	{
	  provide_random_seeds++;             //累加
	  random_mult_last = random_mult_set; //上次的值为上次设定的值（初始值为1）
    srand(provide_random_seeds);        //播种随机数种子
    random_mult_set = 1 + random(8)/10.0f; //获取随机倍率（1~2）
	  random_mult_error = random_mult_set - random_mult_last; //误差等于设定减上次
		ifnext_random = 0;
	} 
	i++;
	if(i <= 750)
	{
		random_cycle += 0.12 * 3.1415 / 180; //每2ms增加0.12度，1.5秒增到90度
	  random_mult = random_mult_last + random_mult_error * sin(random_cycle);
	}
	if(i == 750) {ifnext_random = 1; random_cycle = 0, i = 0;} //循环结束开始下一个随机数，周期置零
	
}

//判断底盘是否移动，用速度和判断(用于自动关弹舱盖/检测时间1s)
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

//功率控制刷新
/*
*CTRL+SHIFT+A+D--开启ULTS模式 0.4s CD
*SW1->Down--检录模式
*/
int ULTS_CD = 0;
void Chassis::powerCtrlUpdate()
{
	power.flySloping = RC.Key.V;   //按V三级加速，松开取消
	power.speedUpLevel = power.flySloping?3:speedUpjudge->doubleKickVal(RC.Key.SHIFT);
	slowRun = (RC.Key.CTRL && !RC.Key.SHIFT && !RC.Key.Q);  //慢行

	//含：双击检测、检测是否离线、向功率模块发送消息、获得限幅系数
//  if(RC.Key.CTRL && RC.Key.SHIFT && RC.Key.A && RC.Key.D && !ULTS_CD)	
//  {power.ULTS_Mode = !power.ULTS_Mode; ULTS_CD = 1;}
//	if(ULTS_CD)  {ULTS_CD++;}
//	if(ULTS_CD > 200) {ULTS_CD = 0;}
	
	power.useCap = 1;             //使用电容/默认开启
	power.myPowerOfflineJudge();  //检测是否离线
	power.myPowerCanTx();         //2ms发一次
	power.getPowerKp();           //获得功率限幅系数
}
//速度输出
void Chassis::motorOutput()
{
	powerCtrlUpdate();   
	
	if(power.speedUpLevel == 1 ) {Multiplier = 1.6; Gyro_mult = 1.5;} //shift手动赋加速值(电容智能恢复)
	else if(power.speedUpLevel == 2 ) {Multiplier = 1.85; Gyro_mult = 1.6;}
	
	else if(power.speedUpLevel == 3 )
  {
		Multiplier = Smooth_1.smooth_cos(1.5, 2.7);  //余弦缓启
		Gyro_mult = 1.5;
  }
	
  else if(power.capacitance_percentage <= 60) { Multiplier = 0.8; Gyro_mult = 1;} //防超电容
	else if((power.capacitance_percentage <= 65) && (power.capacitance_percentage >60) ) {Multiplier = 1; Gyro_mult = 1;}
	else {Multiplier = 1.35; Gyro_mult = 1.25;}
	if((ABS(chassisSpd[_X]) + ABS(chassisSpd[_Y])) > 400) {Gyro_mult *= 0.8;}
	if(slowRun)  {Multiplier = 0.1;}
	
	lastSpeedLevel = power.speedUpLevel; //记录上次加速等级
	
  if (Lockchassis)    //是否锁底盘 
  { 
    memset(motorSpd.data, 0, sizeof(motorSpd.data));//利用库函数memset将0复制到参数allspeed所指向的字符串的所有字符
  }
//	motorSpd.data[0] = Smooth_1.smooth_xy(0, motorSpd.data[0]);
//  motorSpd.data[1] = Smooth_2.smooth_xy(0, motorSpd.data[1]);
//	motorSpd.data[2] = Smooth_3.smooth_xy(0, motorSpd.data[2]);
//	motorSpd.data[3] = Smooth_4.smooth_xy(0, motorSpd.data[3]);
    for (u8 i = 0; i < 4; i++)
    {
//			Wheel[i]->pidInner.Calculate(motorSpd.data[i]);		
//			Wheel[i]->setPowerOutLimit(&power.powerLimitKp); //pid计算方式
//			Wheel[i]->ctrlCurrent(Wheel[i]->pidInner.result);
			Wheel[i]->setPowerOutLimit(&power.powerLimitKp); //功率限制控制
			Wheel[i]->ctrlSpeed(motorSpd.data[i]);
    }
}

