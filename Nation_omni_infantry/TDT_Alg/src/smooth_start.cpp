/*
*Ϊ��ֹ��������ʱ�������ת��ƫ��������
*�˺����������ڴ�0���ٵ�ƽ��
*/

#include "smooth_start.h"
#include "chassis_task.h"

 SMOOTH Smooth_1, Smooth_2, Smooth_3, Smooth_4;
 
 float SMOOTH::smooth_sin(float a,float b) //�������
{
	if(b == 0) {if_start_smooth = 1;} //����趨ֵΪ0�����´ο����ú���
	if(if_start_smooth)
	{
	   angle_change = 0.12;
	   value_last = a;
	   value_set = b;
	   error = value_set - value_last; 
	   angle_now += angle_change / 180 * 3.1415f;
	   value_return = value_last + error*sin(angle_now);
	   times++;
	   if(times > 750) {times = 0;angle_now = 0; if_start_smooth = 0;} //1s֮��ִ�иú���
	   return value_return;		 
  }
	return b;
}

float SMOOTH::smooth_xy(float a, float b) //ֱ�����
{
	if(b == 0) {if_start_smooth = 1;} //����趨ֵΪ0�����´ο����ú���
	if(if_start_smooth)
	{
	   value_last = a;
	   value_set = b;
	   error = value_set - value_last; 
		 length += error / 750;
	   value_return = value_last + length;
	   times++;
	   if(times > 750) {times = 0;length = 0; if_start_smooth = 0;} //1.5s֮��ִ�иú���
	   return value_return;		 
  }
	return b;
}

 float SMOOTH::smooth_cos(float a,float b) //�������
{
	if(b == 0) {if_start_smooth = 1;} //����趨ֵΪ0�����´ο����ú���
	//����©��--�˴������ڷ���
	if(chassis.lastSpeedLevel != 3) {Smooth_1.if_start_smooth = 1; times = 0;angle_now = 0; if_start_smooth = 0;}
	if(if_start_smooth)
	{
	   angle_change_cos = 0.12;
	   value_last = a;
	   value_set = b;
	   error = value_set - value_last; 
	   angle_now_cos += angle_change_cos / 180 * 3.1415f;
	   value_return = value_last + error*(1 - cos(angle_now_cos));
	   times++;
	   if(times > 600) {times = 0;angle_now = 0; if_start_smooth = 0;} //1s֮��ִ�иú���
	   return value_return;		 
  }
	return b;
}

float SMOOTH::smooth_start(float a, float b)
{
	if(b == 0) {if_start_smooth = 1;}
	times++;
	if(if_start_smooth)
	{
		if(times <= 750 && b > 2500)
		{
			return b*0.5;
		}
		else 
		{
			times = 0;
			if_start_smooth = 0;
		}
	}
	return b;
}