#ifndef _SMOOTH_START
#define _SMOOTH_START
#include "board.h"
class SMOOTH
{
	public:
	u8 if_start_smooth = 1; //是否开始执行平滑操作
	float error;
	float value_last;   //设定前的值
	float value_set;    //设定值
	float value_return = 0; //经计算返回的注入值
	
	float angle_change; //角度2ms变化值 1s--0.18 1.5s--0.12
	float angle_now = 0;    //sin角度值
	float smooth_sin(float a, float b);
	float k;                //线性斜率
	float length;           //执行长度
	float smooth_xy(float a, float b);
	float angle_change_cos;
	float angle_now_cos = 0;
	float smooth_cos(float a, float b);
	float smooth_start(float a, float b);
	int times = 0;          //执行次数
};

extern SMOOTH Smooth_1, Smooth_2, Smooth_3, Smooth_4;

#endif