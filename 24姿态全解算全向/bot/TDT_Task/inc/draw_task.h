#ifndef _DRAW_
#define _DRAW_

#include "judgement.h"
#include "board.h"
#include "my_math.h"
/*
*****************************************************************
ͼ��ID0	|ͼ��ID1	|ͼ��ID2									|

************f**************************************
ͼ�β���|ͼ������	|ͼ����	|��ɫ		|��ʼ�Ƕ�	|��ֹ�Ƕ�	|
--------+-----------+-------+-----------+-----------+-----------|
0�ղ���	|0ֱ��		|		|0������ɫ	|			|			|
1����	|1����		|		|1��ɫ		|			|			|
2�޸�	|2��Բ		|	0	|2��ɫ		|	0		|	0		|
3ɾ��	|3��Բ		|	��	|3��ɫ		|	��		|	��		|
		|4Բ��		|	9	|4�Ϻ�ɫ	|	360		|	360		|
		|5������	|		|5��ɫ		|			|			|
		|6������	|		|6��ɫ		| [�����С]| [��Ч����]|
		|7�ַ�		|		|7��ɫ		|			|			|
		|			|		|8��ɫ		|			|			|
*****************************************************************
�߿�	|���x		|���y	|�ֺ�&�뾶	|�յ�x		|�յ�y		|
*****************************************************************

����ԭ�������½�[0,0]
�Ƕ�ԭ�������Ϸ���˳ʱ������
������ģʽΪ��32λint32_tֵ/1000
������ģʽΪ��32λint32_tֵ

*/
void Draw_Task(void);
void myUInumLoad(Judgement::GraphicDataStruct *GraphicDataStruct,int32_t num);
void Chars_Update(Judgement::ClientCustomCharacter Demo_Chars);

/*********************************************************************************/
///<UI�����ƶ����궨��

#define SHOTABLE_X_BASE 963 //���X���׼

#define ENEMYINFO_X_BASE 960 //Ŀ����ϢX���׼
#define ENEMYINFO_Y_BASE 720 //Ŀ����ϢY���׼

#define CAPINFO_X_BASE 550 //������ϢX���׼
#define CAPINFO_Y_BASE 100 //������ϢY���׼

#define ATTITUDE_X_BASE 960 //��̬��X���׼
#define ATTITUDE_Y_BASE 540 //��̬��Y���׼

#define COVER_X_BASE 300 //���ձ�ʶX���׼
#define COVER_Y_BASE 800 //���ձ�ʶY���׼
	   
#define FEED_X_BASE 200 //���̱�ʶX���׼
#define FEED_Y_BASE 630 //���̱�ʶY���׼

#define METER_X_BASE 1300 //�ٶ��Ǳ�X���׼
#define METER_Y_BASE 150 //�ٶ��Ǳ�Y���׼

#define BATTERY_X_BASE 1875 //��ر�ʶX���׼
#define BATTERY_Y_BASE 830 //��ر�ʶY���׼

#define FOLLOW_X_BASE 1150 //�����ʶX���׼
#define FOLLOW_Y_BASE 200 //�����ʶY���׼

#define LEG_X_BASE 1750 //�Ȳ���ʶX���׼
#define LEG_Y_BASE 550 //�Ȳ���ʶY���׼

#define IDLER_X_BASE 1680 //���ֱ�ʶX���׼
#define IDLER_Y_BASE 400 //���ֱ�ʶY���׼

#define HEAD_X_BASE 10 //��̨��ʶX���׼
#define HEAD_Y_BASE 760 //��̨��ʶY���׼

#define FRI_X_BASE HEAD_X_BASE //Ħ���ֱ�ʶX���׼
#define FRI_Y_BASE HEAD_Y_BASE-120 //Ħ���ֱ�ʶY���׼

#define CHASSIS_STATE_X_BASE 800 //����״̬��ʶX���׼
#define CHASSIS_STATE_Y_BASE 850 //����״̬��ʶY���׼

#define TEMP_X_BASE 1000 //�ؽڵ���¶�X���׼
#define TEMP_Y_BASE 800 //�ؽڵ���¶�Y���׼
/*********************************************************************************/

#define _1X1 (uint32_t)(METER_X_BASE+cos((-120+90)*RAD_PER_DEG)*80	)
#define _1Y1 (uint32_t)(METER_Y_BASE+sin((-120+90)*RAD_PER_DEG)*80	)
#define _1X2 (uint32_t)(METER_X_BASE+cos((-120+90)*RAD_PER_DEG)*100	)
#define _1Y2 (uint32_t)(METER_Y_BASE+sin((-120+90)*RAD_PER_DEG)*100	)
#define _2X1 (uint32_t)(METER_X_BASE+cos((  30+90)*RAD_PER_DEG)*80	)
#define _2Y1 (uint32_t)(METER_Y_BASE+sin((  30+90)*RAD_PER_DEG)*80	)
#define _2X2 (uint32_t)(METER_X_BASE+cos((  30+90)*RAD_PER_DEG)*100	)
#define _2Y2 (uint32_t)(METER_Y_BASE+sin((  30+90)*RAD_PER_DEG)*100	)
#define _3X1 (uint32_t)(METER_X_BASE+cos((  60+90)*RAD_PER_DEG)*80	)
#define _3Y1 (uint32_t)(METER_Y_BASE+sin((  60+90)*RAD_PER_DEG)*80	)
#define _3X2 (uint32_t)(METER_X_BASE+cos((  60+90)*RAD_PER_DEG)*100	)
#define _3Y2 (uint32_t)(METER_Y_BASE+sin((  60+90)*RAD_PER_DEG)*100	)
#define _4X1 (uint32_t)(METER_X_BASE+cos((  90+90)*RAD_PER_DEG)*80	)
#define _4Y1 (uint32_t)(METER_Y_BASE+sin((  90+90)*RAD_PER_DEG)*80	)
#define _4X2 (uint32_t)(METER_X_BASE+cos((  90+90)*RAD_PER_DEG)*100	)
#define _4Y2 (uint32_t)(METER_Y_BASE+sin((  90+90)*RAD_PER_DEG)*100	)
#define _5X1 (uint32_t)(METER_X_BASE+cos(( 120+90)*RAD_PER_DEG)*80	)
#define _5Y1 (uint32_t)(METER_Y_BASE+sin(( 120+90)*RAD_PER_DEG)*80	)
#define _5X2 (uint32_t)(METER_X_BASE+cos(( 120+90)*RAD_PER_DEG)*100	)
#define _5Y2 (uint32_t)(METER_Y_BASE+sin(( 120+90)*RAD_PER_DEG)*100	)
#define _6X1 (uint32_t)(METER_X_BASE+cos(( -30+90)*RAD_PER_DEG)*80	)
#define _6Y1 (uint32_t)(METER_Y_BASE+sin(( -30+90)*RAD_PER_DEG)*80	)
#define _6X2 (uint32_t)(METER_X_BASE+cos(( -30+90)*RAD_PER_DEG)*100	)
#define _6Y2 (uint32_t)(METER_Y_BASE+sin(( -30+90)*RAD_PER_DEG)*100	)
#define _7X1 (uint32_t)(METER_X_BASE+cos(( -60+90)*RAD_PER_DEG)*80	)
#define _7Y1 (uint32_t)(METER_Y_BASE+sin(( -60+90)*RAD_PER_DEG)*80	)
#define _7X2 (uint32_t)(METER_X_BASE+cos(( -60+90)*RAD_PER_DEG)*100	)
#define _7Y2 (uint32_t)(METER_Y_BASE+sin(( -60+90)*RAD_PER_DEG)*100	)
#define _8X1 (uint32_t)(METER_X_BASE+cos(( -90+90)*RAD_PER_DEG)*80	)
#define _8Y1 (uint32_t)(METER_Y_BASE+sin(( -90+90)*RAD_PER_DEG)*80	)
#define _8X2 (uint32_t)(METER_X_BASE+cos(( -90+90)*RAD_PER_DEG)*100	)
#define _8Y2 (uint32_t)(METER_Y_BASE+sin(( -90+90)*RAD_PER_DEG)*100	)

#endif


