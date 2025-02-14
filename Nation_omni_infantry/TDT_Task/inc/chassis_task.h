
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
    ChassisFollow = 0,//���̸���   0
    ChassisNoFollow , //���̲����� 1
		Gyroscope ,       //С����     2
    SpeedingGyroscope,//��������   3
		ChassisProtect    //���߱���
  }mode;

class Chassis 
{
	public:
		Chassis() ;
		Motor *Wheel[4] ;
	  doubleKickJudge *speedUpjudge;   //����ģʽѡ��˫����⣩
	  float Multiplier = 1;
	  bool Lockchassis = 0;    //�Ƿ���������
	  bool slowRun = 0;        //�Ƿ������ʻ
	  u8 ifChassisNoFollow = 0;//�Ƿ����õ��̲�����ģʽ
	  bool ifChassisProtect = 0;
	  u8 lastSpeedLevel = 0;
		enum dir
		{
			_X = 0 ,
			_Y = 1 ,
			_R = 2
		} ;
		//���̸���
		bool ifChassisFollow = 0; 
		int angle_target = 0;
		float angle_now = 0;   
		float angle_poor = 0;               //�ǶȲ�ֵ
		float angle_poor_integralError = 0; //�ǶȲ�ĺ�
		float follow_spd = 0;
		void ChFollow(void);
		//ҡ��
	  bool swingFlag = 0;      
		int swing_drc = 1;
		float angle = 0;
		float swing_speed = 0;
		float mult = 0;
		void angle_judge(void);
		//С����
		bool rotateFlag = 0;      
		float Gyro_angle = 0;
		float Gyro_speed = 700;
		float Gyro_mult = 1;
		void Gyro(void);
		//�Զ����
		u8 allow_speeding = 0;   //�Ƿ�����������ݱ�־λ
		bool flexRotate = 0;
		float WorldAngle_For_chassis;	//��������� �Ƕ���
		float WorldAngle_For_hurt_chassis;	//�ܻ���ʱ��������� �Ƕ���
		float ExtendAngle = 20;	      //��չ��
		float Avoid_range[2] = {0,0};	//��ܷ�Χ���ߡ���λ��
		int Hurt_count;		//�˺��ۼ�ֵ
		int HurtUpdate_count;		//�˺���Դ���¼�����
		
		float speedingmult = 1.0;     //�������ݱ���
		
		
		//�����������
		u8 allow_random_gyro = 0;//�Ƿ�����������ݱ�־λ
		int provide_random_seeds = 0; //�ṩ������Ľڵ�
		double random_cycle = 0;  //��������
		u8 ifnext_random = 1;     //�Ƿ���һ�������
		float random_mult = 1.0;  //������ݱ���
		float random_mult_set = 1.0;   //�趨�������
		float random_mult_last;  //�ϴ�ֵ
		float random_mult_error; //�������ֵ
		void randomspeed_gyro(void);
		
    
		void SpeedingGyro(void);
		//����
	
		vec4f motorSpd ;            //�ĸ����ӵ��ٶ�
		vec4f lastmotorSpd ;        //�ĸ�������һ���ٶ�
		float chassisSpd[3] ;          //�����᷽��
		
		void chassis_run(void);
		
		float getGimbalAngle(void);    //�����̨�Ƕ�
		
    void ChNoFollow(void);         //���̲�����
		
		bool judgeIfMoving(void);
		
    void powerCtrlUpdate(void);    //���ʿ���ˢ��
		void motorOutput();            //�ٶ����
};

void Chassis_Task(void) ;

extern Chassis chassis;
extern Lpf2p l1, l2, l3;
 
#endif



