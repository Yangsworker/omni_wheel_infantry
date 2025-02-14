#ifndef __MPUDATA_H__
#define __MPUDATA_H__

#include "board.h"

#define xx 0
#define yy 1
#define zz 2

#define ANGLE_TO_RAD 0.01745329f

#define TO_ANGLE     0.061036f

#define MPU6050_FILTER_NUM   10


#define A_X    0
#define A_Y    1
#define A_Z    2
#define G_X    3
#define G_Y    4
#define G_Z    5

#define ITEMS  6

typedef struct _accdata{
												vec3int16 origin;  //原始值
												vec3f offset;      //零偏值 
												vec3f calibration; //校准值
												vec3f filter;      //滑动平均滤波值
											}accdata;

typedef struct _gyrodata{
												vec3int16 origin;  //原始值
												vec3f offset_max;  //零偏值最大值
												vec3f offset_min;  //零偏值最小值	
												vec3f offset;      //零偏值 
												vec3f calibration; //校准值
												vec3f filter;      //滑动平均滤波值
												vec3f dps;         //度每秒 
												vec3f radps;       //弧度每秒
											}gyrodata;

typedef struct _mpu{   
											accdata acc;
											gyrodata gyro;
											float yaw_angle;
											float pitch_angle;
									 }mpu;

extern mpu mpu6050Top;

//读取加速度和角速度数据
void Mpu6050Top_Read(void);
//mpu6050数据准备
void Mpu6050Top_Data_Prepare(void);
//陀螺仪零偏矫正
void Mpu6050Top_CalOffset_Gyro(void);
                                     
#endif


