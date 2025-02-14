#ifndef __CHASSIS_TASK_H__
#define __CHASSIS_TASK_H__
#include "board.h"
#include "power.h"
#include "filter.h"
#include "motor.h"
#include "imu.h"
#include "KeyProcess.h"

#define YAW_OFFSET  285.1f   //yaw电机安装偏置
#define CHASSIS_RAD 0.208f //底盘半径
#define SQRT_TWO    1.414f //根号2

typedef enum Mode
	{
		DEFORCE = 0,
    FOLLOW ,     //底盘跟随   1
    NO_FOLLOW ,  //底盘不跟随 2
		GYRO ,       //小陀螺    3
    SpeedingGyroscope,//差速陀螺   3
		ChassisProtect    //离线保护
  }mode;


class Chassis
{
public:
	enum dir
		{
			_X = 0 ,
			_Y = 1 ,
			_R = 2
		} ;
	Chassis();
	Motor *Wheel[4] ;
	doubleKickJudge *speedUpjudge;   //加速模式选择（双击检测）
		
	float amp[4] = {0.5,0.5,0.5,0.5};   //倍率
	int8_t drc[3][4] = {{-1,1,1,-1},{-1,-1,1,1},{1,1,1,1}} ;
		
	float angle_yaw = 0;     //云台绝对角度
	float yaw_angle_draw = 0;//绘制ui用相对角度
	float yaw_angle = 0;     //云台底盘相对角
	float yaw_angle_deathroom = 0;
	float yaw_dps = 0;
	
/***********************运动姿态优化相关参数************************************/	
	bool angle_state = 0; //云台与机体相对指向
	float Turnspd_limit = 1.0f; //陀螺速度系数
	bool gyro_dir = 1;    //陀螺速度方向，用于陀螺结束时姿态纠正
	u8 turn_dir_offset = 0; //陀螺转跟随方向校正0-无处理 1-正转 2-反转
		
	float Speed_set[3] = {0};      //设定值
	float Speed_real_set[3] = {0}; //速度设定值转换正方向
	float Speed_View[3] = {0};     //观测值
	float Speed_View_Offset[3] = {0}; //经陀螺速度校准后的轮子速度
	float WheelSpd_Set_BF[4] = {0};  //轮子速度设定
	float WheelSpd_Set_LR[4] = {0};
	float WheelSpd_Set_turn[4] = {0};
	float WheelSpd_View[4] = {0}; //轮子速度观测
	
	float Current_set_BF = 0;
	float Current_set_LR = 0;
	float Current_set_turn = 0;
	
	bool ifSlip = 1;  //是否打滑
	float Slip_coeff1 = 0; //打滑补偿系数
  float Slip_coeff2 = 0; 
	float power_limit = 0; //功率限制系数
	float Current_set_fin[4] = {0};
	
    //底盘跟随
		bool ifChassisFollow = 0; 
		int angle_target = 0;
		float angle_now = 0;   
		float angle_poor = 0;               //角度差值
		float angle_poor_integralError = 0; //角度差的和
		float follow_spd = 0;
		void ChFollow(void);
		//小陀螺
		bool rotateFlag = 0;      
		float Gyro_angle = 0;
		float Gyro_speed = 700;
		float Gyro_mult = 1;
		void Gyro(void);
		//卡尔曼滤波参数
		float WS_Q, WS_R;
		
		
		float getGimbalAngle(void);    //获得云台角度		
		void Chassis_mode(void);       //底盘模式选择
		void Chassis_speedView(void);  //底盘速度观测
    void Chassis_ctrl(void);       //底盘控制
		void Speed_Limit(float speed_set);        //速度限制
		void Motor_ctrl(void);         //电机控制
    void Slip_Check(void);         //打滑检测

		void chassis_run(void);
		

		
    void ChNoFollow(void);         //底盘不跟随
		
		void motorOutput();            //速度输出
};
void my_pid_init(void);
extern Chassis chassis;
extern Lpf2p l1, l2, l3;

#endif







