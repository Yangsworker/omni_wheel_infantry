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

enum MotorID_t   //��̨�������Y�� P�� ̧���� 
{
	YAW, ROLL
};

enum VisionColor_t  //������ɫ������Ӿ��ֶ��޸ģ�
{
	RED = 0, BLUE = 1
};

enum CtrlPlan_t     
{
//���ɣ�������Ӿ�װ�ף��Ӿ�����
	Free, Buff, Fastbuff, Vision, VisionSniper
};  

enum VisionMode_t  //�Ӿ���������
{
	ARMOR=0,  //װ�װ�
	bigBUFF,  //���
	ANTIBUFF, //����
	rotate,   //�Ӿ�����
	smallBUFF,//С��
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
		float WorldDps[2];  //��̨������ٶȷ�����Yaw��Roll��
	  float SelfDps[2];   //��̨��е���ٶȣ�Yaw�� Roll��
	public :
		PTZ() ;
	  void Ptz_Task_Init(void);
	
		Motor *Yaw ;    //Yaw����
		Motor *Roll;    //ROLL����
	
	  doubleKickJudge *visionmodejudge;   //�Ӿ�ģʽѡ��˫����⣩
	 	u8 VMjudge;                         //�ж��һ�ʶ��װ��/С����
	 //�������˲�������������ûѧ
			float yawProcessNiose_Q , yawMeasureNoise_R;
			float pitchProcessNiose_Q , pitchMeasureNoise_R;
	//��ͨ�˲�����������Ҳûѧ
			float yawFbLPF_smpFreq=1000, yawFbLPF_coFreq=42;
			float pitchFbLPF_smpFreq=1000, pitchFbLPF_coFreq=42;

	
	  float eulerAngleSet[2];  //��̨ŷ�����趨
	  float mechAlgSet[2];   //��̨��е���趨
	  float WorldAlg[2];  //��̨����Ƿ���     �������ǣ�
	  float SelfAlg[2]; //��̨��е�Ƿ���
	
	  float motorPos2[2] = {0};   //λ�û�λ�ü��㣨����
		float PTZPos_set[3];        //ң��ֵ����
	  float recordYawAngle = 0;   //��¼������yaw��ֵ
		float recordRollAngle = 0;  //��¼������roll��ֵ
	  bool  angle_change_allow = 0; //�Ƿ�����Ƕȸ�ֵ
	
	  void ModeJudge_PTZ();       //��̨ģʽѡ��
	  void fbValUpdate(void) ;    //��������ˢ��

		u8 direction_change_flag;
		void change_direction(void);  //��ͷ����
		void Modeswitch_run(void);    //��ģʽ�ٶȸ�ֵ����
		
		float YawErrorToVision = 0;
		float PitchErrorToVision = 0;
		u8 autofireFlag = 0;      //�Ƿ������Զ������־λ
		void getautofire(void);   //��ȡ�Ƿ�����Զ�����
		
};



void Ptz_task(void);

extern PTZ ptz;
extern VisionMode_t VM; //��̨��������ģʽѡ��״̬��������Ӿ�����Ϣ
extern CtrlPlan_t CtrlPlan;
extern u8 ptz_error[4];

#endif
