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
#include "bmi088.h"
#include "TimeMatch.h"
#include "cycle.h"
#include "curve_model.h"
#include "parameter.h"
#include "flash_var.h"
#include "imu.h"
#include "filter.h"

extern TimeSimultaneity imuTimeMatch;

MultiImu *boardImu;

eulerAngle angleForWatch;
accdata accForWatch1;
gyrodata gyroForWatch1;

int16_t gr_ori_roll , gr_ori_pitch , gr_ori_yaw ;//陀螺仪三轴原始数据
float acc_1 , acc_2 , acc_3 ;//三轴角速度
float dps_1 , dps_2 , dps_3 ;//三轴加速度
float bmi088yaw , bmi088pitch , bmi088roll ;//BMI088调试接口
//Lpf2p filter_lpf ;

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
	/*读取*/
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
		
	gr_ori_roll = (gyroForWatch1.origin.data[0]) ;
	gr_ori_pitch = (gyroForWatch1.origin.data[1]) ;
	gr_ori_yaw = (gyroForWatch1.origin.data[2]) ;
	acc_1 = accForWatch1.accValue.data[0] ;
	acc_2 = accForWatch1.accValue.data[1] ;
	acc_3 = accForWatch1.accValue.data[2] ;
	dps_1 = gyroForWatch1.dps.data[0] ;
	dps_2 = gyroForWatch1.dps.data[1] ;
	dps_3 = gyroForWatch1.dps.data[2] ;
	bmi088yaw = boardImu->AHRS_data.Angle.yaw ;
	bmi088pitch = boardImu->AHRS_data.Angle.pitch ;
	bmi088roll = boardImu->AHRS_data.Angle.roll ;
		
}

#include "flash_var.h"
void imuInit()
{
//	filter_lpf.SetCutoffFreq(500 , 50) ;
	u8 currentImuInit = 0;
	boardImu = new MultiImu;
	if(ICM20608_def)
	{
		//仅用icm20608  且已更改轴 已适配
		boardImu->validImu[currentImuInit] = new Icm20608(SPI1, SPI_BaudRatePrescaler_64);
		float gyroScaleFactor[3][3] = {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}};
		boardImu->validImu[currentImuInit]->setGyroScaleFactor(gyroScaleFactor);
		float accScaleFactor[3][3] = {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}};
		boardImu->validImu[currentImuInit]->setAccScaleFactor(accScaleFactor);
		IFlash.link(boardImu->validImu[currentImuInit]->gyro.offset, 2);
		IFlash.link(boardImu->validImu[currentImuInit]->acc.offset, 3);
		currentImuInit++;

	}
	else if(MPU6050_def)
	{
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOA时钟

		boardImu->validImu[currentImuInit] = new Mpu6050(GPIOC, GPIO_Pin_2, GPIO_Pin_1);
		float gyroScaleFactor[3][3] = {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}};
		boardImu->validImu[currentImuInit]->setGyroScaleFactor(gyroScaleFactor);
		float accScaleFactor[3][3] = {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}};
		boardImu->validImu[currentImuInit]->setAccScaleFactor(accScaleFactor);
		IFlash.link(boardImu->validImu[currentImuInit]->gyro.offset, 4);
		IFlash.link(boardImu->validImu[currentImuInit]->acc.offset, 5);
		currentImuInit++;		
	}	
	else if(Scha634_03_def)
	{
		boardImu->validImu[currentImuInit] = new Scha634_03(SPI3,SPI_BaudRatePrescaler_32,CsPin(),{GPIOA, GPIO_Pin_15});
		float gyroScaleFactor[3][3] = {{0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
		boardImu->validImu[currentImuInit]->setGyroScaleFactor(gyroScaleFactor);
		float accScaleFactor[3][3] = {{0.0f, -1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
		boardImu->validImu[currentImuInit]->setAccScaleFactor(accScaleFactor);
		IFlash.link(boardImu->validImu[currentImuInit]->gyro.offset, 6);
		IFlash.link(boardImu->validImu[currentImuInit]->acc.offset, 7);
		currentImuInit++;
	}
	else if(BMI088_def)
	{
		boardImu->validImu[currentImuInit] = new Bmi088(SPI1,SPI_BaudRatePrescaler_64) ;
		float gyroScaleFactor[3][3] = {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}};
		boardImu->validImu[currentImuInit]->setGyroScaleFactor(gyroScaleFactor);
		float accScaleFactor[3][3] = {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}};
		boardImu->validImu[currentImuInit]->setAccScaleFactor(accScaleFactor);
		IFlash.link(boardImu->validImu[currentImuInit]->gyro.offset, 8);
		IFlash.link(boardImu->validImu[currentImuInit]->acc.offset, 9);
		currentImuInit++;
		
	}	

    boardImu->forceAppoint=1;
	boardImu->forceImuUsed = 0;
	boardImu->init();
	delayMs(50);
	boardImu->getOffset();
	boardImu->initalAngle();
	
	boardImu->imu_OK = 1;
     
	//视觉发送的值的初始化
	visionSendYaw = &boardImu->Angle.yaw;
	visionSendPitch = &boardImu->Angle.pitch;
}

