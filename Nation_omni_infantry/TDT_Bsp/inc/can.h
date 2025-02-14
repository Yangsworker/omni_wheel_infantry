/*
 * @Author: your name
 * @Date: 2021-05-09 06:17:01
 * @LastEditTime: 2021-05-10 12:18:02
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \Projectd:\TDT\TDT-Infantry\Infantry_II\TDT_Bsp\inc\can.h
 */
/*****************************************************************************
File name: TDT_Bsp\src\can.cpp
Description: can底层
Author: 肖银河
Version: 1.1.1.191112_alpha
Date: 19.11.12
History:
	——————————————————————————————————————————————————————————————————————————
	19.11.16 #合成can1.c和can2.c #修改接收函数，从队列传递改为直接解算
	——————————————————————————————————————————————————————————————————————————
	19.11.12 #首次完成
	——————————————————————————————————————————————————————————————————————————
*****************************************************************************/
#ifndef __CAN1_H__
#define __CAN1_H__

#include "board.h"

///@addtogroup TDT_BSP_CAN
///@{

void canInit(CAN_TypeDef *can_x);
//void TDT_Can_Tx(vec4f* value,CAN_TypeDef * can_x,uint32_t id);
void canTx(float *data, CAN_TypeDef *can_x, uint32_t id);
void canTx(u8 data[8], CAN_TypeDef *can_x, uint32_t id);

///@}

#ifdef __cplusplus
extern "C"
{
#endif /*__cplusplus*/

	void CAN1_TX_IRQHandler(void);
	void CAN2_TX_IRQHandler(void);
	void CAN1_RX0_IRQHandler(void);
	void CAN2_RX0_IRQHandler(void);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

struct Can1Feedback
{ /*裁判系统数据*/
	int8_t game_progress;       //游戏进程
	int8_t hurt_armor_id;       //受击打装甲id
	int8_t hurt_type;           //伤害类型
	int8_t robot_level;         //机器人等级
	int8_t buff_type;           //buff类型
	int16_t stageRemainTime;    //阶段剩余时间
	float Jgmt_RealPower;       //裁判系统实时功率值
	float remainPowerBuffer;    //剩余缓冲
	uint16_t Jgmt_ShootNum;     //裁判系统射击数量
	uint16_t chassisPowerLimit; //底盘功率上限
	uint16_t shooterId1_17mmSpeedLimit;  //射速上限
	float SuperPowerRemain_P;
	float SuperPower_RealPower;
	float Boost_V;
	float SuperPowerReady;
	float Jgmt_OutSpd;         //出膛速度
	float lastJgmtOutSpd;
	float Jgmt_Heat;   //实时热量数据
	u16 CoolRate;      //冷却速率
	int16_t MaxHeat;   //热量上限
	int16_t remain_hp;
	int16_t max_hp;    //最高血量

	u8 EnemyColor;     //敌方颜色

	u8 SuperPowerOffline;
	u8 SuperPowerOfflineCheck;
	u8 jgmtOffline;      //裁判系统离线
	u8 jgmtOfflineCheck; //离线检查系数
	float GYdistance[3]; //ui相关
};

#pragma pack(1)
struct PowerSendMsg  //0x110发送消息
{
	uint32_t haha :10;
	uint32_t hahaha:10;
	uint32_t chassis_power:10;
	uint16_t chassis_power_buffer;
	uint16_t max_chassis_power;
};

struct OrgPowerRecvMsg   //电容消息
{
	uint16_t capacitance_percentage;//电容能量百分比
	uint16_t currentMax;//最大电流
	uint16_t currentNow;//当前电流
	uint16_t capHealth:1;
	uint16_t useCapNow:1;
	uint16_t reserved : 14;
};

#define useCapNowBufferCnt 50
struct PowerRecvMsg            //0x130收消息
{
	float capacitance_percentage;//电容能量百分比
	float currentMax;//最大电流
	float currentNow;//当前电流
	u8 capHealth;
	u8 useCapNow;
	u8 useCapLast;
	//todo 实现缓存队列
	u8 useCapNowBuffer=0;
	u8 useCapIndex=0;
	float useCapNowVar;
};
#pragma pack()

extern Can1Feedback can1Feedback;

extern PowerSendMsg powerSendMsg;
extern PowerRecvMsg powerRecvMsg;
#endif
