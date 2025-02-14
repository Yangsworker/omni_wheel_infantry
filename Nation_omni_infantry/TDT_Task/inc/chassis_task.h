
#ifndef _CHASSIS_
#define _CHASSIS_

#include "board.h"
#include "motor.h"
#include "imu.h"
#include "imu_task.h"
#include "ptz_task.h"
#include "fire_task.h"
#include "power.h"
#include "filter.h"

typedef enum Mode
	{
    ChassisFollow = 0,//底盘跟随   0
    ChassisNoFollow , //底盘不跟随 1
		Gyroscope ,       //小陀螺     2
    SpeedingGyroscope,//差速陀螺   3
		ChassisProtect    //离线保护
  }mode;

class Chassis 
{
	public:
		Chassis() ;
		Motor *Wheel[4] ;
	  doubleKickJudge *speedUpjudge;   //加速模式选择（双击检测）
	  float Multiplier = 1;
	  bool Lockchassis = 0;    //是否锁定底盘
	  bool slowRun = 0;        //是否减速行驶
	  u8 ifChassisNoFollow = 0;//是否启用底盘不跟随模式
	  bool ifChassisProtect = 0;
	  u8 lastSpeedLevel = 0;
		enum dir
		{
			_X = 0 ,
			_Y = 1 ,
			_R = 2
		} ;
		//底盘跟随
		bool ifChassisFollow = 0; 
		int angle_target = 0;
		float angle_now = 0;   
		float angle_poor = 0;               //角度差值
		float angle_poor_integralError = 0; //角度差的和
		float follow_spd = 0;
		void ChFollow(void);
		//摇摆
	  bool swingFlag = 0;      
		int swing_drc = 1;
		float angle = 0;
		float swing_speed = 0;
		float mult = 0;
		void angle_judge(void);
		//小陀螺
		bool rotateFlag = 0;      
		float Gyro_angle = 0;
		float Gyro_speed = 700;
		float Gyro_mult = 1;
		void Gyro(void);
		//自动躲避
		u8 allow_speeding = 0;   //是否允许差速陀螺标志位
		bool flexRotate = 0;
		float WorldAngle_For_chassis;	//底盘世界角 角度制
		float WorldAngle_For_hurt_chassis;	//受击打时底盘世界角 角度制
		float ExtendAngle = 20;	      //扩展角
		float Avoid_range[2] = {0,0};	//规避范围（高、低位）
		int Hurt_count;		//伤害累计值
		int HurtUpdate_count;		//伤害来源更新计数器
		
		float speedingmult = 1.0;     //差速陀螺倍率
		
		
		//随机差速陀螺
		u8 allow_random_gyro = 0;//是否允许随机陀螺标志位
		int provide_random_seeds = 0; //提供随机数的节点
		double random_cycle = 0;  //播种周期
		u8 ifnext_random = 1;     //是否下一个随机数
		float random_mult = 1.0;  //随机陀螺倍率
		float random_mult_set = 1.0;   //设定（随机）
		float random_mult_last;  //上次值
		float random_mult_error; //倍率误差值
		void randomspeed_gyro(void);
		
    
		void SpeedingGyro(void);
		//换向
	
		vec4f motorSpd ;            //四个轮子的速度
		vec4f lastmotorSpd ;        //四个轮子上一次速度
		float chassisSpd[3] ;          //三个轴方向
		
		void chassis_run(void);
		
		float getGimbalAngle(void);    //获得云台角度
		
    void ChNoFollow(void);         //底盘不跟随
		
		bool judgeIfMoving(void);
		
    void powerCtrlUpdate(void);    //功率控制刷新
		void motorOutput();            //速度输出
};

void Chassis_Task(void) ;

extern Chassis chassis;
extern Lpf2p l1, l2, l3;
 
#endif



