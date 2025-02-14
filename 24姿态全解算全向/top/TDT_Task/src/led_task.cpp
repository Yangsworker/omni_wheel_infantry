/******************************
File name: TDT_Task\src\led_task.cpp
Description: 呼吸灯控制任务
function:
	——————————————————————————————————————————————————————————————————————————
	void Led_Task(void *pvParameters)
	——————————————————————————————————————————————————————————————————————————
Author: 肖银河
Version: 1.1.1.191112_alpha
Date: 19.11.12
History: 
	——————————————————————————————————————————————————————————————————————————
	19.11.12 首次完成
	——————————————————————————————————————————————————————————————————————————
****************************  */
#include "led_task.h"
#include "iwdg.h"
#include "vision.h"
#include "myRC_task.h"

//定义板载lED灯对象
Led boardLed = Led(RCC_AHB1Periph_GPIOB , GPIOB , GPIO_Pin_14);
#if defined USE_MAIN_CTRL_2019	
Led	laser = Led(RCC_AHB1Periph_GPIOB , GPIOB, GPIO_Pin_9);
#elif defined USE_MAIN_CTRL_2021_PJ||defined USE_MAIN_CTRL_2021_B	\
	||defined USE_MAIN_CTRL_2021_A||defined USE_MAIN_CTRL_2020
Led laser = Led(RCC_AHB1Periph_GPIOC , GPIOC, GPIO_Pin_8);
#endif

void laserLoadParam();
extern u8 deforceFlag;

uint8_t botCtrlerOffline;
uint8_t botCtrlOfflineCnt;
uint8_t offlineErrorCode;//离线错误码


void ledInit()
{
	/*LED初始化*/
	boardLed.init();
	
	laser.init();
	laser.setLHNagation(0);
	laser.show(0);
}

/**
  * @brief LED任务函数
  * @note 负责LED的控制和喂狗
  * @warning 该函数为重写完成
  */
void Led_Task()
{
	static u8 visioncnt = 0;
	//趁机喂狗
	iwdgFeed();
	
	if(deforceFlag || myRC.visionMode)
		laser.setError(-1,LedES_ConstantDark);
	else
		laser.setError(-1,LedES_ConstantLight);

	//LED状态灯
	boardLed.stateShow(50);
	laser.stateShow(50);

	if(visioncnt++ == 20)
	{
		if(visionInfo.visionCnt == 0)
		{
			visionInfo.offlineFlag = 1;
			visionInfo.visionFPS = 0;
		}
		else
		{
			visionInfo.visionFPS = visionInfo.visionCnt;
			visionInfo.visionCnt = 0;
		}
		visioncnt = 0;
	}
}






