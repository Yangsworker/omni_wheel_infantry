#ifndef _PTZ_
#define _PTZ_

#include "board.h"
#include "motor.h"
#include "chassis_task.h"
#include "fire_task.h"
#include "filter.h"
#include "power.h"


#define PIDplanNum 5
#define LIFT_OFFSET_TIME_MAX 3000

enum MotorID_t   //云台三电机（Y轴 P轴 抬升） 
{
	YAW, ROLL
};

enum VisionColor_t  //敌人颜色（最好视觉手动修改）
{
	RED = 0, BLUE = 1
};

enum CtrlPlan_t     
{
//自由，打符，视觉装甲，视觉陀螺
	Free, Buff, Fastbuff, Vision, VisionSniper
};  

enum VisionMode_t  //视觉各个功能
{
	ARMOR=0,  //装甲板
	bigBUFF,  //大符
	ANTIBUFF, //反符
	rotate,   //视觉陀螺
	smallBUFF,//小符
};

enum vision_error_t
{
	vision_yaw,
	vision_pitch,
	buff_yaw,
	buff_pitch
};

class PTZ
{
	private:
		float WorldDps[2];  //云台世界角速度反馈（Yaw，Roll）
	  float SelfDps[2];   //云台机械角速度（Yaw， Roll）
	public :
		PTZ() ;
	  void Ptz_Task_Init(void);
	
		Motor *Yaw ;    //Yaw轴电机
		Motor *Roll;    //ROLL轴电机
	
	  doubleKickJudge *visionmodejudge;   //视觉模式选择（双击检测）
	 	u8 VMjudge;                         //判断右击识别装甲/小陀螺
	 //卡尔曼滤波俩参数，鼠鼠没学
			float yawProcessNiose_Q , yawMeasureNoise_R;
			float pitchProcessNiose_Q , pitchMeasureNoise_R;
	//低通滤波参数，鼠鼠也没学
			float yawFbLPF_smpFreq=1000, yawFbLPF_coFreq=42;
			float pitchFbLPF_smpFreq=1000, pitchFbLPF_coFreq=42;

	
	  float eulerAngleSet[2];  //云台欧拉角设定
	  float mechAlgSet[2];   //云台机械角设定
	  float WorldAlg[2];  //云台世界角反馈     （陀螺仪）
	  float SelfAlg[2]; //云台机械角反馈
	
	  float motorPos2[2] = {0};   //位置环位置计算（主）
		float PTZPos_set[3];        //遥控值接收
	  float recordYawAngle = 0;   //记录陀螺仪yaw轴值
		float recordRollAngle = 0;  //记录陀螺仪roll轴值
	  bool  angle_change_allow = 0; //是否允许角度赋值
	
	  void ModeJudge_PTZ();       //云台模式选择
	  void fbValUpdate(void) ;    //反馈数据刷新

		u8 direction_change_flag;
		void change_direction(void);  //换头函数
		void Modeswitch_run(void);    //各模式速度赋值函数
		
		float YawErrorToVision = 0;
		float PitchErrorToVision = 0;
		u8 autofireFlag = 0;      //是否允许自动开火标志位
		void getautofire(void);   //获取是否可以自动开火
		
};



void Ptz_task(void);

extern PTZ ptz;
extern VisionMode_t VM; //云台函数处理模式选择，状态机负责给视觉发消息
extern CtrlPlan_t CtrlPlan;
extern u8 ptz_error[4];

#endif
