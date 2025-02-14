#ifndef _SMOOTH_START
#define _SMOOTH_START
#include "board.h"
class SMOOTH
{
	public:
	u8 if_start_smooth = 1; //�Ƿ�ʼִ��ƽ������
	float error;
	float value_last;   //�趨ǰ��ֵ
	float value_set;    //�趨ֵ
	float value_return = 0; //�����㷵�ص�ע��ֵ
	
	float angle_change; //�Ƕ�2ms�仯ֵ 1s--0.18 1.5s--0.12
	float angle_now = 0;    //sin�Ƕ�ֵ
	float smooth_sin(float a, float b);
	float k;                //����б��
	float length;           //ִ�г���
	float smooth_xy(float a, float b);
	float angle_change_cos;
	float angle_now_cos = 0;
	float smooth_cos(float a, float b);
	float smooth_start(float a, float b);
	int times = 0;          //ִ�д���
};

extern SMOOTH Smooth_1, Smooth_2, Smooth_3, Smooth_4;

#endif