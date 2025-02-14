#include "schedule.h"
#include "can.h"
#include "string.h"
#include "tdt_draw.h"
#include "judgement.h"
/*裁判系统测频用
extern int Status_data_cnt, Robot_survival_data_cnt, Robot_State_Data_cnt, Real_time_power_and_heat_data_cnt, Robot_position_data_cnt;
int AVERAGE_Status_data_cnt = 0, AVERAGE_Robot_survival_data_cnt = 0, AVERAGE_Robot_State_Data_cnt = 0, AVERAGE_Real_time_power_and_heat_data_cnt = 0, AVERAGE_Robot_position_data_cnt = 0;
float Status_data_cnt_freq, Robot_survival_data_cnt_freq, Robot_State_Data_cnt_freq, Real_time_power_and_heat_data_cnt_freq, Robot_position_data_cnt_freq;
float AVERAGE_Status_data_cnt_freq = 0, AVERAGE_Robot_survival_data_cnt_freq = 0, AVERAGE_Robot_State_Data_cnt_freq = 0, AVERAGE_Real_time_power_and_heat_data_cnt_freq = 0, AVERAGE_Robot_position_data_cnt_freq = 0;
float SUM_Status_data_cnt_freq = 0, SUM_Robot_survival_data_cnt_freq = 0, SUM_Robot_State_Data_cnt_freq = 0, SUM_Real_time_power_and_heat_data_cnt_freq = 0, SUM_Robot_position_data_cnt_freq = 0;
*/
//reset
struct _schedule schedule;
extern int hurt_flag;

int test = 1;

int MidLvTime = 5;
int LowLvTime = 100;
float MidKp = 1.0f;
float LowKp = 1.0f;
u8 gimbalOffline = 0;
int gimbalOffineCheck = 0;
extern u8 superPowerOffline,superPowerOfflineCheck;
u8 jgmtDrawCounter = 0;
void Loop_1000Hz(void)	//1ms执行一次
{
	static int MidTimer, LowTimer;
	judge_Ring_queue();	//经检验, 裁判系统一圈的计数约为60ms, 且该函数放于主函数易被打断,故放Loop_1000Hz函数中

	if (hurt_flag > 0)
	{
		hurt_flag--;
	}
	else
	{
		jgmt_mesg.robotHurt.armorId = 0x5;
	}
		
	if (++MidTimer > MidLvTime * MidKp)
	{
		MidTimer = 0;

		Send_Judgement_Robot_Data();  //0x111 //弹速热量等基本ui
//		Send_Judgement_Robot_Data2();
		Send_Judgement_Game_Data();   //0x112 //比赛消息-比赛进程、机器人等级等消息
	}
	else if (++LowTimer > LowLvTime * LowKp)
	{
		LowTimer = 0;
		Send_Judgement_RobotBasicState_Data();  //0x403
		Send_LIMIT_Data();        //0x404
	}
	
	jgmtDrawCounter++;
	if(jgmtDrawCounter > 34) //30ms发一次
	{
		jgmtDrawCounter = 0;
		draw_schedule1(); 
	}
}
#include "can.h"
void Loop_500Hz(void)	 //2ms执行一次（由主控发送）
{
//	if (jgmt_mesg.gameStatus.gameProgress == 2 || jgmt_mesg.gameStatus.gameProgress == 3)
//	{
//		MidKp = 0.5f;
//		LowKp = 1.5f;
//	}
//	else
//	{
//		float p1 = (float)jgmt_mesg.powerHeatData.shooterId1_17mmCoolingHeat / (120 + 120 * jgmt_mesg.gameRobotStatus.robotLevel);
//		float p2 = 1.0f - (float)jgmt_mesg.powerHeatData.chassisPowerBuffer / 60;
//		MidKp = 1.0 + 0.6f * LIMIT(p1, 0, 1.0) + 0.4f * LIMIT(p2, 0, 1.0);
//		LowKp = 1.0 + 0.75f * LIMIT(p1, 0, 1.0) + 0.75f * LIMIT(p2, 0, 1.0);
//	}
//	
//	powerSendMsg.chassis_power = jgmt_mesg.powerHeatData.chassisPower;
//	powerSendMsg.chassis_power_buffer = jgmt_mesg.powerHeatData.chassisPowerBuffer;
//	powerSendMsg.max_chassis_power = jgmt_mesg.gameRobotStatus.chassisPowerLimit;
//	canTx((u8*)&powerSendMsg,CAN1, 0x110);


}
void Loop_200Hz(void)	 //5ms执行一次
{
//	if(gimbalOffline)
//	{
//		replaceMainCtrlSendData();//会导致通用发送错误
//	}
//	if(!superPowerOffline)
//	{
//		Send_Superpower_Data_To_Maincontrol();
//	}
}

int8_t aim_point_cnt = 0,find_sub_obj = 0;
void Loop_100Hz(void)  //10ms执行一次
{
	if (++JgmtOfflineCheck > 20)		//离线判断时间20 * 10ms
	{
		JgmtOffline = 1;
		JgmtOfflineCheck = 250;		//防止溢出
	}
	else JgmtOffline = 0;
	if (++gimbalOffineCheck > 20)		//离线判断时间20 * 10ms
	{
		gimbalOffline = 1;
		gimbalOffineCheck = 250;		//防止溢出
	}
	else gimbalOffline = 0;
	Send_Distance_Data();    //0x103
}


void Loop_50Hz(void)	//20ms执行一次
{
	superPowerOfflineCheck++;
	if(superPowerOfflineCheck>50)
	{
		superPowerOffline = 1;
	}

}

void Loop_20Hz(void)	//50ms执行一次
{
	extern u8 useCap;
	extern u8 ULTS_mode;
	if(gimbalOffline == 1)
	{
		useCap = 1;
		ULTS_mode = 0;
	}
	USART_Send_to_SuperPower(jgmt_mesg.powerHeatData.chassisPower,jgmt_mesg.gameRobotStatus.chassisPowerLimit,jgmt_mesg.powerHeatData.chassisPowerBuffer,useCap);
}

int8_t time_cut1, time_cut2, num = 0;
unsigned char tmpdata[2] = {0x11,0x22};
int DrawTmp = 0;
int pp=0;
void Loop_10Hz(void)	//100ms执行一次
{
	pp++;
	time_cut1++;
    time_cut2++;
	/*LED灯闪烁*/
	if (time_cut1 == 3)
	{
		time_cut1 = 0;
		LED_TOGGLE;
	}
	if (time_cut2 == 5)
	{
		time_cut2 = 0;
		for (num = 0; num < 6; num++)
		{
			if (can1Feedback.Light_Toggle[num] == 1 || can1Feedback.Light_FastToggle[num] == 1)
			{
				if (can1Feedback.Light[num] == 0) can1Feedback.Light[num] = 1;
				else can1Feedback.Light[num] = 0;
			}
		}
	}
	

//	if(pp>50){
//			__set_FAULTMASK(1);							  //关闭所有中断
//			NVIC_SystemReset();
//		pp=0;
//	}
	
}





void Loop(void)
{
	if (schedule.cnt_1ms >= 1)
	{
		Loop_1000Hz();
		schedule.cnt_1ms = 0;
	}
	if (schedule.cnt_2ms >= 2)
	{
		Loop_500Hz();
		schedule.cnt_2ms = 0;
	}
	if (schedule.cnt_5ms >= 5)
	{
		Loop_200Hz();
		schedule.cnt_5ms = 0;
	}
	if (schedule.cnt_10ms >= 10)
	{
		Loop_100Hz();
		schedule.cnt_10ms = 0;
	}
	if (schedule.cnt_20ms >= 20)
	{
		Loop_50Hz();
		schedule.cnt_20ms = 0;
	}
	if (schedule.cnt_50ms >= 50)
	{
		Loop_20Hz();
		schedule.cnt_50ms = 0;
	}
	if (schedule.cnt_100ms >= 100)
	{
		Loop_10Hz();
		schedule.cnt_100ms = 0;
	}
}
