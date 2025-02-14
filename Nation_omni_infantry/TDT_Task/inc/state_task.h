#ifndef __STATE_TASK_H
#define __STATE_TASK_H

#include "board.h"

/***visionDebugType����***/
ENUM_START(VisionDebugType)
{
	VDT_ARMOR=0,
	VDT_bigBUFF,
	VDT_ANTIBUFF,
	VDT_rotate,
	VDT_smallBUFF,
};
//ENUM_END(VisionDebugType)

//USE_OPERATOR_AUTO_INC(VisionDebugType);
//USE_OPERATOR_AUTO_DEC(VisionDebugType);

class StateCtrl  //״̬����
{
public:
	u8 lastDeforceFlag;
	VisionDebugType visionDebugType = VDT_ARMOR;
	u8 jgmtOfflineEnemyColor = 0;
	float autoGetAmmoFromTime;//��ʲôʱ��ʼ�Զ���λ
	float rightClickFromTimeFirst;//��ʲôʱ���һ�ΰ����Ҽ�
	float rightClickFromTimeNext;//��ʲôʱ���һ�ΰ����Ҽ�
	void autoGetAmmo();
	vec3f customeSpeed = {600.0f,600.0f,300.0f};
	u8 levelChoose=0;
	u8 chassisPerformanceChoose=0;
	u8 gimbalPerformanceChoose=0;
	u8 chassisPowerPerformance[3][3] = {{45,50,55}, {60,80,100},{60,80,100}};
	u16 chassisBloodPerformance[3][3] = {{200,300,400},{150,200,250},{300,400,500}};
	u8 shootSpeedPerformance[3][3] = {{15,15,15},{15,18,18},{30,30,30}};
	u8 shootCoolRatePerformance[3][3]={{15,25,35},{40,60,80},{15,25,35}};
	u16 shootMaxHeatPerformance[3][3]={{150,280,400},{50,100,150},{75,150,200}};
	bool getAmmoIng = 0;
	
	void changeVisionError();
	int8_t nowVisionError_Pitch,nowVisionErrorNext_Pitch;
	int8_t nowVisionError_Yaw,nowVisionErrorNext_Yaw;
	int8_t addYawFlag,addPitchFlag,addYawFlag_last,addPitchFlag_last;
	
	int8_t nowBuffError_Pitch,nowBuffErrorNext_Pitch;
	int8_t nowBuffError_Yaw,nowBuffErrorNext_Yaw;
	int8_t addYawBuffFlag,addPitchBuffFlag,addYawBuffFlag_last,addPitchBuffFlag_last;
};
extern StateCtrl stateCtrl;

#pragma pack(1) //ָ���ڴ���뷽ʽ
//0x141
struct CanStateStruct  //can״̬��
{
	uint64_t useCap:1;
	uint64_t checkMode:1;
	uint64_t ULTSMode:1;
	uint64_t powerDownLimit:7;
	uint64_t elevationAngle:7; //ǹ������
	uint64_t visionOffline:1;
	uint64_t frictionOffline:1;
	uint64_t fireFrequencyMode:2; //��Ƶ
	uint64_t unLimitedFired:1;
	uint64_t chassisMode:2;//0������1���ݣ�2ҡ�ڣ�3�����̣����䣩
	uint64_t ammoCoverOpen:1;
	uint64_t blockError:1;//������
	uint64_t visionLock:1;
	uint64_t visionBeat:1;
	uint64_t doubleShift:2;
	uint64_t chassisAngle:8;
	uint64_t localHeat:9;
	uint64_t sprocketMotorOffline:1;//����������
	uint64_t VisionMode:3;
	uint64_t flySloping:1;
	uint64_t flexRotate:1;
  uint64_t frictionState:2;
	uint64_t enemyID:3;
	uint64_t anti_arch:1;
  uint64_t uiinit:1;
};
//0x151  --���ظ���can��
struct CanStateStruct_2
{
	uint64_t e_vserrFlag:1;
	uint64_t e_vserrMode:2;
	int visionyawError:6;
	int visionpitchError:6;
	int BuffyawError:6;
	int BuffpitchError:6;
};



struct ROBOTCommunication
{
	uint16_t move; //1-ǰ�� 2-���� 3-�� 4-��
	uint16_t gyro:1; //0-������С���� 1-����С����
};

//�з�λ��
struct Enemyposition
{
	u32 object1_x:11;
	u32 object2_x:11;
	u32 object3_x:11;

	
	u32 object1_y:10;
	u32 object2_y:10;
	u32 object3_y:10;
  u32 no_Obj : 1;
};
#pragma pack()
#pragma pack(2)
struct Enemyradius   //�о��뾶����
{
  u32 object1_radius:10;
	u32 object2_radius:10;
	u32 object3_radius:10;
};
#pragma pack()

class FastSelfCheck
{
	public:
	void run();
	uint64_t systime;
	u8 selfCheckFlag = 0;
};
extern FastSelfCheck fastselfcheck;

extern struct Enemyposition enemyposition;
extern struct Enemyradius enemyradius;
extern struct CanStateStruct canStateStruct;
extern struct ROBOTCommunication RobotCommunication;
extern struct CanStateStruct_2 canStateStruct_2;

void State_Ctrl();

void State_Ctrl_RC_Info();

#endif

