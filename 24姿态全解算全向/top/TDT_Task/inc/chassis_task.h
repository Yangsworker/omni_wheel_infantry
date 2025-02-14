#ifndef __CHASSIS_TASK_H__
#define __CHASSIS_TASK_H__
#include "board.h"
#include "power.h"
#include "filter.h"
#include "motor.h"
#include "imu.h"
#include "KeyProcess.h"

#define YAW_OFFSET  285.1f   //yaw�����װƫ��
#define CHASSIS_RAD 0.208f //���̰뾶
#define SQRT_TWO    1.414f //����2

typedef enum Mode
	{
		DEFORCE = 0,
    FOLLOW ,     //���̸���   1
    NO_FOLLOW ,  //���̲����� 2
		GYRO ,       //С����    3
    SpeedingGyroscope,//��������   3
		ChassisProtect    //���߱���
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
	doubleKickJudge *speedUpjudge;   //����ģʽѡ��˫����⣩
		
	float amp[4] = {0.5,0.5,0.5,0.5};   //����
	int8_t drc[3][4] = {{-1,1,1,-1},{-1,-1,1,1},{1,1,1,1}} ;
		
	float angle_yaw = 0;     //��̨���ԽǶ�
	float yaw_angle_draw = 0;//����ui����ԽǶ�
	float yaw_angle = 0;     //��̨������Խ�
	float yaw_angle_deathroom = 0;
	float yaw_dps = 0;
	
/***********************�˶���̬�Ż���ز���************************************/	
	bool angle_state = 0; //��̨��������ָ��
	float Turnspd_limit = 1.0f; //�����ٶ�ϵ��
	bool gyro_dir = 1;    //�����ٶȷ����������ݽ���ʱ��̬����
	u8 turn_dir_offset = 0; //����ת���淽��У��0-�޴��� 1-��ת 2-��ת
		
	float Speed_set[3] = {0};      //�趨ֵ
	float Speed_real_set[3] = {0}; //�ٶ��趨ֵת��������
	float Speed_View[3] = {0};     //�۲�ֵ
	float Speed_View_Offset[3] = {0}; //�������ٶ�У׼��������ٶ�
	float WheelSpd_Set_BF[4] = {0};  //�����ٶ��趨
	float WheelSpd_Set_LR[4] = {0};
	float WheelSpd_Set_turn[4] = {0};
	float WheelSpd_View[4] = {0}; //�����ٶȹ۲�
	
	float Current_set_BF = 0;
	float Current_set_LR = 0;
	float Current_set_turn = 0;
	
	bool ifSlip = 1;  //�Ƿ��
	float Slip_coeff1 = 0; //�򻬲���ϵ��
  float Slip_coeff2 = 0; 
	float power_limit = 0; //��������ϵ��
	float Current_set_fin[4] = {0};
	
    //���̸���
		bool ifChassisFollow = 0; 
		int angle_target = 0;
		float angle_now = 0;   
		float angle_poor = 0;               //�ǶȲ�ֵ
		float angle_poor_integralError = 0; //�ǶȲ�ĺ�
		float follow_spd = 0;
		void ChFollow(void);
		//С����
		bool rotateFlag = 0;      
		float Gyro_angle = 0;
		float Gyro_speed = 700;
		float Gyro_mult = 1;
		void Gyro(void);
		//�������˲�����
		float WS_Q, WS_R;
		
		
		float getGimbalAngle(void);    //�����̨�Ƕ�		
		void Chassis_mode(void);       //����ģʽѡ��
		void Chassis_speedView(void);  //�����ٶȹ۲�
    void Chassis_ctrl(void);       //���̿���
		void Speed_Limit(float speed_set);        //�ٶ�����
		void Motor_ctrl(void);         //�������
    void Slip_Check(void);         //�򻬼��

		void chassis_run(void);
		

		
    void ChNoFollow(void);         //���̲�����
		
		void motorOutput();            //�ٶ����
};
void my_pid_init(void);
extern Chassis chassis;
extern Lpf2p l1, l2, l3;

#endif







