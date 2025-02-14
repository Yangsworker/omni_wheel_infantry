#ifndef _IMU_H
#define	_IMU_H
#include "board.h"

#define ANGLE_TO_RAD 0.01745329f

#define TO_ANGLE     0.061036f






#define IMU_INTEGRAL_LIM  ( 2.0f *ANGLE_TO_RAD )

#define RtA 	  57.324841f

typedef struct _eulerAngle{
														float pitch;
														float roll;
														float yaw;
													}eulerAngle;


extern eulerAngle gimbalTopAngle;
extern eulerAngle gimbalBotAngle;

void TDT_IMUupdate(float half_T, vec3f* gyro, vec3f* acc);

#endif
