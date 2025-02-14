#ifndef _DRAW_
#define _DRAW_

#include "judgement.h"
#include "board.h"
#include "my_math.h"
/*
*****************************************************************
图形ID0	|图形ID1	|图形ID2									|

************f**************************************
图形操作|图形类型	|图层数	|颜色		|起始角度	|终止角度	|
--------+-----------+-------+-----------+-----------+-----------|
0空操作	|0直线		|		|0红蓝主色	|			|			|
1增加	|1矩形		|		|1黄色		|			|			|
2修改	|2整圆		|	0	|2绿色		|	0		|	0		|
3删除	|3椭圆		|	到	|3橙色		|	到		|	到		|
		|4圆弧		|	9	|4紫红色	|	360		|	360		|
		|5浮点数	|		|5粉色		|			|			|
		|6整型数	|		|6青色		| [字体大小]| [有效长度]|
		|7字符		|		|7黑色		|			|			|
		|			|		|8白色		|			|			|
*****************************************************************
线宽	|起点x		|起点y	|字号&半径	|终点x		|终点y		|
*****************************************************************

坐标原点在左下角[0,0]
角度原点在正上方，顺时针增加
浮点数模式为后32位int32_t值/1000
整形数模式为后32位int32_t值

*/
void Draw_Task(void);
void myUInumLoad(Judgement::GraphicDataStruct *GraphicDataStruct,int32_t num);
void Chars_Update(Judgement::ClientCustomCharacter Demo_Chars);

/*********************************************************************************/
///<UI快速移动坐标定义

#define SHOTABLE_X_BASE 963 //射表X轴基准

#define ENEMYINFO_X_BASE 960 //目标信息X轴基准
#define ENEMYINFO_Y_BASE 720 //目标信息Y轴基准

#define CAPINFO_X_BASE 550 //电容信息X轴基准
#define CAPINFO_Y_BASE 100 //电容信息Y轴基准

#define ATTITUDE_X_BASE 960 //姿态球X轴基准
#define ATTITUDE_Y_BASE 540 //姿态球Y轴基准

#define COVER_X_BASE 300 //弹舱标识X轴基准
#define COVER_Y_BASE 800 //弹舱标识Y轴基准
	   
#define FEED_X_BASE 200 //拨盘标识X轴基准
#define FEED_Y_BASE 630 //拨盘标识Y轴基准

#define METER_X_BASE 1300 //速度仪表X轴基准
#define METER_Y_BASE 150 //速度仪表Y轴基准

#define BATTERY_X_BASE 1875 //电池标识X轴基准
#define BATTERY_Y_BASE 830 //电池标识Y轴基准

#define FOLLOW_X_BASE 1150 //跟随标识X轴基准
#define FOLLOW_Y_BASE 200 //跟随标识Y轴基准

#define LEG_X_BASE 1750 //腿部标识X轴基准
#define LEG_Y_BASE 550 //腿部标识Y轴基准

#define IDLER_X_BASE 1680 //滚轮标识X轴基准
#define IDLER_Y_BASE 400 //滚轮标识Y轴基准

#define HEAD_X_BASE 10 //云台标识X轴基准
#define HEAD_Y_BASE 760 //云台标识Y轴基准

#define FRI_X_BASE HEAD_X_BASE //摩擦轮标识X轴基准
#define FRI_Y_BASE HEAD_Y_BASE-120 //摩擦轮标识Y轴基准

#define CHASSIS_STATE_X_BASE 800 //底盘状态标识X轴基准
#define CHASSIS_STATE_Y_BASE 850 //底盘状态标识Y轴基准

#define TEMP_X_BASE 1000 //关节电机温度X轴基准
#define TEMP_Y_BASE 800 //关节电机温度Y轴基准
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


