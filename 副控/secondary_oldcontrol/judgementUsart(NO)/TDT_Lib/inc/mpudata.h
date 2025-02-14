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
												vec3int16 origin;  //ԭʼֵ
												vec3f offset;      //��ƫֵ 
												vec3f calibration; //У׼ֵ
												vec3f filter;      //����ƽ���˲�ֵ
											}accdata;

typedef struct _gyrodata{
												vec3int16 origin;  //ԭʼֵ
												vec3f offset_max;  //��ƫֵ���ֵ
												vec3f offset_min;  //��ƫֵ��Сֵ	
												vec3f offset;      //��ƫֵ 
												vec3f calibration; //У׼ֵ
												vec3f filter;      //����ƽ���˲�ֵ
												vec3f dps;         //��ÿ�� 
												vec3f radps;       //����ÿ��
											}gyrodata;

typedef struct _mpu{   
											accdata acc;
											gyrodata gyro;
											float yaw_angle;
											float pitch_angle;
									 }mpu;

extern mpu mpu6050Top;

//��ȡ���ٶȺͽ��ٶ�����
void Mpu6050Top_Read(void);
//mpu6050����׼��
void Mpu6050Top_Data_Prepare(void);
//��������ƫ����
void Mpu6050Top_CalOffset_Gyro(void);
                                     
#endif


