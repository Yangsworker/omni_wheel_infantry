#include "fire_task.h"
#include "dbus.h"
#include "pid.h"
#include "ptz_task.h"

#define FIRE_SPEED_SLOW 4300  //15mps
#define FIRE_SPEED_MID  4650  //18mps
#define FIRE_SPEED_FAST 7450  //30mps
#define DIAL_ONE_POSITION 1620    //发射一个子弹拨盘移动位置360 * 36 / 8   1620
#define DIAL_HALF_POSITION 810
//发射一个子弹所需时间
#define DIAL_ONE_TIME_FAST 20   //25/s         
#define DIAL_ONE_TIME_MID 25    //20/s  //25
#define DIAL_ONE_TIME_SLOW 50  //10/s
#define DIAL_ONE_TIME_SINGLE 500//1/s

#define current_limit 9600      //电流值限制
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
	if(deforceFlag)   //脱力记录拨盘位置
	{
    fire.Dialpos_change = 1;
	}
	fire.ModeJudge_Fire();
}
Fire::Fire(u16 fire1ID, u16 fire2ID, u16 DialID)  //（pid 电机初始化）
{
	Firemotor[0] = new Motor (M3508, CAN2, fire1ID);
  Firemotor[1] = new Motor (M3508, CAN2, fire2ID);
	Dial    = new Motor (M2006, CAN2, DialID);
	//pid参数
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
//发弹速度选择
int heat_now = 0;        //进行裁判系统/本地热量选择
u8 Heat_limit_mode = 0;  //热量过高进入热量限制模式
int rotate_fire_clock = 0, buff_fire_clock = 0;
void Fire::fireSpeedSwitch()
{ 
	if (can1Feedback.MaxHeat == -1 )  //如果热量上限为-1，或者接除热量限制 -> 高频
	{
		DialFireTime = DIAL_ONE_TIME_MID;
		return;
	}

	switch(Fire_Mode)
	{
		case 0:
			DialFireTime = DIAL_ONE_TIME_SLOW;  //默认10弹频
		  break;
		case 1:
			DialFireTime = DIAL_ONE_TIME_SLOW;  
			break;
		case 2:
			DialFireTime = DIAL_ONE_TIME_MID;  //20弹频
			break;
		default:
			DialFireTime = DIAL_ONE_TIME_SLOW;  //默认10弹频
			break;
	}
	if(fire.unLimitedFired)  //解除热量限制仍可以切换发射模式
	{
		return;
	}

//	if(!can1Feedback.jgmtOffline)
//		heat_now = jgmtHeat;
//	else
		heat_now = sprocketHeat;
	
  LastDialFireTime = DialFireTime;
	float Fire_num = 0;
	if(Robot_Fire_Mode == 1)//--------------------------------------爆发优先------------------------------------------------------------
	{
	Fire_num = fire.jgmtCoolDown / 10;
	if(DialFireTime == DIAL_ONE_TIME_MID)  //20弹频
	{
		if (heat_now >= jgmtHeatMax - 60) //裁判系统热量超限自动降速
	  {
		  Heat_limit_mode = 1;    //单发限制
			buff_fire_clock = 1;    //打符开火限制
	  }
	  if(Heat_limit_mode)
	  {
      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//可一直发弹的最高弹速
		  if(heat_now < (jgmtHeatMax - 71))  
	     {DialFireTime = LastDialFireTime; 
	      Heat_limit_mode = 0;}
	  }
	}
	else
	{
		if (heat_now >= jgmtHeatMax - 37) //单发/10弹频
	  {
		  Heat_limit_mode = 1;    //单发限制
			buff_fire_clock = 1;
	  }
	  if(Heat_limit_mode)
	  {
      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//可一直发弹的最高弹速
		  if(heat_now < (jgmtHeatMax - 47))  
	     {DialFireTime = LastDialFireTime; 
	      Heat_limit_mode = 0;}
	  }
  }
  }
	if(Robot_Fire_Mode == 2)//-------------------------------------冷却优先-------------------------------------------
	{
	Fire_num = fire.jgmtCoolDown / 10;
	if(DialFireTime == DIAL_ONE_TIME_MID)  //20弹频
	{
		if (heat_now >= jgmtHeatMax - 40) //裁判系统热量超限自动降速
	  {
		  Heat_limit_mode = 1;    //单发限制
			buff_fire_clock = 1;    //打符开火限制
	  }
	  if(Heat_limit_mode)
	  {
      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//可一直发弹的最高弹速
		  if(heat_now < (jgmtHeatMax - 50))  
	     {DialFireTime = LastDialFireTime; 
	      Heat_limit_mode = 0;}
	  }
	}
	else
	{
		if (heat_now >= jgmtHeatMax - 30) //单发/10弹频
	  {
		  Heat_limit_mode = 1;    //单发限制
			buff_fire_clock = 1;
	  }
	  if(Heat_limit_mode)
	  {
      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//可一直发弹的最高弹速
		  if(heat_now < (jgmtHeatMax - 40))  
	     {DialFireTime = LastDialFireTime; 
	      Heat_limit_mode = 0;}
	  }
  }
  }
  if(Robot_Fire_Mode == 3 || !Robot_Fire_Mode)//---------------------------------------弹速优先---------------------------------------------
	{
	Fire_num = fire.jgmtCoolDown / 10;
		if(Fire_num >= 3)
		{Fire_num = 3;}
	if(DialFireTime == DIAL_ONE_TIME_MID)  //20弹频
	{
		if (heat_now >= jgmtHeatMax - 27) //裁判系统热量超限自动降速
	  {
		  Heat_limit_mode = 1;    //单发限制
			buff_fire_clock = 1;    //打符开火限制
	  }
	  if(Heat_limit_mode)
	  {
      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//可一直发弹的最高弹速
		  if(heat_now < (jgmtHeatMax - 37))  
	     {DialFireTime = LastDialFireTime; 
	      Heat_limit_mode = 0;}
	  }
	}
	else
	{
		if (heat_now >= jgmtHeatMax - 23) //单发/10弹频
	  {
		  Heat_limit_mode = 1;    //单发限制
			buff_fire_clock = 1;
	  }
	  if(Heat_limit_mode)
	  {
      DialFireTime = DIAL_ONE_TIME_SINGLE / Fire_num  ;//可一直发弹的最高弹速
		  if(heat_now < (jgmtHeatMax - 33))  
	     {DialFireTime = LastDialFireTime; 
	      Heat_limit_mode = 0;}
	  }
  }
  }	

}

//摩擦轮控制(读取弹速限制并控制)  
int friction_run = 0;
void Fire::friction_ctrl()     
{
	if(RC.Key.SW2 == RCS::Up)
	{
		//速度改变允许摩擦轮换速
		if(jgmtBulletSpeedMax != last_SpeedMax)
		{friction_run = 0;}
		if(!friction_run)  //读取裁判系统弹速限制控制弹速 并向视觉发送消息
	    {if(jgmtBulletSpeedMax == 30) {vision_SendStruct.nominalBulletSpeed = 30; friction_spd1 = FIRE_SPEED_FAST; friction_spd2 = FIRE_SPEED_FAST + 19;}
	     else if(jgmtBulletSpeedMax == 18) {vision_SendStruct.nominalBulletSpeed = 18; friction_spd1 = FIRE_SPEED_MID; friction_spd2 = FIRE_SPEED_MID + 13;}
	     else if(jgmtBulletSpeedMax == 15) {vision_SendStruct.nominalBulletSpeed = 15; friction_spd1 = FIRE_SPEED_SLOW; friction_spd2 = FIRE_SPEED_SLOW + 12;}
			 else                       {friction_spd1 = FIRE_SPEED_SLOW + 15;  friction_spd2 = FIRE_SPEED_SLOW;}
	     friction_run = 1;}
			 last_SpeedMax = jgmtBulletSpeedMax; //记录上次摩擦轮速度限制
//因该函数对弹速影响较大会影响视觉故应后续优化
		   frictionSpdLimit();  //速度限制
		   allowshoot = 1;  //允许射击
			 allowshoot_vision = 1;	
	}
	if(RC.Key.SW2 == RCS::Mid)
	{
		friction_spd1 = 0; friction_spd2 = 0; 
		friction_run = 0;
		allowshoot = 0;  //不允许射击 
		allowshoot_vision = 0;
	}
   
		Firemotor[0]->ctrlSpeed(friction_spd1);
	  Firemotor[1]->ctrlSpeed(-friction_spd2);
	
}
//拨盘发射程序（前提摩擦轮转）
uint8_t Change_CD;   //单发/连发转化冷却
int singlefireback_time = 0;//单发堵转时间
bool allow_back = 1; //是否允许回退
int autofire_clock = 0; //自动发射计时
u8 islastfired = 0;   //上次发射记录（电控自动）
int last_fire_num = 0, jgmt_fire_num = 0;//连发自动校准发弹量检测
u8 isDoubleshot = 0; //是否二连发
int fire_num_real = 0;
void Fire::fireload()          
{
	 
	J_Dial = Dialpos_set;
	R_Dial = Dialpos_record;
	D_current = Dial->canInfo.trueCurrent;
	J_fireSpd1 = Firemotor[0]->canInfo.speed;
	J_fireSpd2 = Firemotor[1]->canInfo.speed;
	
	jgmtHeatCalc();     //裁判系统热量获取
	sprocketHeatCalc(); //本地热量获取
	fireSpeedSwitch();  //拨轮转速选择
	current_now = ABS(Dial->canInfo.trueCurrent);       //记录拨盘电流值
	Dialpos_record = Dial->canInfo.totalAngle_f;        //记录拨盘位置
	Dialencouder_record = Dial->canInfo.encoder;        //记录拨盘encouder值

	if(Dialpos_change) {Dialpos_set = Dialpos_record; Dialpos_initial = Dialpos_record; Dialpos_change = 0;} //上力计算拨盘初始位置

	if(Change_CD ) {Change_CD++;}
	if(Change_CD > 100) {Change_CD = 0;}
	if(!Change_CD && RC.Key.F) 
  {
		Fire_Mode++;
		if(Fire_Mode > 2)
		{Fire_Mode = 0;}
	  Change_CD = 1;
	}  
//	fire.unLimitedFired = RC.Key.C;                     //解除热量限制

//视觉开火模式（开符不允许操作手开火，陀螺自动开火模式也允许操作手开火）
	if(RC.Key.SW1 == RCS::Up && CtrlPlan == Vision)
	{
	//开符自动开火（读取一次允许开火发射一次,若一直允许发射则5发/s）
	 if(VM == bigBUFF || VM == smallBUFF || VM == ANTIBUFF)
	 {
		 if(VM == ANTIBUFF) {allowshoot = 1;}
		 else         	    {allowshoot = 0;} //禁止操作手开火
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
////电控自动开火控制
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

	 //陀螺自动开火（发射频率可调）
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
//手动连发模式
	if(RC.Key.CH[8] && allowshoot && Fire_Mode && DialFireTime)  //DialFireTime-禁止发射时不允许发射
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
	if( RC.Key.CH[8] == 0 && RC.Key.SW1 != RCS::Down)                         //不左击，初始化值
	{
		last_key = 2 ;
	}
//堵转检测(单发检测电流，连发检测位置)
//	if(!Fire_Mode)  //单发/连发堵转检测
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
////防超热量	
	  if ( (ABS(Dialpos_set - Dialpos_record) > DIAL_ONE_POSITION * 3) )    //连发堵转检测/单发也可以触发           
			{         
//       allowshoot = 0;  //堵转不允许正向发射
  			Dialpos_set = Dialpos_set - DIAL_ONE_POSITION * 2;
//				allow_back = 0; 
			}
//		}	
	if(!allow_back)
	{if(current_now < current_limit - 3000) {allowshoot = 1; allow_back = 1;}} 
//二连发检测
	if(!Fire_Mode)
	{
		jgmt_fire_num = fire_num_real - last_fire_num;
		if(jgmt_fire_num>1)
		{
			isDoubleshot++;
		}
	}
//拨盘（本地）发弹量
	thisShootNum.sprocketShootNum = (Dialpos_record-Dialpos_initial)/DIAL_ONE_POSITION;
//拨盘控制	
	Dial->ctrlPosition(Dialpos_set) ;  
}
//各参数传递 （最大热量/弹速 系统冷却 裁判系统弹速）
void Fire::getFireLimit()
{
	if (!can1Feedback.jgmtOffline)//裁判系统未掉线
	{
		fire.jgmtCoolDown = can1Feedback.CoolRate;                        //裁判系统冷却
		fire.jgmtBulletSpeedMax = can1Feedback.shooterId1_17mmSpeedLimit; //裁判系统最大弹速
		fire.jgmtHeatMax = can1Feedback.MaxHeat;                          //裁判系统最大热量
	}
	else //若裁判系统离线，按弹速一级进行
	{
		fire.jgmtCoolDown = 15;
		fire.jgmtBulletSpeedMax = 30;
		fire.jgmtHeatMax = 75;
	}
//发弹模式选择判断
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
	fire.jgmtBulletSpeed = can1Feedback.Jgmt_OutSpd;  //裁判系统弹速传递
	vision_SendStruct.realBulletSpeed = fire.jgmtBulletSpeed;//限制发送弹速
}
//裁判系统热量计算
void Fire::jgmtHeatCalc()
{
	if (!can1Feedback.jgmtOffline)
		jgmtHeat = can1Feedback.Jgmt_Heat;//读取裁判系统热量
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

//	if(sprocketHeat > 0)
	sprocketHeat -= fire.jgmtCoolDown * sprocketHeatTimer.getCycleT();  //热量随时间自减
	sprocketHeat = LIMIT(sprocketHeat, 0, jgmtHeatMax + 20);
}
//摩擦轮速度限制/同时记录发弹量 
void Fire::frictionSpdLimit()
{
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
  if(!can1Feedback.jgmtOffline)	//弹速调整须在裁判系统未离线情况下
	{
	 if(jgmtBulletSpeed > (jgmtBulletSpeedMax - 0.8))//速度超限自动降速
	 {
		friction_spd1 -= 60; 
		 friction_spd2 -= 60; 
	 }
	 else if (jgmtBulletSpeed < jgmtBulletSpeedMax - 5 && jgmtBulletSpeedMax<32 ) //速度小于最高弹速提高弹速
	 { 
			friction_spd1 += 100; 
		 friction_spd2 += 100; 
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





