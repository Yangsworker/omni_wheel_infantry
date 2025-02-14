/*****************************************************************************
File name: TDT_Task\src\imu_task.h
Description: 陀螺仪姿态解算任务
Author: 肖银河
Version: 1.1.1.191112_alpha
Date: 19.11.12
History: 
	——————————————————————————————————————————————————————————————————————————
	19.11.12 首次完成
	——————————————————————————————————————————————————————————————————————————
*****************************************************************************/
#ifndef __IMU_TASK_H__
#define __IMU_TASK_H__

#include "board.h"
#include "multi_imu.h"

#define ICM20608_def 0
#define ICM20602_def 0
#define MPU6050_def  0
#define Scha634_03_def 0
#define BMI088_def 1
extern MultiImu *boardImu;
extern float *visionSendYaw, *visionSendPitch;

void imuInit(void);
void Imu_Task(void);
#endif
