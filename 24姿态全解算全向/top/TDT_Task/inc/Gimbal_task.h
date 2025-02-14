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
};//��̨�����ID

enum CtrlPlan_t{
	Manual,	//������ģʽ
	V_static,	//�Ӿ�����ģʽ
	V_rotating,	//�Ӿ����ģʽ
	V_buff	//��е��ģʽ����δʹ�á�
};//����ģʽ

class Gimbal
{
private:
	Kf pitchDps;
	Kf yawDps;
	Kf pitchAlg;
	Kf yawAlg;

	float yawProcessNiose_Q; //Y��������
	float yawMeasureNoise_R; //Y��������
	float pitchProcessNiose_Q; //P��������
	float pitchMeasureNoise_R; //P��������
	float yawDpsProcessNoise_Q; //Y��������
	float yawDpsMeasureNoise_R; //Y��������
	float pitchDpsProcessNoise_Q; //P��������
	float pitchDpsMeasureNoise_R; //P��������
	bool kfSwitch; //ʹ�ÿ������˲�

	FivePower lookback; //�������ʽ ��ͷ
	uint8_t lookbackOK = 1;
	float lookbackBase;

	float limitEulerPitch[2];//��̬ŷ������λ
	static const float mechPitchLimit[2];//��̬��е����λ
	float pitchCurrentOutput;

	uint8_t lastLookBack;
	float WorldDps[2];//��̨������ٶȷ���{Yaw,Pitch}
	float SelfDps [2];//��̨��е���ٶ�{Yaw,Pitch}
	
	
public:
	Gimbal(uint16_t YawID, uint16_t PitchID);
	Motor *Yaw;		//yaw����
	Motor *Pitch;	//pitch����

	float eulerAlgSet[2];//��̨ŷ�����趨{Yaw,Pitch}
	float mechAlgSet [2];//��̨��е���趨{Yaw,Pitch}
	float WorldAlg[2];//��̨����Ƿ���{Yaw,Pitch}
	float SelfAlg [2];//��̨��е�Ƿ���{Yaw,Pitch}
  float anglePitch=0;
  float pitchspeed=0;

  void fbValUpdate(); //��ֵ��ȡ
  void remoteCtrlUpdate(); //����ֵ��ȡ
  void motorCtrl(void); //�������

	enum CtrlPlan_t CtrlPlan;
	enum CtrlPlan_t lastCtrlPlan;

  bool lastDeforce; 
};
	void gimbal_task(void);
//mt�������ֵ��ȡ����
void MT_getFabervalue(void);

extern Motor *MT_motor;

extern Gimbal gimbal;

#endif