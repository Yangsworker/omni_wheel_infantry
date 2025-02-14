#ifndef __CAN_H__ 
#define __CAN_H__ 
 
#include "board.h" 
extern int Mspeed; 
extern struct OrgPowerRecvMsg orgPowerRecvMsg; 
extern struct PowerRecvMsg powerRecvMsg; 
typedef struct _can1_feedback 
{ 
	int16_t FeedBackState;		//状态 
	float SuperPowerRemain_P;	//电容剩余容量百分百 
	u8 Light[6];				//1为绿，0为红 
	u8 Light_Toggle[6];			//闪烁(优先级高于长亮) 
	u8 Light_FastToggle[6];		//高频闪烁(优先级高于闪烁) 
	u8 ForeOpenLoop;			//强制开环 
	u8 FrictionWheelOffline;	//摩擦轮离线 
	int16_t SpdFeedBack_A;		//摩擦轮转速A 
	int16_t SpdFeedBack_B;		//摩擦轮转速B 
	u8 VisionShoot;				//视觉射击 
	u8 EmergencyClosePower;		//功率紧急切断 
	u8 chassisOffline[4]; 
	u8 chassisOfflineCheck[4];	 
	u16 gimbalPos[2]; 
	u8 gimbalTemperature[2]; 
	u8 chassisTemperature[4]; 
   }can1_feedback; //似乎没用
 
__packed struct CanStateStruct 
{ 
	uint64_t useCap:1; 
	uint64_t checkMode:1; 
	uint64_t ULTSMode:1; 
	uint64_t powerDownLimit:7; 
	//	uint64_t frictionSpdA:6; 待解注或者看看上面结构体的摩擦轮
//	uint64_t frictionSpdB:6;
//  uint64_t AvrgfrictionSpd:6;//平均摩擦轮速
	uint64_t elevationAngle:7;
	uint64_t visionOffline:1; 
	uint64_t frictionOffline:1; 
//	uint64_t forceOpenloop:1; 
	uint64_t fireFrequencyMode:2; //弹频
	uint64_t unLimitedFired:1; 
	uint64_t chassisMode:2;//0正常，1陀螺，2摇摆，3锁底盘（吊射） 
	uint64_t ammoCoverOpen:1; 
	uint64_t blockError:1; 
	uint64_t visionLock:1; 
	uint64_t visionBeat:1; 
	uint64_t doubleShift:2; 
	uint64_t chassisAngle:8; 
	uint64_t localHeat:9; 
//	uint64_t heatChoose:1;//0为摩擦轮，1为拨弹轮 
//  uint64_t commandguardFlag:1;
	uint64_t sprocketMotorOffline:1;//拨弹轮离线 
	uint64_t VisionMode:3; 
	uint64_t flySloping:1; 
	uint64_t flexRotate:1;
	uint64_t frictionSpeed_State:2;//
	uint64_t enemyID:3;
	uint64_t Anti_Building_Flag:1;



	uint8_t  UI_Init:1;
//	uint64_t AuxiliaryCarlibration:2;//0N1E2S3W
}; 

__packed struct CanStateStruct1
{
	uint64_t OffsetModifyFlag:1;
	uint64_t OffsetModifyMode:2;
	int YawOffsetError:6;
	int PitchOffsetError:6;
	int YawBuffOffset:6;
	int PitchBuffOffset:6;
};
__packed struct CommandToGuardStruct
{
	uint16_t move; //1-前进 2-后退 3-左 4-右 5-
	uint16_t gyro:1; //0-不控制小陀螺 1-控制小陀螺
};


__packed struct Enemyposition 
{ 
	u32 object1_x:11; 
	u32 object2_x:11; 
	u32 object3_x:11; 
	 
	u32 object1_y:10; 
	u32 object2_y:10; 
	u32 object3_y:10; 
//  u32 no_Obj : 1; 
}; 
struct PowerSendMsg 
{ 
	
	uint32_t aaaaa:10;
	uint32_t bbbbb:10;//无用数据
	uint32_t chassis_power:10; 
	uint16_t chassis_power_buffer; 
	uint16_t max_chassis_power; 
}; 
struct Enemyradius 
{ 
  u32 object1_radius:10; 
	u32 object2_radius:10; 
	u32 object3_radius:10; 
 
}; 
struct OrgPowerRecvMsg 
{ 
	uint16_t capacitance_percentage;//电容能量百分比 
	uint16_t currentMax;//最大电流 
	uint16_t currentNow;//当前电流 
	uint16_t capHealth:1; 
	uint16_t useCapNow:1; 
	uint16_t reserved : 14; 
}; 
 
struct PowerRecvMsg 
{ 
	float capacitance_percentage;//电容能量百分比 
	float currentMax;//最大电流 
	float currentNow;//当前电流 
	u8 capHealth; 
	u8 useCapNow; 
	u8 useCapLast; 
	//todo 实现缓存队列 
	u8 useCapNowBuffer; 
	u8 useCapIndex; 
	float useCapNowVar; 
}; 
 
extern struct Enemyposition enemyposition; 
 
extern struct Enemyradius enemyradius; 
 
void canTx(u8 data[8], CAN_TypeDef *can_x, uint32_t id); 
 
extern struct CanStateStruct canStateStruct; 
extern struct CanStateStruct1 canStateStruct1; 
extern struct CommandToGuardStruct commandtoguardstruct;

 
#define CAN_RX0_INT_ENABLE	0		//0,不使能;1,使能.								     
			 
void Can0_Init(void); 
void Send_Distance_Data(void); 
void Send_Judgement_Game_Data(void); 
void Send_Judgement_Robot_Data(void); 
//void Send_Judgement_Robot_Data2(void); 
void Send_Judgement_RobotBasicState_Data(void); 
void Send_LIMIT_Data(void); 
extern struct PowerSendMsg powerSendMsg; 
extern can1_feedback can1Feedback; 
#endif  
 
 
 
 
