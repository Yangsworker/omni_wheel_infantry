#ifndef __JUDGEMENT_H
#define __JUDGEMENT_H
#include "board.h"


/**
 * @addtogroup TDT_DEVICE_JUDGEMENT
 * @{
 */
 extern float shootingSpeed[3] ;
enum RobotIdDef
{
	ID_HERO = 1,
	ID_ENGINEER = 2,
	ID_INFANTRY1 = 3,
	ID_INFANTRY2 = 4,
	ID_INFANTRY3 = 5,
	ID_AIR = 6,
	ID_GUARD = 7,
	ID_RADAR = 9
};

#define ID1_17mm_SHOOT_NUM 0
#define ID2_17mm_SHOOT_NUM 1
#define ID1_42mm_SHOOT_NUM 2

enum JudgementStep
{
	STEP_HEADER = 0,		//<帧头数据获取
	STEP_HEADER_CRC8 = 1,	//<CRC8校验
	STEP_CMDID_GET = 2,		//<获取命令码
	STEP_DATA_TRANSFER = 3, //<数据转移（从JudgeDataBuffer转移至cmd_id对应的联合体中）
	STEP_DATA_CRC16 = 4,	//<CRC16校验（暂未加校验）
};

#define JUDGE_BUFFER_LENGTH 255

#define PURE_DATA_LEN (113)				///<不算ID与帧头的长度，用于定义data[Pure_data_len]
#define DATA_LENGTH (6 + PURE_DATA_LEN) ///<不算帧头的长度，用于赋值frame_header.data_length
#define ALL_LEN (PURE_DATA_LEN + 15)	///<总长，用于CRC16校验

#pragma pack(1) //用于结构体的内存对齐， 方便使用联合体调用
class Judgement
{
	struct FrameHeader
	{
		unsigned char sof;		   ///<帧头
		unsigned short dataLength; ///<数据长度（0x301包含内容ID、发送者ID以及接收者ID）
		unsigned char seq;		   ///<包序号，可填0
		unsigned char crc8;		   ///<CRC8-校验
		unsigned short cmdid;	   ///<命令ID
	};

public:
	/******************以下函数调用频率最高为10Hz***************/
	/// 0x301联合体数据发送，需将dataCmdId、senderId、receiverId填写完，一般不调用此函数
	void customSend(u8 count);
	/// 填写与机器人交互的数据后调用此函数可直接发送
	void robotsCommunication(uint16_t dataCmdId, RobotIdDef robotIdDef, u8 dataLenth);
	/// 发送sendGraphics个图形
	void graphicDraw(u8 sendGraphics);
	/// 发送字符传
	void characterDraw();
	/// 删除图层
	void graphicDel();
	/******************以上函数调用频率最高为10Hz***************/
	///发送地图命令
	void mapCommandSend();
	void PathSend();
	//初始化
	void init();

	//队列解析
	void ringQueue();

	uint16_t IdToMate(RobotIdDef robotIdDef);
	uint16_t IdToMate(uint16_t robotId);
	uint16_t ClientId();
	uint16_t IdToEnemy(RobotIdDef robotIdDef);
	uint16_t IdToEnemy(uint16_t robotId);

	struct GameStatus
	{
		uint8_t gameType : 4;
		uint8_t gameProgress : 4;
		uint16_t stageRemainTime;
		uint64_t syncTimeStamp;
	} gameStatus;

	struct GameResult
	{
		uint8_t winner;
	} gameResult;

	union GameRobotHP
	{
		u16 teamHp[2][8];
		u16 allHp[16];
		struct
		{
			uint16_t red_1RobotHp;
			uint16_t red_2RobotHp;
			uint16_t red_3RobotHp;
			uint16_t red_4RobotHp;
			uint16_t red_5RobotHp;
			uint16_t red_7RobotHp;
			uint16_t redOutpostHp;
			uint16_t redBaseHp;
			uint16_t blue_1RobotHp;
			uint16_t blue_2RobotHp;
			uint16_t blue_3RobotHp;
			uint16_t blue_4RobotHp;
			uint16_t blue_5RobotHp;
			uint16_t blue_7RobotHp;
			uint16_t blueOutpostHp;
			uint16_t blueBaseHp;
		} singleHp;
	} gameRobotHP;

	struct DartStatus
	{
		uint8_t dartBelong;
		uint16_t stageRemainingTime;
	} dartStatus;

	struct ICRA_BuffDebuffZoneStatus_t
	{
		uint8_t f1ZoneStatus : 1;
		uint8_t f1ZoneBuffDebuffStatus : 3;
		uint8_t f2ZoneStatus : 1;
		uint8_t f2ZoneBuffDebuffStatus : 3;
		uint8_t f3ZoneStatus : 1;
		uint8_t f3ZoneBuffDebuffStatus : 3;
		uint8_t f4ZoneStatus : 1;
		uint8_t f4ZoneBuffDebuffStatus : 3;
		uint8_t f5ZoneStatus : 1;
		uint8_t f5ZoneBuffDebuffStatus : 3;
		uint8_t f6ZoneStatus : 1;
		uint8_t f6ZoneBuffDebuffStatus : 3;
		uint16_t red1BulletLeft;
		uint16_t red2BulletLeft;
		uint16_t blue1BulletLeft;
		uint16_t blue2BulletLeft;
	} ICRA_BuffDebuffZoneStatus;

	struct EventData
	{
		uint32_t eventType;
	} eventData;

	struct SupplyProjectileAction
	{
		uint8_t supplyProjectileId;
		uint8_t supplyRobotId;
		uint8_t supplyProjectileStep;
		uint8_t supplyProjectileNum;
	} supplyProjectileAction;

	struct RefereeWarning
	{
		uint8_t level;
		uint8_t foulRobotId;
		uint8_t count;
	} refereeWarning;

	struct DartRemainingTime
	{
		uint8_t time;
		uint16_t dart_info;
	} dartRemainingTime;

	struct GameRobotStatus
	{
		uint8_t robotId;
		uint8_t robotLevel;
		uint16_t remainHp;  
		uint16_t maxHp;
		uint16_t shooter_barrel_cooling_value;  //枪口每秒冷却值
		uint16_t shooter_barrel_heat_limit;    //枪口热量上限
		uint16_t chassisPowerLimit;
		uint8_t mainsPowerGimbalOutput : 1;    //gimbal:0-无输出 1-24V输出
		uint8_t mainsPowerChassisOutput : 1;   //chassis
		uint8_t mainsPowerShooterOutput : 1;   //shooter
	} gameRobotStatus;

	struct PowerHeatData
	{
		uint16_t chassisVolt;        //chassis口电压（mV）
		uint16_t chassisCurrent;      //mA
		float chassisPower;
		uint16_t chassisPowerBuffer;   //缓冲能量
		uint16_t shooterId1_17mmCoolingHeat;
		uint16_t shooterId2_17mmCoolingHeat;
		uint16_t shooterId1_42mmCoolingHeat;
	} powerHeatData;

	struct GameRobotPos
	{
		float x;
		float y;
		float yaw;
		} gameRobotPos;  //本机器人位置

	struct Buff
	{
		uint8_t recovery_buff;
		uint8_t cooling_buff;
		uint8_t defence_buff;
		uint8_t vulnerability_buff;
		uint16_t attack_buff;
	} buff;

	struct AerialRobotEnergy
	{
		uint8_t airforce_status;
		uint8_t time_remain;
		} aerialRobotEnergy;   //空中机器人状态

	struct RobotHurt
	{
		uint8_t armorId : 4;
		uint8_t hurtType : 4;  
	} robotHurt;

	struct ShootData
	{
		uint8_t bulletType;
		uint8_t shooterId;
		uint8_t bulletFreq; //弹频（Hz）
		float bulletSpeed;  //弹速
	} shootData; 

	struct BulletRemaining
	{
		uint16_t bulletRemainingNum_17mm;  //允许发弹量
		uint16_t bulletRemainingNum_42mm;
		uint16_t coinRemainingNum; 
	} bulletRemaining;

	struct RfidStatus
	{
		uint32_t rfidStatus;
	} rfidStatus;

	struct DartClientCmd
	{
		uint8_t dart_launch_opening_status;
		uint8_t reserved;
		uint16_t target_change_time;
		uint16_t latest_launch_cmd_time;
	} dartClientCmd;
	
	 struct Ground_robot_position_t
	{
		 float hero_x;
		 float hero_y;
		 float engineer_x;
		 float engineer_y;
		 float standard_3_x;
		 float standard_3_y;
		 float standard_4_x;
		 float standard_4_y;
		 float standard_5_x;
		 float standard_5_y;
	}ground_robot_position_t;
	
	struct Radar_mark_data_t
	{
	 uint8_t mark_hero_progress;
	 uint8_t mark_engineer_progress;
	 uint8_t mark_standard_3_progress;
	 uint8_t mark_standard_4_progress;
	 uint8_t mark_standard_5_progress;
	 uint8_t mark_sentry_progress;
		}radar_mark_data_t;  //对方机器人被雷达标记进度
	
	struct  Sentry_info_t
	{
	 uint32_t sentry_info;
	} sentry_info_t;

	 struct Radar_info_t
	{
	 uint8_t radar_info;
	} radar_info_t;

	struct StudentInteractiveData
	{
		uint16_t dataCmdId;
		uint16_t senderId;
		uint16_t receiverId;
		uint8_t data[113];
	} studentRecviveData;

	struct ClientCustomGraphicDelete
	{
		uint8_t operateType;
		uint8_t layer;
	};

	struct GraphicDataStruct
	{
		uint8_t graphicName[3];
		uint32_t operateType : 3;
		uint32_t graphicType : 3;
		uint32_t layer : 4;
		uint32_t color : 4;
		uint32_t startAngle : 9;
		uint32_t endAngle : 9;
		uint32_t width : 10;
		uint32_t startX : 11;
		uint32_t startY : 11;
		uint32_t radius : 10;
		uint32_t endX : 11;
		uint32_t endY : 11;
	};

	struct ClientCustomCharacter
	{
		GraphicDataStruct graphic_data_struct;
		uint8_t data[30];
	};

	struct MapCommand
	{
		float targetPositionX;
		float targetPositionY;
		float targetPositionZ;
		uint8_t cmdKeyboard;
		uint16_t targetRobotId;
	} mapCommand;//接收地图指令

	struct SendUnionData
	{
		struct FrameHeader frameHeader;
		uint16_t dataCmdId;
		uint16_t senderId;
		uint16_t receiverId;

		union
		{
			uint8_t studentSendData[113];
			struct ClientCustomGraphicDelete clientCustomGraphicDelete;
			struct GraphicDataStruct graphicDataStruct[7];
			struct ClientCustomCharacter clientCustomCharacter;
		};
		uint16_t CRC16;
	} sendUnionData;

	struct MapCommandData
	{
		struct FrameHeader frameHeader;
		struct MapCommand mapCommand;
		uint16_t CRC16;
	} mapCommandData;
	
	struct PathPlanningData//规划路径信息的结构体
	{
		struct FrameHeader frameHeader;
		/*
			data
		*///////////////
		uint8_t intention ;
		uint16_t start_position_x ;
		uint16_t start_position_y ;
		int8_t delta_x[49] ;
		int8_t delta_y[49] ;
		////////////////
		
		uint16_t CRC16;
	} pathPlanningData ;

	struct WrongStatusCnt
	{
		int CRC8_Wrong_cnt;	 //CRC8校验错误次数累计
		int CRC16_Wrong_cnt; //CRC16校验错误次数累计
		int CYCLE_Wrong_cnt; //包圈错误次数累计
		int SEQ_Wrong_cnt;	 //包序号错误次数累计
							 /*******若以上错误同时出现，则需提高裁判系统解析频率与优先级（防止丢包）******/
	} wrongStatusCnt;

	struct AlliedPos
	{
		float alliedposall[10] ;
//		顺序如下：
//		float hero_x;
//		float hero_y;
//		float engineer_x;
//		float engineer_y;
//		float standard_3_x;
//		float standard_3_y;
//		float standard_4_x;
//		float standard_4_y;
//		float standard_5_x;
//		float standard_5_y;
		}alliedpos ;//所有己方机器人坐标
	
#pragma pack()

	uint16_t shootNum[3] = {0};
	u8 jgmtOffline = 0; ///<裁判系统离线
	int hurtCount = 0;	///<裁判系统受到伤害才更新装甲板,此次与上一次hurtCount不相同即为受攻击
	u8 batteryLevel;

	inline void addFullCount() { judgementFullCount++; }

private:
	enum JudgeDataId
	{
		STATUS_DATA = 0x0001, //10Hz
		RESULT_DATA = 0x0002,
		ROBOT_HP_DATA = 0x0003,
		DART_STATUS = 0x0004,
		ICRA_BUFF_DEBUFF_ZONE_STATUS = 0x0005,

		EVENT_DATA = 0x0101,			   //50hZ
		SUPPLY_PROJECTILE_ACTION = 0x0102, //10hz
		ROBOT_WARNING_DATA = 0x0104,
		DART_REMAINING_TIME = 0x0105,

		GAME_ROBOT_STATUS = 0x0201,
		POWER_HEAT_DATA = 0x0202, //50hz
		GAME_ROBOT_POS = 0x0203,
		BUFF = 0x0204,
		AERIAL_ROBOT_ENERGY = 0x0205,
		ROBOT_HURT = 0x0206,
		SHOOT_DATA = 0x0207,
		BULLET_REMAINING = 0x0208,
		RFID_STATUS = 0x0209,
		DART_CLIENT_CMD = 0x020A,
		ROBOT_POSITION=0x020B,
		RADAR_MARK=0x020C,
		SENTRY_INFO=0x020D,
		RADAR_INFO=0x020E,
		STUDENT_INTERACTIVE_HEADER_DATA = 0x0301
	};

	JudgementStep judgementStep;

	uint8_t judgeDataBuffer[JUDGE_BUFFER_LENGTH];
	uint16_t judgementFullCount;

	uint8_t fullDataBuffer[128];
	void uart6Config(void);
	unsigned char getLength(FrameHeader *frameHeader);
	void uart6SendBytes(void *ptr, u8 len);
	void getJudgeData();
};

void ringQueue();

extern "C" void DMA2_Stream1_IRQHandler(void);

extern Judgement judgement;

/** @} */

// #endif


#endif