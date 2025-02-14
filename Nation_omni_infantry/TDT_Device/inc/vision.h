#ifndef _VISION_H
#define _VISION_H
#include "board.h"
#include "imu.h"
#include "imu_task.h"
/**
 * @addtogroup TDT_DEVICE_VISION
 * @{
 */

///是否启用应答模式
#define ANSWER_MODE 0
#pragma pack(1)
///视觉发送结构体（MCU->NUC）.
///以21赛季步兵为例，其他兵种或有更改
struct vision_Send_Struct_t
{
	uint8_t frameHeader;		  ///<0xA5
	float gimbalTimeStamp[3];	  ///<陀螺仪数据 
	uint8_t enemyColor : 2;		  ///<0->blue 1->special 2->red
	uint8_t energyBeatMode : 2;	  ///<0--非打符模式   1--打符模式   2--反符
	uint8_t anti_arch : 1;	  //攻击建筑
	uint8_t EnableAutoAim : 1;	  ///<开启自瞄，提醒视觉有跳变时优先选择距离枪口最近的装甲板
	uint8_t SpiningShoot : 1;	  ///<操作手手动判断陀螺
	uint8_t selfspining : 1;   //自身是否小陀螺
	uint8_t buffbeatmode : 1; ///0-small 1-big buff
	float nominalBulletSpeed;	  ///<标称弹速
	float realBulletSpeed;		  ///<实际弹速
	int8_t visionGunErrorFlag;	  ///<枪口偏置 +- 1 Yaw +-2 Pitch +-YawBuff +-PitchBuff
	/*↓↓↓↓↓↓↓↓↓↓↓custom data start↓↓↓↓↓↓↓↓↓↓↓*/
	/*↑↑↑↑↑↑↑↑↑↑↑ custom data end ↑↑↑↑↑↑↑↑↑↑↑*/
	uint16_t CRC16CheckSum;
};

///视觉接收结构体（NUC->MCU）.
///以21赛季步兵为例，其他兵种或有更改
struct vision_Recv_Struct_t
{
	uint8_t frameHeader; //0xA5
	double recvTime;
	float Yaw;					   ///<单位: 度
	float Pitch;				   ///<单位: 度
	
////	float v_yaw;
////	float v_pitch;
	float distance;
	float predict_point_angle;
	uint8_t objID :4;	
	uint8_t no_Obj : 1;			   ///<是否找到目标
	uint8_t beat : 1;			   ///<是否能击打
	uint8_t unLimitedFireTime : 6; ///<目标与己方枪口几乎相对静止，且之后unLimitedFireTime微秒内都满足
	uint8_t objSpining : 1;		   ///<目标是否在陀螺
	uint8_t isInEnergyBeat : 1;	   ///<当前模式是否为能量机关
	uint8_t clearInternal :1;
	///<yaw轴的补偿
	/*↓↓↓↓↓↓↓↓↓↓↓custom data start↓↓↓↓↓↓↓↓↓↓↓*/
	//usart
//	float object_x[3];
//	float object_y[3];
////	float radius[3];
////  int number;
	/*↑↑↑↑↑↑↑↑↑↑↑ custom data end ↑↑↑↑↑↑↑↑↑↑↑*///	
	int8_t nowPitchGunError;	   ///<pitch的补偿
	int8_t nowYawGunError;	
	uint16_t CRC16CheckSum;
};
#pragma pack()

struct visionInfo_t
{
	u16 visionCnt;
	u16 visionFPS;
	u8 offlineFlag;
};

extern vision_Recv_Struct_t vision_RecvStruct;
extern vision_Send_Struct_t vision_SendStruct;
extern visionInfo_t visionInfo;

///串口初始化
void Vision_Init(void);

#ifdef __cplusplus
extern "C"
{
#endif

	void USART3_IRQHandler(void);
	void UART5_IRQHandler(void);
#ifdef __cplusplus
}
#endif

/** @} */

#endif
