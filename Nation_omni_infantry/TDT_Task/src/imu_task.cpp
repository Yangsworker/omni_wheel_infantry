/******************************
File name: TDT_Task\src\imu_task.cpp
Description: 陀螺仪姿态解算任务
function:
	——————————————————————————————————————————————————————————————————————————
	void Imu_Task(void *pvParameters)
	——————————————————————————————————————————————————————————————————————————
Author: 肖银河
Version: 1.1.1.191112_alpha
Date: 19.11.12
History:
	——————————————————————————————————————————————————————————————————————————
	19.11.12 首次完成
	——————————————————————————————————————————————————————————————————————————
****************************  */
#include "imu_task.h"
/**TDT_Device************************/
#include "mpu6050.h"
#include "icm20602.h"
#include "icm20608.h"
#include "scha634_03.h"
#include "TimeMatch.h"
#include "cycle.h"
#include "curve_model.h"
#include "parameter.h"
#include "flash_var.h"
#include "bl24c512a.h"
#include "ptz_task.h"

extern TimeSimultaneity imuTimeMatch;
extern float send_roll_offset;

#define USE_V5_ICM 0
#define USE_V5_MPU 1

MultiImu *boardImu;

eulerAngle angleForWatch;
accdata accForWatch1;
gyrodata gyroForWatch1;

float *visionSendYaw, *visionSendPitch;
u8 currentImuUsed = 0;
u8 currentImuShouldUsed = 0;
void startControlTasks(); //等待imu初始化后开启控制任务
/**
  * @brief 陀螺仪任务
  * @note 负责数据读取和解算
  */
void Imu_Task()
{	
	if (boardImu->forceGetOffset)
	{
		boardImu->getOffset();
	}
	/*MPU6050读取*/
	uint64_t readImuTime = boardImu->TDT_IMU_update();
	if (visionSendYaw != NULL && visionSendPitch != NULL)
		imuTimeMatch.top(float(readImuTime) / 1e6f) << (vec2f({*visionSendYaw, *visionSendPitch}));

#if !ANSWER_MODE
	void vision_Send_Data();
	vision_Send_Data();
#endif
	angleForWatch = boardImu->Angle;
	accForWatch1 = boardImu->acc;
	gyroForWatch1 = boardImu->gyro;
}

#include "flash_var.h"
void imuInit()
{
	u8 currentImuInit = 0;
	boardImu = new MultiImu;
	float accOffset[7] = {0};
	
#if USE_V5_MPU
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	boardImu->validImu[currentImuInit] = new Mpu6050(GPIOC,GPIO_Pin_2,GPIO_Pin_1);
	float gyroScaleFactor[3][3] = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
	boardImu->validImu[currentImuInit]->setGyroScaleFactor(gyroScaleFactor);
	float accScaleFactor[3][3] = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
	boardImu->validImu[currentImuInit]->setAccScaleFactor(accScaleFactor);
	IFlash.link(boardImu->validImu[currentImuInit]->gyro.offset, 2);
	IFlash.link(boardImu->validImu[currentImuInit]->acc.offset, 3);
	currentImuInit++;
#endif
#if USE_V5_ICM
	boardImu->validImu[currentImuInit] = new Icm20602(SPI1,SPI_BaudRatePrescaler_64);
	float gyroScaleFactor1[3][3] = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
	boardImu->validImu[currentImuInit]->setGyroScaleFactor(gyroScaleFactor1);
	float accScaleFactor1[3][3] = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
	boardImu->validImu[currentImuInit]->setAccScaleFactor(accScaleFactor1);
	IFlash.link(boardImu->validImu[currentImuInit]->gyro.offset, 4);
	IFlash.link(boardImu->validImu[currentImuInit]->acc.offset, 5);
	currentImuInit++;
#endif


	/*icm20602以及MPU6050初始化*/
	boardImu->init();
	boardImu->getOffset();
	delayMs(50);
	boardImu->initalAngle();
	
	boardImu->forceAppoint=1;
	boardImu->forceImuUsed=0;
	
//	for(int i = 0; i < currentImuInit; i++)
//	{
//		bl24C512.readPage(i*6*4, (u8*)accOffset, 6*4);
//		boardImu->validImu[i]->calOffset_AccFromOffset(accOffset);
//	}

	boardImu->imu_OK = 1;

	//视觉发送的值的初始化
	visionSendYaw = &boardImu->Angle.yaw;
	//visionSendPitch = &boardImu->Angle.roll ;
	visionSendPitch = &send_roll_offset ;
}

