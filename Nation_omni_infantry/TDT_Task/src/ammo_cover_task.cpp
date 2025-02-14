#include "ammo_cover_task.h"
#include "dbus.h"
#include "pid.h"
#include "my_math.h"


PidParam BulletInner, BulletOuter;
AmmoCover ammoCover;
const float position_rota = -2500;  //行程，表示最大位置

void Ammo_Task()
{
	if(deforceFlag || (RC.Key.B && RC.Key.CTRL)) {ammoCover.ifstartoffset = 1;} //如果脱力/按Ctrl+E，再上力便开始校准
	if(!deforceFlag) 
	{
		if(ammoCover.ifstartoffset )  
		{
			ammoCover.Ammo_offset(20, 5000, 8000); //0.4s
		}
		if(!ammoCover.offset_start)
		{
		ammoCover.Ammo_open(); 
		ammoCover.auto_close();
		}
	}
}
//创建电机，pid参数赋值 
AmmoCover::AmmoCover()
{
	Bullet  = new Motor (M2006, CAN2, 0x204);
	
	BulletInner.kp = 13;   //12 30 0 10 9000
	BulletInner.ki = 30;
	BulletInner.kd = 0;
	BulletInner.integralErrorMax = 0;
	BulletInner.resultMax = 9000;
	
	BulletOuter.kp = 11;
	BulletOuter.ki = 15;
	BulletOuter.kd = 0;
	BulletOuter.integralErrorMax = 1;
	BulletOuter.resultMax = 3000;
	
  Bullet ->pidInner.setPlanNum(1);
	Bullet ->pidOuter.setPlanNum(1);
	
	Bullet-> pidInner.paramPtr = &BulletInner;
	Bullet-> pidOuter.paramPtr = &BulletOuter;
	
	Bullet ->pidInner.fbValuePtr[0] = &Bullet ->canInfo.speed;
	Bullet ->pidOuter.fbValuePtr[0] = &Bullet ->canInfo.totalAngle_f;
}


//弹舱盖校准程序--校准最低位置 
bool AmmoCover::Ammo_offset(float reSetSpeed, float maxErr, float outLimit)
{
   switch(ammoCover.count)
		{
			case 0:
				//记录原始输出限幅值
				ammoCover.outLimitTemp = this -> Bullet -> pidInner.paramPtr ->resultMax;
			  //获取现在yaw云台反馈totalangle值
			  ammoCover.offsetPos = *(this -> Bullet -> pidOuter.fbValuePtr[0]);
				ammoCover.count++;
				break;
			case 1: 
				//更改pid输出电流限幅为设定值
			  this -> Bullet -> pidInner.paramPtr -> resultMax = outLimit;
			  //给定校准速度，开始校准
			  ammoCover.offset_start = 1;
			  ammoCover.offsetPos += reSetSpeed;
			  this -> Bullet -> ctrlPosition(ammoCover.offsetPos);
			  //堵转检测
			  if(ABS(Bullet -> pidOuter.error) > maxErr)   //可以在此加入比较次数
				{ ammoCover.count++; }
				break;
			case 2: 
				//获取电机机械位置,最低位置赋0
				int16_t offsetEncoderTemp = Bullet -> canInfo.encoder;
			  memset(&(Bullet -> canInfo), 0, sizeof(Bullet -> canInfo));
			  Bullet -> canInfo.offsetEncoder = offsetEncoderTemp;
			  //清零具有累加性质的pid变量
			  Bullet -> pidOuter.Clear();
			  Bullet -> ctrlPosition(0);
				ammoCover.count = 0;
			  //恢复pid输出限幅为正常模式
			  Bullet -> pidInner.paramPtr -> resultMax = ammoCover.outLimitTemp;
			  //电机校零成功更改值
			 	ammoCover.ifstartoffset = 0; //校准完成关闭校准接口
			  ammoCover.offset_start = 0;
			  open_close = 0; //校准后瞬间不允许开弹舱盖
				return 1; 
		}	
  return 0;

}




//弹舱开闭(0.6s CD)
int open_CD = 0;
float bullet_current_now = 0, bullet_position_now = 0, bullet_position_error = 0;
float fuck_time = 0, fuck_speed = 1500, fuck_times = 0;
bool mode_change = 0, if_fuck = 0;
void AmmoCover::Ammo_open(void)     
{
	bullet_current_now = ABS(Bullet->canInfo.trueCurrent);
	bullet_position_now = Bullet->canInfo.totalAngle_f;
	bullet_position_error = ABS(bullet_position_now - position_set);
	mode_change = 0;
//	if(!offset_start && !if_fuck) //校准或堵转时不允许开闭
	if(!if_fuck)
	{
	  if(RC.Key.B && !open_CD){ 
	    open_close = !open_close; 
			mode_change = 1;  //开闭时传值给标志位
			open_CD = 1;}
		if(open_CD) {
	    open_CD++;
		  if(open_CD > 300) {open_CD = 0;}
		}
		
		if(mode_change)
		{position_set = bullet_position_now;}
	  if(!open_close) //关弹舱
	  {
			if(position_set <= -150) 
		  {
		  	position_set += 20;
		  }
			else
			{
				position_set = -150;
			}
	  }
	  if(open_close)  //开弹舱
	  {
		  if(position_set >= position_rota)
		  {
			  position_set -= 20;
		  }
			else
			{
				position_set = position_rota;
			}
	  }
	  Bullet->ctrlPosition(position_set);
 }
//时刻检测是否堵转
 if(bullet_position_error > 800)
 {
	 fuck_times++;
	 if(fuck_times >= 1000)
	 {
		 fuck_times = 0;
	 if_fuck = 1;
	 }
 }
 else if(bullet_position_error < 800)
 {fuck_times = 0;}
 if(if_fuck)
 {
	 fuck_time++;
	 if(open_close) //开弹舱
	 {
		 Bullet->ctrlSpeed(fuck_speed);
	 }
	 else           //关弹舱
	 {
		 Bullet->ctrlSpeed(-fuck_speed);
	 }
	 if(fuck_time >= 100)
	 {
		 if_fuck = 0;
		 position_set = bullet_position_now;
		 fuck_time = 0;
	 }
 }
}

//判断弹舱盖是否打开(位置简易判断)
void AmmoCover::Coveropen_Judge()
{
	bullet_position_now = Bullet -> canInfo.totalAngle_f;
	if(bullet_position_now < -2300) {coverOpen = 1;}
	if(bullet_position_now > -500) {coverOpen = 0;}
}
void AmmoCover::auto_close(void)
{
	Coveropen_Judge();
	//弹舱盖开启，并且底盘移动--关闭弹舱
	if(chassis.judgeIfMoving() && coverOpen && !offset_start && !chassis.rotateFlag && !chassis.flexRotate) 
	{
		ifchassismove = 1;
		open_close = 0;
	}
}


	