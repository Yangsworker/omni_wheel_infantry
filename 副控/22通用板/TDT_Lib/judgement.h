#ifndef _JUDGEMENT_H
#define _JUDGEMENT_H

#include "board.h"

#ifdef  __DRIVER_GLOBALS
#define __DRIVER_EXT
#else
#define __DRIVER_EXT extern
#endif

#define JudgeBufferLength       255

//裁判系统数据缓存
__DRIVER_EXT uint8_t JudgeDataBuffer[JudgeBufferLength];

//机器人id定义(in survival_data)
#define HERO			1
#define ENGINEER		2
#define INFANTRY1		3
#define INFANTRY2		4
#define INFANTRY3		5
#define AIR				6
#define GUARD			7

#pragma pack (1)//用于结构体的内存对齐， 方便使用联合体调用

typedef struct _vec4u
{
    unsigned char data[4];
} _vec4u;

//格式转换联合体
typedef union
{
    unsigned char U[4];
    float F;
    unsigned short I;
} FormatTrans;


/**
* @brief  judgement data command id
*/
typedef enum
{
    Status_data = 0x0001,  //10Hz
    Result_data = 0x0002,
    Robot_HP_data = 0x0003,

    Site_event_data = 0x0101,  //50hZ
    Site_action_identification_data = 0x0102,  //10hz
    Reservation_bullet_data_for_site_supply_station = 0x0103,
    Robot_warning_data = 0x104,

    Robot_State_Data = 0x0201,
    Real_time_power_and_heat_data = 0x0202,  //50hz
    Robot_position_data = 0x0203,
    Robot_buff_data = 0x0204,
    Energy_State_Data_of_Air_Robot = 0x0205,
    Damage_status_data = 0x0206,
    Real_time_shooting_data = 0x0207,
    Last_bullet_num_data = 0x208,

    Interactive_data_between_robots = 0x0301

} judge_data_id_e;



/**************************/
/*结构体定义*/
/**
* @brief  brief frame header structure definition
*/
typedef struct
{
    unsigned char sof;				//帧头
    unsigned short data_length;		//数据长度（0x301包含内容ID、发送者ID以及接收者ID）
    unsigned char seq;				//包序号，可填0
    unsigned char crc8;				//CRC8-校验
    unsigned short cmdid;			//命令ID
} ext_frame_header_t;

typedef enum
{
    STEP_HEADER = 0,				//帧头数据获取
    STEP_HEADER_CRC8 = 1,			//CRC8校验
    STEP_CMDID_GET = 2,				//获取命令码
    STEP_DATA_TRANSFER = 3,			//数据转移（从JudgeDataBuffer转移至cmd_id对应的联合体中）
    STEP_DATA_CRC16 = 4,			//CRC16校验（暂未加校验）
} unpack_step_e;

/**
* @brief  game information structures definition(0x0001)
*         this package send frequency is 10Hz
*/
typedef struct
{
    unsigned char game_type : 4;			//比赛类型
    unsigned char game_progress : 4;		//比赛进程
    unsigned short stage_remain_time;		//剩余时间
} ext_game_status_t;

/**
* @brief  Result of competition data(0x0002)
*/
typedef struct
{
    unsigned char winner;					//胜利者
} ext_game_result_t;

/**
* @brief  Robot survival  data(0x0003)
* Bit 0-7
*/
typedef struct
{
    unsigned short red_1_robot_HP;
    unsigned short red_2_robot_HP;
    unsigned short red_3_robot_HP;
    unsigned short red_4_robot_HP;
    unsigned short red_5_robot_HP;
    unsigned short red_7_robot_HP;
    unsigned short red_base_HP;
    unsigned short blue_1_robot_HP;
    unsigned short blue_2_robot_HP;
    unsigned short blue_3_robot_HP;
    unsigned short blue_4_robot_HP;
    unsigned short blue_5_robot_HP;
    unsigned short blue_7_robot_HP;
    unsigned short blue_base_HP;
} ext_game_robot_HP_t;

/**
* @brief  Site Event data (0x0101)
*/
typedef struct
{
    unsigned int Landing_field : 2;			//停机坪占领状态
    unsigned int depot_number : 3;			//补给站补血点占领状态
    unsigned int energy_state : 3;			//能量机关状态
    unsigned int strategic_pass : 1;		//己方关口占领状态
    unsigned int pillbox : 1;				//己方碉堡占领状态
    unsigned int Resource_island : 1;		//己方资源岛占领状态
} ext_event_data_t;

/**
* @brief  Site Action Identification data(0x0102)
*/
typedef struct
{
    unsigned char supply_projectile_id;
    unsigned char supply_robot_id;
    unsigned char supply_projectile_step;
    unsigned char supply_projectile_num;
} ext_supply_projectile_action_t;
/**************可借此自动进补给站*************/
/**
* @brief  Reservation_bullet_data_for_site_supply_station (0x0103)
*/
typedef struct
{
    unsigned char supply_projectile_id;
    unsigned char supply_robot_id;
    unsigned char supply_num;
} ext_supply_projectile_booking_t;

/**
* @brief  判罚 (0x0104)
*/
typedef struct
{
    unsigned char level;				//惩罚等级
    unsigned char foul_robot_id;		//惩罚机器人的ID
} ext_referee_warning_t;

/**
* @brief  Robot_State_Data(0x0201)
*/
typedef struct
{
    unsigned char robot_id;
    unsigned char robot_level;
    unsigned short remain_HP;
    unsigned short max_HP;
    unsigned short shooter_heat0_cooling_rate;
    unsigned short shooter_heat0_cooling_limit;
    unsigned short shooter_heat1_cooling_rate;
    unsigned short shooter_heat1_cooling_limit;
    unsigned char mains_power_gimbal_output : 1;
    unsigned char mains_power_chassis_output : 1;
    unsigned char mains_power_shooter_output : 1;
} ext_game_robot_status_t;

/**
* @brief  Real-time power and heat data (0x0202)
*/
typedef struct
{
    unsigned short chassis_volt;
    unsigned short chassis_current;
    float chassis_power;
    unsigned short chassis_power_buffer;	//缓冲能量
    unsigned short shooter_heat0;
    unsigned short shooter_heat1;
} ext_power_heat_data_t;

/**
* @brief  Robot position data (0x0203)
*/
typedef struct
{
    float x;
    float y;
    float z;
    float yaw;
} ext_game_robot_pos_t;

/**
* @brief  robot buff data (0x0204)
*/
typedef struct
{
    unsigned char enrich_the_blood:1;			//机器人补血加成
	unsigned char cooling_rate_buff:1;			//机器人冷却加成
	unsigned char defence_buff:1;				//机器人防御加成
	unsigned char attack_buff:1;				//机器人攻击加成
} ext_buff_t;

/**
* @brief  Energy State of Air Robot  (0x0205)
*/
typedef struct
{
    unsigned char energy_point;					//无人机能量点
    unsigned char attack_time;					//无人机可攻击时间
} ext_aerial_robot_energy_t;

/**
* @brief  Injury state  (0x0206)
*/
typedef struct
{
    unsigned char armor_id : 4;					//受伤害装甲板
    unsigned char hurt_type : 4;				//扣血类型
} ext_robot_hurt_t;

/**
* @brief  Real-time shooting information  (0x0207)
*/
typedef struct
{
    unsigned char bullet_type;					//子弹类型
    unsigned char bullet_freq;					//子弹射频
    float bullet_speed;							//子弹射速
} ext_shoot_data_t;

/*
*  @brief 剩余子弹发射数 (0x208)
*/
typedef struct
{
    unsigned short bullet_remaining_num;		//剩余子弹数目（哨兵、无人机）
} ext_bullet_remaining_t;

/**
* @brief Interactive Data Receiving Information  (0x0301)
*
* 总共长最大为 128 个字节减去 frame_header,cmd_id,frame_tail 以及数据段头结构 的 6 个字节，故而发送的内容数据段最大为 113。
* 整个交互数据 0x0301 的包上行频率为 10Hz。
*	机器人 ID：    	1， 英雄(红)；2， 工程(红)；3/4/5，   步兵(红)；6， 空中(红)；7， 哨兵(红)；
*					11，英雄(蓝)；12，工程(蓝)；13/14/15，步兵(蓝)；16，空中(蓝)；17，哨兵(蓝)。
*
*				0xD180  		6 + 13  客户端自定义数据
*				0x0200~0x02FF  	6+n  	己方机器人间通信
*
*				客 户 端 ID： 	0x0101 为 英 雄 操作 手客 户端( 红) ；
*								0x0102， 工程 操作 手客 户端 ((红 )；
*								0x0103/0x0104/0x0105，步兵操作手客户端(红)；
*								0x0106，空中操作手客户端((红)；
*								0x0111，英雄操作手客户端(蓝)；
*								0x0112，工程操作手客户端(蓝)；
*								0x0113/0x0114/0x0115，操作手客户端步兵(蓝)；
*								0x0116，空中操作手客户端(蓝)。
*/
typedef struct
{
    unsigned short data_cmd_id;
    unsigned short sender_ID;
    unsigned short receiver_ID;
} ext_student_interactive_header_data_t;




/**
* @brief 往其他机器人发送的数据(ID为0x0201-0x02ff,具体 ID 含义待定),data最大为113，内容待编写
*/

typedef struct
{
    ext_frame_header_t frame_header;
    ext_student_interactive_header_data_t ID;
    unsigned char data[113];
    unsigned short CRC_16;
} ext_robot_interactive_data_t;

/***********图形名字联合体，即可已赋值5个字符，又可赋值一个int32位的数据（虽然最高可以支持40位）***********/
union graphic_name_t
{
	unsigned char name[5];
	int32_t number;
};
typedef struct
{
    ext_frame_header_t frame_header;
    ext_student_interactive_header_data_t ID;
    unsigned char operate_tpye;
    unsigned char graphic_tpye;
	union graphic_name_t graphic_name;		//联合体解释如上
    unsigned char layer;
    unsigned char color;
    unsigned char width;
    unsigned short start_x;
    unsigned short start_y;
    unsigned short radius;
    unsigned short end_x;
    unsigned short end_y;
    int16_t start_angle;
    int16_t end_angle;
    unsigned char text_lenght;
    unsigned char text[30];
    unsigned short CRC_16;
} ext_client_graphic_draw_t;

/***************************************/

/*联合体定义dataBuff为所存储的JudgeDataBuffer对应的字符串*/
union frame_header_t
{
    ext_frame_header_t data;
    unsigned char  dataBuff[8];
};
union game_status_t
{
    ext_game_status_t data;
    unsigned char dataBuff[3];
};
union game_result_t
{
    ext_game_result_t data;
    unsigned char dataBuff;
};
union game_robot_HP_t
{
    ext_game_robot_HP_t data;
    unsigned char dataBuff[28];
    unsigned short HP_Buff[14];		//当遍历兵种HP不方便时可使用此数组
};
union event_data_t
{
    ext_event_data_t data;
    unsigned char dataBuff[4];
};
union supply_projectile_action_t
{
    ext_supply_projectile_action_t data;
    unsigned char dataBuff[4];
};
union supply_projectile_booking_t
{
    ext_supply_projectile_booking_t data;
    unsigned char dataBuff[3];
};
union referee_warning_t
{
    ext_referee_warning_t data;
    unsigned char dataBuff[2];
};
union game_robot_status_t
{
    ext_game_robot_status_t data;
    unsigned char dataBuff[15];
};
union power_heat_data_t
{
    ext_power_heat_data_t data;
    unsigned char dataBuff[14];
};
union game_robot_pos_t
{
    ext_game_robot_pos_t data;
    unsigned char dataBuff[16];
};
union buff_t
{
    ext_buff_t data;
    unsigned char dataBuff;
};
union aerial_robot_energy_t
{
    ext_aerial_robot_energy_t data;
    unsigned char dataBuff[2];
};
union robot_hurt_t
{
    ext_robot_hurt_t data;
    unsigned char dataBuff;
};
union shoot_data_t
{
    ext_shoot_data_t data;
    unsigned char dataBuff[6];
};
union bullet_remaining_t
{
    ext_bullet_remaining_t data;
    unsigned char dataBuff[2];
};

union robot_interactive_data_t
{
    ext_robot_interactive_data_t data;
    unsigned char dataBuff[128];
};
union client_graphic_draw_t
{
    ext_client_graphic_draw_t data;
    unsigned char dataBuff[70];
};

/**************裁判系统数据结构体***************/
typedef struct
{
    union frame_header_t jgmt_frame_header;

    union game_status_t jgmt_game_status;
    union game_result_t jgmt_game_result;
    union game_robot_HP_t jgmt_game_robot_HP;
    union event_data_t jgmt_event_data;
    union supply_projectile_action_t jgmt_supply_projectile_action;
    union supply_projectile_booking_t jgmt_supply_projectile_booking;
    union referee_warning_t jgmt_referee_warning;
    union game_robot_status_t jgmt_game_robot_status;
    union power_heat_data_t jgmt_power_heat_data;
    union game_robot_pos_t jgmt_game_robot_pos;
    union buff_t jgmt_buff;
    union aerial_robot_energy_t jgmt_aerial_robot_energy;
    union robot_hurt_t jgmt_robot_hurt;
    union shoot_data_t jgmt_shoot_data;
    union bullet_remaining_t jgmt_bullet_remaining;

    union client_custom_data_t jgmt_client_custom_data;
    union robot_interactive_data_t jgmt_robot_interactive_data;
} jgmt_mesg_t;			//


extern jgmt_mesg_t jgmt_mesg;//接收信息
extern union robot_interactive_data_t send_interactive_data;//发送信息



#pragma pack ()
void Judge_Uart3_Config(void);
unsigned char get_length(unsigned short cmdid);
void judge_Ring_queue(void);
void Uart5_SendChar(unsigned char b);
void Judge_send_to_client(void);
void aim_point_draw(u8 type, int16_t offset_x, int16_t offset_y,u8 WIDE, u16 LENTH, u8 progress);
void unpointable_test(uint16_t y);//可绘范围测试
u8 engineer_pointing(int16_t offset_x, int16_t offset_y, u8 WIDE, u16 radius);
void back_camera_tips(int8_t tips_dir);//左:1右:-1中间为0清空为其他值


extern int16_t judge_shoot_num;
extern float StartTime;			//时序数据，用于判断指示灯及数据的可靠性与及时性
extern u8 JgmtOfflineCheck;		//裁判系统离线计数
extern u8 JgmtOffline;			//裁判系统离线
#endif /*_JUDGEMENT_H*/
