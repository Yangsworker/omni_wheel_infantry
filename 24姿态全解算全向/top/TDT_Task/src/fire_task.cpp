#include "fire_task.h"
#include "dbus.h"
#include "pid.h"
#include "Gimbal_task.h"
#include "com_task.h"
#include "myRC_task.h"

#define DIAL_ONE_POSITION 1310  //发射一个子弹拨盘移动位置360 * 36 / 8   1620  360*36*53/26/10 2641  //1294
#define DIAL_HALF_POSITION 650
//发射一个子弹所需时间
#define DIAL_ONE_TIME_FAST 20   //25/s         
#define DIAL_ONE_TIME_MID 25    //20/s  //25
#define DIAL_ONE_TIME_SLOW 50  //10/s
#define DIAL_ONE_TIME_GYRO 70
#define DIAL_ONE_TIME_SINGLE 500//1/s

#define current_limit 7000      //电流值限制

float fire_speed_fast =7400;  //高弹速  //7400 


int half_fire_time = 0;

    PidParam firepid,  
     DialInner, DialOuter;
	
 Fire fire(0x201, 0x202, 0x205); 
 
 float S_Dial = 0, R_Dial = 0, D_current = 0;
 float J_fireSpd1 = 0, J_fireSpd2 = 0;
 
 void Fire_Task(void)
{
	fire.Dialpos_record = fire.Dial->canInfo.totalAngle_f;
	if(deforceFlag)   //脱力记录拨盘位置
	{
    fire.Dialpos_change = 1;
	}
	fire.ModeJudge_Fire();
}

//发弹速度选择
int heat_now = 0;        //进行裁判系统/本地热量选择
u8 Heat_limit_mode = 0;  //热量过高进入热量限制模式
int rotate_fire_clock = 0, buff_fire_clock = 0;
void Fire::fireSpeedSwitch()
{ 

	switch(Fire_Mode)  //默认20弹频
	{
		case 1:
			DialFireTime = DIAL_ONE_TIME_SLOW;  //10弹频
			break;
		case 2:
			DialFireTime = DIAL_ONE_TIME_FAST;  //20弹频
			break;
		case 3:
			DialFireTime = DIAL_ONE_TIME_GYRO; //陀螺低弹频
		default:
			DialFireTime = DIAL_ONE_TIME_SLOW; 
			break;
	}
	if(fire.unLimitedFired)  //解除热量限制仍可以切换发射模式
	{
		return;
	}

  LastDialFireTime = DialFireTime;
	float Fire_num = 0;
 
	Fire_num = fire.jgmtCoolDown / 10;
		if(Fire_num >= 5)
		{Fire_num = 5;}
/********************************使用裁判系统判断热量********************************/   //记录剩余热量
	if(DialFireTime == DIAL_ONE_TIME_MID)  //20弹频    
	{
		if (jgmtHeat <=  68) //裁判系统剩余热量
	  {
		  Heat_limit_mode = 1;    //单发限制
			buff_fire_clock = 1;    //打符开火限制
	  }
	  if(Heat_limit_mode)
	  {
      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//可一直发弹的最高弹速
		  if(jgmtHeat >=  80)  
	     {DialFireTime = LastDialFireTime; 
	      Heat_limit_mode = 0;}
	  }
	}
	else
	{
		if (jgmtHeat <=  35) //裁判系统剩余热量
	  {
		  Heat_limit_mode = 1;    //单发限制
			buff_fire_clock = 1;    //打符开火限制
	  }
	  if(Heat_limit_mode)
	  {
      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//可一直发弹的最高弹速
		  if(jgmtHeat >=  50)  
	     {DialFireTime = LastDialFireTime; 
	      Heat_limit_mode = 0;}
	  }
  }
/********************************使用拨盘位置判断热量********************************/	//记录当前热量
//	heat_now = sprocketHeat;
//if(DialFireTime == DIAL_ONE_TIME_MID)  //20弹频
//	{
//		if (heat_now >= jgmtHeatMax - 50) //裁判系统热量超限自动降速
//	  {
//		  Heat_limit_mode = 1;    //单发限制
//			buff_fire_clock = 1;    //打符开火限制
//	  }
//	  if(Heat_limit_mode)
//	  {
//      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//可一直发弹的最高弹速
//		  if(heat_now < (jgmtHeatMax - 60))  
//	     {DialFireTime = LastDialFireTime; 
//	      Heat_limit_mode = 0;}
//	  }
//	}
//	else
//	{
//		if (heat_now >= jgmtHeatMax - 40) //单发/10弹频
//	  {
//		  Heat_limit_mode = 1;    //单发限制
//			buff_fire_clock = 1;
//	  }
//	  if(Heat_limit_mode)
//	  {
//      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//可一直发弹的最高弹速
//		  if(heat_now < (jgmtHeatMax - 50))  
//	     {DialFireTime = LastDialFireTime; 
//	      Heat_limit_mode = 0;}
//	  }
//  }
 

}

//摩擦轮控制(读取弹速限制并控制)  
float Dial_speed_set = 0, Dial_current_set = 0;
bool allow_offset = 0;
void Fire::friction_ctrl()     
{
	allow_offset = 1;
	if(RC.Key.SW2 == RCS::Up)
	{
     friction_spd1 = fire_speed_fast; friction_spd2 = fire_speed_fast;	
//因该函数对弹速影响较大会影响视觉故应后续优化
//		 frictionSpdLimit();  //速度限制
		 allowshoot = 1;  //允许射击
		allow_offset = 0;
	}
	if(RC.Key.SW2 == RCS::Mid || Firemotor[0]->canInfo.lostFlag || Firemotor[1]->canInfo.lostFlag)
		if(RC.Key.SW2 == RCS::Mid)
	{
		friction_spd1 = 0; friction_spd2 = 0; 
		allowshoot = 0;  //不允许射击 
		allowshoot_vision = 0;
	}
   
		Firemotor[0]->ctrlSpeed(friction_spd1);
	  Firemotor[1]->ctrlSpeed(-friction_spd2);
}
//拨盘发射程序（前提摩擦轮转）
int singlefireback_time = 0;//单发堵转时间
bool allow_back = 1; //是否允许回退
int last_fire_num = 0, jgmt_fire_num = 0;//连发自动校准发弹量检测
u8 isDoubleshot = 0; //是否二连发
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
	
	jgmtHeatCalc();     //裁判系统热量获取
	sprocketHeatCalc(); //本地热量获取
	fireSpeedSwitch();  //拨轮转速选择
	current_now = ABS(Dial->canInfo.trueCurrent);       //记录拨盘电流值
	Dialpos_record = Dial->canInfo.totalAngle_f;        //记录拨盘位置

	if(Dialpos_change) {Dialpos_set = Dialpos_record; Dialpos_initial = Dialpos_record; Dialpos_change = 0;} //上力计算拨盘初始位置



	 //陀螺自动开火（发射频率可调）
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
//手动连发模式
if((RC.Key.CH[8] || myRC.AutoFire) && allowshoot && DialFireTime && Fire_Mode)  //DialFireTime-禁止发射时不允许发射
	{
		if(!(FireTimes % DialFireTime))           
		{
			Dialpos_set += DIAL_ONE_POSITION ;
		  FireTimes = 0 ;
		}
		last_key = 1 ;
	}
 FireTimes++ ; if(FireTimes > 1000) {FireTimes = 1;}    //限制大小防溢出    
//手动单发模式 
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
	if(RC.Key.CH[8] == 0 && RC.Key.SW1 != RCS::Down && !myRC.AutoFire_buff && !myRC.AutoFire)                         //不左击，初始化值
	{
		last_key = 2 ;
	}
//堵转检测(检测电流/连发检测位置)
	 //电流堵转检测
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
  //位置堵转检测	
	  if ( (ABS(Dialpos_set - Dialpos_record) > DIAL_ONE_POSITION * 3) )    //连发堵转检测/单发也可以触发           
			{         
        allowshoot = 0;  //堵转不允许正向发射
  			Dialpos_set = Dialpos_record - DIAL_ONE_POSITION / 2.0;
				allow_back = 0; 
			}
    if(!allow_back)
	  {if(current_now < current_limit - 5000) {allowshoot = 1; allow_back = 1;}} 
//二连发检测
//	if(!Fire_Mode)
//	{
		jgmt_fire_num = fire_num_real - last_fire_num;
		if(jgmt_fire_num >1)
		{
			isDoubleshot++;
			last_fire_num = fire_num_real;
		}
//	}
//拨盘（本地）发弹量
	thisShootNum.sprocketShootNum = (Dialpos_record-Dialpos_initial)/DIAL_ONE_POSITION;
//拨盘控制	
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
 //各参数传递 （最大热量/弹速 系统冷却 裁判系统弹速）
void Fire::getFireLimit()
{
	if (!!JgmtPack.jgmtOfflineFlag)//裁判系统未掉线
	{
		fire.jgmtCoolDown = LocalPack.shooterCoolingRateA;                        //裁判系统冷却
		fire.jgmtBulletSpeedMax = 30; //裁判系统最大弹速
		fire.jgmtHeatMax = LocalPack.shooterCoolingLimitA;                      //裁判系统最大热量
	}
	else //若裁判系统离线，按弹速一级进行
	{
		fire.jgmtCoolDown = 30;
		fire.jgmtBulletSpeedMax = 30;
		fire.jgmtHeatMax = 200;
	}
}
 //裁判系统热量计算
void Fire::jgmtHeatCalc()
{
	if (!JgmtPack.jgmtOfflineFlag)
		jgmtHeat = JgmtPack.shooterHeatRemainA;//裁判系统剩余热量
	else
		jgmtHeat = jgmtHeatMax;//直接设定热量达到上限
}
 //拨轮（本地）热量计算
void Fire::sprocketHeatCalc()
{
	if (thisShootNum.sprocketShootNum != lastShootNum.sprocketShootNum)   //如果拨轮检测到发射，拨轮热量加10
	{
		sprocketHeat += 10 * (thisShootNum.sprocketShootNum - lastShootNum.sprocketShootNum);
	}
	lastShootNum.sprocketShootNum = thisShootNum.sprocketShootNum;

	sprocketHeat -= fire.jgmtCoolDown * sprocketHeatTimer.getCycleT();  //热量随时间自减
	sprocketHeat = LIMIT(sprocketHeat, 0, jgmtHeatMax + 20);
}
 //摩擦轮速度限制/同时记录发弹量 
void Fire::frictionSpdLimit()
{
	jgmtBulletSpeed = JgmtPack.bulletSpeed / 100.0f;
	if (jgmtBulletSpeed == lastJgmtBulletSpeed) 
		return;
	else
		fire_num_real++;
	if (lastJgmtBulletSpeed == 0)//掉帧补偿
	{
		lastJgmtBulletSpeed = jgmtBulletSpeed;
		return;
	}
	if(jgmtBulletSpeed < 10)//视为0mps
	return;
  if(!JgmtPack.jgmtOfflineFlag)	//弹速调整须在裁判系统未离线情况下
	{
	 if(jgmtBulletSpeed > (jgmtBulletSpeedMax - 1.0))//速度超限自动降速
	 {
		friction_spd1 -= 30; 
		 friction_spd2 -= 30; 
		 fire_speed_fast -= 30;
	 }
	 else if (jgmtBulletSpeed < jgmtBulletSpeedMax - 5 && jgmtBulletSpeedMax<32 ) //速度小于最高弹速提高弹速
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
	getFireLimit();   //所需参数传递
	friction_ctrl();  //摩擦轮控制
	fireload();       //拨盘控制
}  

/*
 * 构造函数
*/
Fire::Fire(u16 fire1ID, u16 fire2ID, u16 DialID)  //（pid 电机初始化）
{
	Firemotor[0] = new Motor (M3508, CAN2, fire1ID);
  Firemotor[1] = new Motor (M3508, CAN2, fire2ID);
	Dial    = new Motor (M3508, CAN1, DialID);
	//pid参数
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
	//pid方案数设定
	Dial   ->pidInner.setPlanNum(1);
	Dial   ->pidOuter.setPlanNum(1);
	//pid参数来源
	Dial  -> pidInner.paramPtr = &DialInner;
	Dial  -> pidOuter.paramPtr = &DialOuter;
	//pid反馈值来源
	Dial   ->pidInner.fbValuePtr[0] = &Dial ->canInfo.speed;
	Dial   ->pidOuter.fbValuePtr[0] = &Dial ->canInfo.totalAngle_f;
}
