#ifndef __GIMBAL_TASK_H__
#define __GIMBAL_TASK_H__
#include "board.h"
#include "motor.h"
#include "filter.h"
#include "curve_model.h"
#include "motor_MT_DLC.h"

#define PIDplanNum 4

enum MotorID_t{
	YAW,PITCH
};//云台电机的ID

enum CtrlPlan_t{
	Manual,	//操作手模式
	V_static,	//视觉跟随模式
	V_rotating,	//视觉打符模式
	V_buff	//机械角模式【暂未使用】
};//控制模式

class Gimbal
{
private:
	Kf pitchDps;
	Kf yawDps;
	Kf pitchAlg;
	Kf yawAlg;

	float yawProcessNiose_Q; //Y过程噪声
	float yawMeasureNoise_R; //Y测量噪声
	float pitchProcessNiose_Q; //P过程噪声
	float pitchMeasureNoise_R; //P测量噪声
	float yawDpsProcessNoise_Q; //Y过程噪声
	float yawDpsMeasureNoise_R; //Y测量噪声
	float pitchDpsProcessNoise_Q; //P过程噪声
	float pitchDpsMeasureNoise_R; //P测量噪声
	bool kfSwitch; //使用卡尔曼滤波

	FivePower lookback; //五次三项式 掉头
	uint8_t lookbackOK = 1;
	float lookbackBase;

	float limitEulerPitch[2];//动态欧拉角限位
	static const float mechPitchLimit[2];//静态机械角限位
	float pitchCurrentOutput;

	uint8_t lastLookBack;
	float WorldDps[2];//云台世界角速度反馈{Yaw,Pitch}
	float SelfDps [2];//云台机械角速度{Yaw,Pitch}
	
	
public:
	Gimbal(uint16_t YawID, uint16_t PitchID);
	Motor *Yaw;		//yaw轴电机
	Motor *Pitch;	//pitch轴电机

	float eulerAlgSet[2];//云台欧拉角设定{Yaw,Pitch}
	float mechAlgSet [2];//云台机械角设定{Yaw,Pitch}
	float WorldAlg[2];//云台世界角反馈{Yaw,Pitch}
	float SelfAlg [2];//云台机械角反馈{Yaw,Pitch}
  float anglePitch=0;
  float pitchspeed=0;

  void fbValUpdate(); //馈值获取
  void remoteCtrlUpdate(); //控制值获取
  void motorCtrl(void); //电机控制

	enum CtrlPlan_t CtrlPlan;
	enum CtrlPlan_t lastCtrlPlan;

  bool lastDeforce; 
};
	void gimbal_task(void);
//mt电机反馈值获取函数
void MT_getFabervalue(void);

extern Motor *MT_motor;

extern Gimbal gimbal;

#endif