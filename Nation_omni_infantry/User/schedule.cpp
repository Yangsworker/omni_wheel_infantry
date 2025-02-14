#include "schedule.h"
#include "dbus.h"
#include "imu_task.h"
#include "led_task.h"
#include "judgement.h"
#include "motor.h"
#include "pwm.h"
#include "adc.h"
#include "chassis_task.h"
#include "ptz_task.h"
#include "fire_task.h"
#include "power.h"
#include "state_task.h"
#include "ammo_cover_task.h"

extern float send_roll_offset;  //校准使每次向视觉发送的roll轴角度校准
void TDT_Loop_1000Hz(void) //1ms执行一次
{
	RC.run_1000Hz();
#if USE_JUDGEMENT
	ringQueue();

	send_roll_offset = -(boardImu -> Angle.roll);
	
#endif

}
void TDT_Loop_500Hz(void) //2ms执行一次
{
//	adcMpuTemp.Get_Adc();
//	pwmMpuTemp.pwmCalculate(50);
	Imu_Task();
	Chassis_Task();   //底盘测试函数
	Ptz_task();      //云台测试函数
	Fire_Task();        //枪测试函数
	Ammo_Task();     //弹舱盖测试函数
	fastselfcheck.run();
	
	State_Ctrl();    //状态机ui测试
	State_Ctrl_RC_Info(); //视觉模式发送
	stateCtrl.changeVisionError();
	Motor::sendCanMsg();
}

void TDT_Loop_200Hz(void) //5ms执行一次
{
    //修改
}

void TDT_Loop_100Hz(void) //10ms执行一次
{
	
}

void TDT_Loop_50Hz(void) //20ms执行一次
{
	
}

void TDT_Loop_20Hz(void) //50ms执行一次
{
	Led_Task();
}

void TDT_Loop_10Hz(void) //100ms执行一次
{
}

void TDT_Loop_2Hz(void) //500ms执行一次
{
}

void TDT_Loop_1Hz(void) //1000ms执行一次
{
}

/**
 * @ingroup TDT_Frame
 * @defgroup TDT_SCHEDULE_API schedule相关接口
 * @brief 该模块展示了schedule的相关接口
 */


void TDT_Loop(struct _Schedule *robotSchedule)
{
	uint64_t startTimeStamp = getSysTimeUs();
	TDT_Loop_1000Hz();
	robotSchedule->runTime_1ms = getSysTimeUs() - startTimeStamp;
	
	if (robotSchedule->cnt_2ms >= 2)
	{
		robotSchedule->cnt_2ms = 0;
		uint64_t startTimeStamp = getSysTimeUs();
		TDT_Loop_500Hz();
		robotSchedule->runTime_2ms = getSysTimeUs() - startTimeStamp;
	}
	if (robotSchedule->cnt_5ms >= 5)
	{
		robotSchedule->cnt_5ms = 0;
		uint64_t startTimeStamp = getSysTimeUs();
		TDT_Loop_200Hz();
		robotSchedule->runTime_5ms = getSysTimeUs() - startTimeStamp;
	}
	if (robotSchedule->cnt_10ms >= 10)
	{
		robotSchedule->cnt_10ms = 0;
		uint64_t startTimeStamp = getSysTimeUs();
		TDT_Loop_100Hz();
		robotSchedule->runTime_10ms = getSysTimeUs() - startTimeStamp;
	}
	if (robotSchedule->cnt_20ms >= 20)
	{
		robotSchedule->cnt_20ms = 0;
		uint64_t startTimeStamp = getSysTimeUs();
		TDT_Loop_50Hz();
		robotSchedule->runTime_20ms = getSysTimeUs() - startTimeStamp;
	}
	if (robotSchedule->cnt_50ms >= 50)
	{
		robotSchedule->cnt_50ms = 0;
		uint64_t startTimeStamp = getSysTimeUs();
		TDT_Loop_20Hz();
		robotSchedule->runTime_50ms = getSysTimeUs() - startTimeStamp;
	}
	if (robotSchedule->cnt_100ms >= 100)
	{
		robotSchedule->cnt_100ms = 0;
		uint64_t startTimeStamp = getSysTimeUs();
		TDT_Loop_10Hz();
		robotSchedule->runTime_100ms = getSysTimeUs() - startTimeStamp;
	}
	if (robotSchedule->cnt_500ms >= 500)
	{
		robotSchedule->cnt_500ms = 0;
		uint64_t startTimeStamp = getSysTimeUs();
		TDT_Loop_2Hz();
		robotSchedule->runTime_500ms = getSysTimeUs() - startTimeStamp;
	}
	if (robotSchedule->cnt_1000ms >= 1000)
	{
		robotSchedule->cnt_1000ms = 0;
		uint64_t startTimeStamp = getSysTimeUs();
		TDT_Loop_1Hz();
		robotSchedule->runTime_1000ms = getSysTimeUs() - startTimeStamp;
	}
	robotSchedule->CPU_usage = (robotSchedule->runTime_1ms*1000 + robotSchedule->runTime_2ms*500 + robotSchedule->runTime_5ms*200 + robotSchedule->runTime_10ms*100 + robotSchedule->runTime_20ms*50 + robotSchedule->runTime_50ms*20 + robotSchedule->runTime_100ms*10 + robotSchedule->runTime_500ms*2 + robotSchedule->runTime_1000ms)/1e6f;
}
