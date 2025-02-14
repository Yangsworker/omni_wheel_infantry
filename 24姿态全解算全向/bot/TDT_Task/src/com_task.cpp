#include "com_task.h"
#include "led_task.h"
#include "multi_imu.h"
#include "judgement.h"
#include "schedule.h"
#include "power.h"
#include "my_math.h"
#include "imu_task.h"


CAN_TypeDef *ThCan = CAN1;//设定贯通CAN


/****底盘->云台的联合包****/
JgmtAbout JgmtPack;
LocalAbout LocalPack;
/****云台->底盘的联合包****/
CtrlAbout CtrlPack;
DisplayAbout DisplayPack;

/*******************自定义处理函数*******************/

//信息从贯通CAN发送
void Com_Task(_myPack myPack)
{
	if(myPack == Jgmt)
	{
		JgmtPack.jgmtOfflineFlag = judgement.jgmtOffline;

		JgmtPack.shooterHeatRemainA = LIMIT(judgement.gameRobotStatus.shooter_barrel_heat_limit 
									- judgement.powerHeatData.shooterId1_17mmCoolingHeat,0,500); //当前A枪剩余热
		
		JgmtPack.bulletSpeed = LIMIT((judgement.shootData.bulletSpeed*100),0,4000); //当前弹速
		JgmtPack.bulletCount = judgement.shootNum[ID1_17mm_SHOOT_NUM]+judgement.shootNum[ID2_17mm_SHOOT_NUM]; //发弹量，仅提供跳变，绝对值意义不大

		JgmtPack.powerlimitKp = power.powerLimitKp* 100;
		JgmtPack.yaw_dps = boardImu->gyro.dps.data[1] * 3.14159f / 180.0f * 500.0f;   //30000
//		/*在上方进行信息预处理*/
		canTx(JgmtPack.data,ThCan,0x400);
	}
	else if(myPack == Local)
	{
		LocalPack.myColor = judgement.gameRobotStatus.robotId > 100;

		LocalPack.shooterCoolingLimitA = judgement.gameRobotStatus.shooter_barrel_heat_limit;   //枪口热量上限
		LocalPack.shooterCoolingRateA = judgement.gameRobotStatus.shooter_barrel_cooling_value; //枪口每秒冷却值

//		/*在上方进行信息预处理*/
		canTx(LocalPack.data,ThCan,0x401);
	}
}


//贯通CAN信息解析
void CanInfo_Handle(CanRxMsg ThCanMsg)
{
	switch(ThCanMsg.StdId)
	{
		case 0X500:
			topCtrlOfflineCnt = 0;
			memcpy(CtrlPack.data,ThCanMsg.Data,8);
			if(CtrlPack.sysRST)
			{
				__disable_irq(); //关闭所有中断
				NVIC_SystemReset(); //复位
				while (1)
				{
				} //仅等待复位
			}
			break;
		case 0X501:    //底盘状态（低频）
			topCtrlOfflineCnt = 0;
			memcpy(DisplayPack.data,ThCanMsg.Data,8);
			break;
		default:
			break;
	}
}