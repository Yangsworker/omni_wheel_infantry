/******************************
File name: TDT_Task\src\board.cpp
Description: 滴答定时器的初始化和延时功能，初始化
function:
	——————————————————————————————————————————————————————————————————————————
	void TDT_SysTick_Init(void)
	——————————————————————————————————————————————————————————————————————————
	uint32_t GetSysTime_us(void)
	——————————————————————————————————————————————————————————————————————————
	void DelayUs(uint32_t us)
	——————————————————————————————————————————————————————————————————————————
	void DelayMs(uint32_t ms)
	——————————————————————————————————————————————————————————————————————————
	void TDT_Board_ALL_Init(void)
	——————————————————————————————————————————————————————————————————————————
Author: 祖传
Version: 1.1.1.191112_alpha
Date: 19.11.12
History:
	——————————————————————————————————————————————————————————————————————————
	19.11.12 首次记录
	——————————————————————————————————————————————————————————————————————————
****************************  */
#include "board.h"
#include "cycle.h"
#include "iwdg.h"
#include "can.h"
#include "vision.h"
#include "imu_task.h"
#include "flash_var.h"
#include "led_task.h"
#include "dbus.h"
#include "pwm.h"
#include "adc.h"
#include "chassis_task.h"
#include "imu.h"
#include "ptz_task.h"
#include "judgement.h"


/***宏定义***/
#define TICK_PER_SECOND 1000
#define TICK_US	(1000000/TICK_PER_SECOND)

/***全局变量***/
/*初始化完成标志*/
u8 Init_OK;
//滴答定时器计数变量 ,49天后溢出
volatile uint32_t sysTickUptime=0;
/**
  * @brief 初始化滴答定时器
  * @note 如果修改外部晶振,记得修改 HSE_VALUE，PLL_M
  */
void sysTickInit(void)
{
	RCC_ClocksTypeDef  rcc_clocks;
	uint32_t           cnts;

	RCC_GetClocksFreq(&rcc_clocks);

	cnts = (uint32_t)rcc_clocks.HCLK_Frequency / TICK_PER_SECOND;
	cnts = cnts / 8;//滴答定时器1ms触发一次中断
	//cnts=168000/8;=1ms
	SysTick_Config(cnts);
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
}


/**
 * @brief 从timeFrom到目前的时间间隔，单位秒
 * @param timeFrom 
 * @return float 
 */
float timeIntervalFrom_f(float timeFrom)
{
	return float(getSysTimeUs()) / 1e6f - timeFrom;
}

/**
 * @brief 从timeFrom到目前的时间间隔，单位毫秒
 * @param timeFrom 
 * @return uint64_t 
 */
uint64_t timeIntervalFrom(uint64_t timeFrom)
{
	return getSysTimeUs() - timeFrom;
}

/**
  * @brief US级延时
  * @param us 延时时长，单位US
  * @note 使用滴答延时会造成极大的开销，时间越长，开销越大，尽量在1毫秒以内
  * @warning 此函数在会算入CPU_usage中
  */
void delayUs(uint32_t us)
{
	uint64_t from = getSysTimeUs();
	while(!(getSysTimeUs() > us + from)){}
}



/**
  * @brief MS级延时
  * @param ms 延时时长，单位MS
  * @return
  * @note 注意在操作系统运行下使用此函数务必进入临界区，否则精度不保证
  */
void delayMs(uint32_t ms)
{
	delayUs(ms*1000);
}


/**
  * @brief 总初始化函数
  * @note 禁止使用延时
  */
void boardALLInit(void)
{
	/* 禁止全局中断*/
	__disable_irq();
	/*CAN1初始化*/
	canInit(CAN1);
	
	/*CAN2初始化*/
	canInit(CAN2);
	
	/*LED初始化*/
	ledInit();
	/*陀螺仪初始化*/
	imuInit();
	
	//底盘低通滤波初始化
	l1.SetCutoffFreq(1000, 5);
	l2.SetCutoffFreq(1000, 5);
	l3.SetCutoffFreq(1000, 5);

	ptz.Ptz_Task_Init();
	
//	adcMpuTemp.adcInit();
//	pwmMpuTemp.pwmInit(1000);
	RC.init();
	/*视觉串口初始化*/
	Vision_Init();
	/*看门狗初始化-喂狗在LED*/
	iwdgInit(4,50);
	/*  使能全局中断 */
	__enable_irq();
	/*初始化完成*/
	Init_OK = 1;
}
/***********End of file*****************  */
