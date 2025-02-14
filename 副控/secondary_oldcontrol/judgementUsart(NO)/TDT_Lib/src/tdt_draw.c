#include "tdt_draw.h" 
#include "board.h" 
#include "judgement.h" 
#include "string.h" 
//#include "gy53.h" 
#include "can.h"
#include "mymath.h"

u8 Bool(u8 uin)//u8取反布尔化
{
	if(uin!=0)
		uin =1;
	else
		uin =0;
	
	return uin;
}

u8 UI_InitFlag = 0;//UI正在初始化标识位
u8 schedule1ReDraw = 200, schedule2ReDraw = 0; 
 
#define DISPLAY_WIDTH 1920 
#define DISPLAY_HIGHT 1080 
 
#define COLOR_MODE1 3//橙色 
#define COLOR_MODE2 6//青色 
#define COLOR_MODE3 4//紫色 
#define COLOR_DISABLE 8//白 
u8 colorSet[] = {COLOR_DISABLE, COLOR_MODE1, COLOR_MODE2, COLOR_MODE3}; 
 
#define MIDDLE_X 960 
#define MIDDLE_Y 540 
float customTimeLast = 0; 
typedef struct SendToGuardTask//无用了
	{
		uint8_t commend;
	}sendtoguard_t;
typedef struct GetGuardStatus//无用了
{
	uint8_t Destination_num;//当前目标点编号0-6
	u8 navi_offline;//定位导航离线
	u8 navi_wake;//定位导航开机
}getguardstatus;
getguardstatus guardstatus;
//		15		18		30 
//		[0]左右 [1]上下 [2]长度 
//竖线	[0][0]	[1][0]	[2][0] 
//1m	[0][1]	[1][1]	[2][1] 
//3m	[0][2]	[1][2]	[2][2] 
//5m	[0][3]	[1][3]	[2][3] 
int16_t quasiHeart[3][4][3]= //准心
{ 
	{{1,2},{3,1},{1,2}}, 
	{{1,2},{3,1},{1,2}}, 
	{{1,2},{3,1},{1,2}}, 
}; 
 
union GraphicDataUnion dynamicGraphic[]= 
{ 
	//CPU offline; visionOfflineFlag; normal 左一灯
	{0, 1, 1, 1, 1, 1, 6, 21, 22, 24, 100+14, DISPLAY_HIGHT-200-14, 26, 150-14, DISPLAY_HIGHT-250+14}, 
	//superPowerOffline; superPowerState; Normal 左二灯
	{0, 1, 2, 1, 1, 1, 6, 21, 22, 24, 150+14, DISPLAY_HIGHT-200-14, 26, 200-14, DISPLAY_HIGHT-250+14}, 
	//BoostShoot; ; Normal 左三灯
	{0, 1, 3, 1, 1, 1, 6, 21, 22, 24, 200+14, DISPLAY_HIGHT-200-14, 26, 250-14, DISPLAY_HIGHT-250+14}, 
	//superPowerRemainP 
	{0, 1, 4, 1, 6, 1, 6, 20, 3, 3, MIDDLE_X+360, DISPLAY_HIGHT-970, 0.0f}, 
	//lastBullet 
	{0,4,2,1,6,2,8,50,2,1,DISPLAY_WIDTH/2+100,DISPLAY_HIGHT/2-50,200}, 
	//steering wheel head头部相对正方向的位置 
	{0,4,1,1,0,2,8,11,12,5,DISPLAY_WIDTH/2,200,16,DISPLAY_WIDTH/2,240}, 
	//visionMode 左四灯
	{0, 1, 5, 1, 1, 1, 6, 21, 22, 24, 250+14, DISPLAY_HIGHT-200-14, 26, 300-14, DISPLAY_HIGHT-250+14},	 
}; 
 
union GraphicDataUnion dynamicGraphic2[]= 
{ 
	//powerBar_Normal 
	{0,3,4,1,1,2,0,21,22,10,DISPLAY_WIDTH/2-350,100,26,DISPLAY_WIDTH/2+350,100}, 
	//backup powerBar 
	{0,3,2,1,1,1,8,21,22,10,DISPLAY_WIDTH/2-350,100,26,DISPLAY_WIDTH/2-350+200,100}, 
	//power down limit	 
	{0,3,3,1,1,0,2,21,22,3,DISPLAY_WIDTH/2-350+300,106,26,DISPLAY_WIDTH/2-350+300,94}, 
	//frictionSpeed ->GuardMode回到摩擦轮速获取
	{0, 1, 6, 1, 6, 1, 3, 30, 1, 3, 1500, DISPLAY_HIGHT - 510, 0.0f}, 
	//pitch 
	{0, 1, 7, 1, 6, 1, 6, 50, 1, 3, 50, DISPLAY_HIGHT - 300, 0.0f}, 
	//敌方ID
	{0,3,5,1,6,1,1,30,1,3,MIDDLE_X-250,MIDDLE_Y,0.0f},
	//弹仓盖灯
	{0, 3, 20, 1, 6, 1, 8, 50, NULL, 3,MIDDLE_X-650,DISPLAY_HIGHT - 200,0},
//	{0,3,20,1,4,1,8,90,270,5,MIDDLE_X-650,DISPLAY_HIGHT - 200,10,10,10}//后面俩参数没看啥意思
//	//GY53 LF Distance 
//	{0, 1, 11, 1, 6, 1, 8, 10, 1, 1, DISPLAY_WIDTH/2+50, 200, 0.0f}, 
}; 
 
union GraphicDataUnion dynamicGraphic3[]= 
{ 
	//motor1Lost 
	{0,5,0,1,4,2,8,0,90,5,DISPLAY_WIDTH/2,200,40,40,40}, 
	//motor2Lost 
	{0,5,1,1,4,2,8,270,359,5,DISPLAY_WIDTH/2,200,40,40,40}, 
	//motor3Lost 
	{0,5,2,1,4,2,8,180,270,5,DISPLAY_WIDTH/2,200,40,40,40}, 
	//motor4Lost	 
	{0,5,3,1,4,2,8,90,180,5,DISPLAY_WIDTH/2,200,40,40,40}, 
	//lock obj 
	{0, 1, 10, 1, 1, 1, 6, 21, 22, 6, MIDDLE_X+100, MIDDLE_Y+100, 13, MIDDLE_X+100+50, MIDDLE_Y+100+50}, 
	//beat obj 
	{0, 1, 9, 1, 1, 1, 3, 21, 22, 6, MIDDLE_X+100+18, MIDDLE_Y+100+18, 26, MIDDLE_X+100+50-18, MIDDLE_Y+100+50-18}, 
//	//GY53 LB Distance 
//	{0, 1, 12, 1, 6, 1, 8, 10, 1, 1, DISPLAY_WIDTH/2+50, 170, 0.0f}, 
	//visionMode 左四灯下添加的反建筑,仅在反建筑时出现
	{0, 1, 12, 1, 6, 1, 2, 10, 1, 5, 275, DISPLAY_HIGHT-270 , 0.0f},			
}; 
 
union GraphicDataUnion dynamicGraphic4[]= 
{ 
	//doubleShift 
	{0,6,2,0,0,4,8,0,0,2,MIDDLE_X,350-50,0,MIDDLE_X-20,315-50}, 
	{0,6,3,0,0,4,8,0,0,2,MIDDLE_X,350-50,0,MIDDLE_X+20,315-50}, 
	{0,6,4,0,0,4,8,0,0,2,MIDDLE_X,350+20-50,0,MIDDLE_X-20,315+20-50}, 
	{0,6,5,0,0,4,8,0,0,2,MIDDLE_X,350+20-50,0,MIDDLE_X+20,315+20-50}, 
	//本地热量条 
	{0,5,4,1,4,2,8,0,360,5,MIDDLE_X,MIDDLE_Y,40,65,65}, 
	//敌方血量条
	{0,1,20,1,4,5,2,0,359,5,MIDDLE_X-250,MIDDLE_Y,40,35,35},
	//哨兵模式ID
//	{0, 1, 19, 1, 6, 1, 8, 50, NULL, 3,MIDDLE_X-12,MIDDLE_Y+100,0},	
	
//	//GY53 FF Distance 
//	{0, 1, 8, 1, 6, 1, 8, 10, 1, 1, DISPLAY_WIDTH/2+50, 230, 0.0f}, 


}; 
 
//union GraphicDataUnion ammoCoverOpenLine[6]= 
//{ 
//	//弹仓盖*6，120° 
////	{0,7,0,0,0,4,8,0,0,1,MIDDLE_X,MIDDLE_Y,0,MIDDLE_X,MIDDLE_Y-100}, 
////	{0,7,1,0,0,4,8,0,0,1,MIDDLE_X,MIDDLE_Y-100,0,MIDDLE_X+87,MIDDLE_Y-150}, 
////	{0,7,2,0,0,4,8,0,0,1,MIDDLE_X+87,MIDDLE_Y-150,0,MIDDLE_X+173,MIDDLE_Y-100}, 
////	{0,7,3,0,0,4,8,0,0,1,MIDDLE_X+173,MIDDLE_Y-100,0,MIDDLE_X+173,MIDDLE_Y}, 
////	{0,7,4,0,0,4,8,0,0,1,MIDDLE_X+173,MIDDLE_Y,0,MIDDLE_X+87,MIDDLE_Y+50}, 
////	{0,7,5,0,0,4,8,0,0,1,MIDDLE_X+87,MIDDLE_Y+50,0,MIDDLE_X,MIDDLE_Y}, 
//	 
//	//0° 
////	{0, 7, 0, 1, 0, 1, 8, 0, 0, 1,MIDDLE_X,MIDDLE_Y,0,MIDDLE_X+50,MIDDLE_Y+87}, 
////	{0, 7, 1, 1, 0, 1, 8, 0, 0, 1,MIDDLE_X+50,MIDDLE_Y+87,0,MIDDLE_X,MIDDLE_Y+173}, 
////	{0, 7, 2, 1, 0, 1, 8, 0, 0, 1,MIDDLE_X,MIDDLE_Y+173,0,MIDDLE_X-100,MIDDLE_Y+173}, 
////	{0, 7, 3, 1, 0, 1, 8, 0, 0, 1,MIDDLE_X-100,MIDDLE_Y+173,0,MIDDLE_X-150,MIDDLE_Y+87}, 
////	{0, 7, 4, 1, 0, 1, 8, 0, 0, 1,MIDDLE_X-150,MIDDLE_Y+87,0,MIDDLE_X-100,MIDDLE_Y}, 
////	{0, 7, 5, 1, 0, 1, 8, 0, 0, 1,MIDDLE_X-100,MIDDLE_Y,0,MIDDLE_X,MIDDLE_Y}, 
//}; 
 
union GraphicDataUnion temperatureShow[6]= 
{ 
	//云台温度*2 + 底盘最高电机温度 
	//pitch 
	{0, 7, 0, 1, 6, 1, 6, 20, NULL, 3, 50, DISPLAY_HIGHT - 375, 100}, 
	//yaw 
	{0, 7, 1, 1, 6, 1, 6, 20, NULL, 3, 50, DISPLAY_HIGHT - 400, 200}, //没必要
	//deleteAmmoCover 
//	{0, 7, 2, 2, 0, 1, 8, 0, 0, 1,MIDDLE_X-100,MIDDLE_Y+173,0,MIDDLE_X-100,MIDDLE_Y+173}, 
//	{0, 7, 3, 2, 0, 1, 8, 0, 0, 1,MIDDLE_X-100,MIDDLE_Y+173,0,MIDDLE_X-100,MIDDLE_Y+173}, 
//	{0, 7, 4, 2, 0, 1, 8, 0, 0, 1,MIDDLE_X-100,MIDDLE_Y+173,0,MIDDLE_X-100,MIDDLE_Y+173}, 
//	{0, 7, 5, 2, 0, 1, 8, 0, 0, 1,MIDDLE_X-100,MIDDLE_Y+173,0,MIDDLE_X-100,MIDDLE_Y+173}, 
}; 
 
union GraphicDataUnion blockErrorGraphic= 
{ 
	0, 7, 6, JGMT_OPERATER_DELETE, JGMT_GRAPHIC_CIRCLE, 1, JGMT_COLOR_BLACK, 0, 0, 10, MIDDLE_X, MIDDLE_Y, 85  
}; 
 
 
union GraphicDataUnion dynamicGraphic5[7];//给堵转检测了

union GraphicDataUnion dynamicGraphic6[]=
{
	//视觉装甲偏置数值
	{0,1,19,1,6,1,8,10,NULL,3,DISPLAY_WIDTH/2+350,DISPLAY_HIGHT-350,0},//YAW
	{0,1,21,1,6,1,8,10,NULL,3,DISPLAY_WIDTH/2+400,DISPLAY_HIGHT-400,0},//PITCH
	//打符
	
	{0,1,23,1,6,1,6,10,NULL,3,DISPLAY_WIDTH/2-400,DISPLAY_HIGHT-400,0},//PITCH
	{0,1,22,1,6,1,6,10,NULL,3,DISPLAY_WIDTH/2-350,DISPLAY_HIGHT-350,0},//YAW
};
 

 
union GraphicDataUnion constGraphic[]= 
{ 
	//tipWidth 
	{0, 2, 1, 1, 1, 2, 7, 21, 22, 3, 100, DISPLAY_HIGHT-200, 26, 100+200, DISPLAY_HIGHT-250}, 
	//default powerBar 
	{0,3,1,1,1,2,7,21,22,1,DISPLAY_WIDTH/2-350,106,26,DISPLAY_WIDTH/2+350,94}, 
	//todo vision range 
 

	//敌方血量白条
	{0,2,5,1,4,1,8,0,360,3,MIDDLE_X-250,MIDDLE_Y,20,25,25},	
	{0,2,2,1,4,1,7,118,122,6,MIDDLE_X-250,MIDDLE_Y,20,27,27},	
	{0,2,3,1,4,1,7,238,242,6,MIDDLE_X-250,MIDDLE_Y,20,27,27},	
  //视觉范围
//	{0,2,6,1,6,5,8,450,NULL,1,MIDDLE_X-450/**/,DISPLAY_HIGHT-420,100},//1
//	{0,2,7,1,6,5,8,450,NULL,1,MIDDLE_X+50/**/,DISPLAY_HIGHT-420,100},//7
	{0,2,6,1,1,8,8,0,360,1,DISPLAY_WIDTH/2-350,200,26,DISPLAY_WIDTH/2+350,DISPLAY_HIGHT-400}, //待调整
}; 


#define calibrationX (MIDDLE_X+7)//Cake: -3;  BigHead: +7;
//#define L2for15_X (calibrationX-25)
//#define L3for15_X (calibrationX-20)
//#define L4for15_X (calibrationX-13)
//#define L45for15_X (calibrationX-13)
//#define L5for15_X (calibrationX-11)
//#define L55for15_X (calibrationX-10)
//#define L6for15_X (calibrationX-9)
#define L2for15_X (calibrationX-15)
#define L3for15_X (calibrationX-15)
#define L4for15_X (calibrationX-15)
#define L45for15_X (calibrationX-15)
#define L5for15_X (calibrationX-15)
#define L55for15_X (calibrationX-15)
#define L6for15_X (calibrationX-15)

union GraphicDataUnion Auxiliary[] = { //射表
	{0, 8, 0, 1, 0, 2, 2, 0, 0, 1, calibrationX, 500, 0, calibrationX, 200},//偏右

	{0, 8, 1, 1, 0, 2, 2, 0, 0, 2, L2for15_X, 500-37.5*1, 0, calibrationX*2-L2for15_X, 500-37.5*1}, //2m激光限-随弹速调整
	{0, 8, 3, 1, 0, 2, 3, 0, 0, 1, L3for15_X, 500-37.5*2, 0, calibrationX*2-L3for15_X, 500-37.5*2}, //3m-15mps
//	{0, 8, 4, 1, 0, 2, 2, 0, 0, 1, 950, 413, 0, 990, 413}, //3.5m-15mps
	{0, 8, 5, 1, 0, 2, 6, 0, 0, 1, L4for15_X, 500-37.5*3, 0, calibrationX*2-L4for15_X, 500-37.5*3}, //4m-15mps
	
	{0, 8, 6, 1, 0, 2, 6, 0, 0, 1, L45for15_X, 500-37.5*4, 0, calibrationX*2-L45for15_X, 500-37.5*4}, //4.5m-15mps 
	{0, 8, 2, 1, 0, 2, 2, 0, 0, 1, L55for15_X, 500-37.5*5, 0, calibrationX*2-L55for15_X, 500-37.5*5}, //5.5m-15mps	
};  

union GraphicDataUnion Auxiliary1[] = { //射表

	{0, 8, 10, 1, 0, 2, 5, 0, 0, 2, L5for15_X, 500-37.5*6, 0, calibrationX*2-L5for15_X, 500-37.5*6}, //5m-15mps & 近战下限
	{0, 8, 11, 1, 0, 2, 6, 0, 0, 1, L6for15_X, 500-37.5*7, 0, calibrationX*2-L6for15_X, 500-37.5*7}, //6m-15mps 

};   


 
u8 speedToIndex() //射速
{ 
	if(jgmt_mesg.gameRobotStatus.shooterId1_17mmSpeedLimit == 18) 
		return 1; 
	 
	if(jgmt_mesg.gameRobotStatus.shooterId1_17mmSpeedLimit == 30) 
		return 2; 
 
	return 0; 
} 
float EnemyHPPercent =0.5;
float EnemyHPMax;
u8 ifFinishJudge =0;
u16 HpArray[6];
u16 HpArrayMax[6];

int16_t customUIBulletTest = 0; 
extern u8 gimbalOffline, superPowerOffline; 
//void AuxiliaryUpdate()
//{
//	if(canStateStruct.AuxiliaryCarlibration<=3)
//	{
//		switch(canStateStruct.AuxiliaryCarlibration)
//		{
//			case 0:
//				for(u8 i;i<7;i++)
//				{Auxiliary[i].GraphicData.startY+=10;Auxiliary[i].GraphicData.endY+=10;}
//				for(u8 i;i<2;i++)
//				{Auxiliary1[i].GraphicData.startY+=10;Auxiliary1[i].GraphicData.endY+=10;}
//				break;
//			case 1:
//				for(u8 i;i<7;i++)
//				{Auxiliary[i].GraphicData.startX+=10;Auxiliary[i].GraphicData.endX+=10;}
//				for(u8 i;i<2;i++)
//				{Auxiliary1[i].GraphicData.startX+=10;Auxiliary1[i].GraphicData.endX+=10;}
//				break;
//			case 2:
//				for(u8 i;i<7;i++)
//				{Auxiliary[i].GraphicData.startY-=10;Auxiliary[i].GraphicData.endY-=10;}
//				for(u8 i;i<2;i++)
//				{Auxiliary1[i].GraphicData.startY-=10;Auxiliary1[i].GraphicData.endY-=10;}
//				break;
//			case 3:
//				for(u8 i;i<7;i++)
//				{Auxiliary[i].GraphicData.startX-=10;Auxiliary[i].GraphicData.endX-=10;}
//				for(u8 i;i<2;i++)
//				{Auxiliary1[i].GraphicData.startX-=10;Auxiliary1[i].GraphicData.endX-=10;}
//				break;
//		}
//		for(u8 i;i<7;i++)
//		{Auxiliary[i].GraphicData.operateType=2;}
//		for(u8 i;i<2;i++)
//		{Auxiliary1[i].GraphicData.operateType=2;}
//	
//	}
//	else
//	{
//		for(u8 i;i<7;i++)
//		{Auxiliary[i].GraphicData.operateType=0;}
//		for(u8 i;i<2;i++)
//		{Auxiliary1[i].GraphicData.operateType=0;}	
//	
//	}

//}
void clientUpdataData() 
{ 
	if(gimbalOffline) //云台
	{ 
		dynamicGraphic[0].GraphicData.color = COLOR_MODE1; 
	} 
	else if(canStateStruct.visionOffline)//为1则视觉离线 
	{ 
		dynamicGraphic[0].GraphicData.color = COLOR_MODE2; 
	} 
	else 
	{ 
		dynamicGraphic[0].GraphicData.color = COLOR_DISABLE; 
	} 
	 
//	if(superPowerOffline)//为1则功率离线 
//	{ 
//		dynamicGraphic[1].GraphicData.color = COLOR_MODE1; //橙
//	} 
//	else
//	{
		switch(canStateStruct.frictionSpeed_State)//摩擦轮状态
		{
			case 0:
				dynamicGraphic[1].GraphicData.color = 7;//黑 
				break;
			case 1:
				dynamicGraphic[1].GraphicData.color = 1; //黄
				break;
			case 2:
				dynamicGraphic[1].GraphicData.color = 8;//白
				break;
		}
//	}
		
	
	
	/* if(canStateStruct.ULTSMode) */
//	{ 
//		dynamicGraphic[1].GraphicData.color = COLOR_MODE2; 
//	} 
//	else 
//	{ 
//		dynamicGraphic[1].GraphicData.color = COLOR_DISABLE; 
//	} 
	 
	if(canStateStruct.fireFrequencyMode) //摩擦轮开环
	{ 
		switch(canStateStruct.fireFrequencyMode)
		{
			case 0:
				break;
			case 1:
				dynamicGraphic[2].GraphicData.color = COLOR_MODE2; //青
				break;
			case 2:
				dynamicGraphic[2].GraphicData.color = COLOR_MODE1; //橙
				break;
		}
						 
	} 
	else if(canStateStruct.unLimitedFired)//解除热量限制
	{ 
		dynamicGraphic[2].GraphicData.color = COLOR_MODE3;    //紫
	} 
	else 
	{ 
		dynamicGraphic[2].GraphicData.color = COLOR_DISABLE;  //白
	} 
 
	dynamicGraphic[3].intData.data = powerRecvMsg.capacitance_percentage;//*1000; 
	if(superPowerOffline) 
	{ 
		dynamicGraphic[3].intData.color = JGMT_COLOR_ORANGE; 
		if(dynamicGraphic[3].intData.operateType == 2 || dynamicGraphic[3].intData.operateType == 3) 
			dynamicGraphic[3].intData.operateType = 1; 
		else 
			dynamicGraphic[3].intData.operateType = 3; 
	} 
	else 
	{ 
		dynamicGraphic[3].intData.color = JGMT_COLOR_CYAN; 
	} 
	 
	if(canStateStruct.sprocketMotorOffline) //拨弹轮离线 
	{ 
		customUIBulletTest = 0; 
	} 
 
	dynamicGraphic[4].intData.data = (customUIBulletTest); 
 
	if(dynamicGraphic[4].intData.data < 10) 
	{ 
		dynamicGraphic[4].intData.width = 5; 
		if(canStateStruct.sprocketMotorOffline) 
			dynamicGraphic[4].intData.color = JGMT_COLOR_WHITE;			 
		else 
			dynamicGraphic[4].intData.color = JGMT_COLOR_ORANGE; 
		if(dynamicGraphic[4].intData.operateType == 2 || dynamicGraphic[4].intData.operateType == 3) 
			dynamicGraphic[4].intData.operateType = 1; 
		else 
			dynamicGraphic[4].intData.operateType = 3; 
	} 
	else 
	{ 
		dynamicGraphic[4].intData.width = 1; 
		if(dynamicGraphic[4].intData.operateType == 3) 
			dynamicGraphic[4].intData.operateType = 1; 
		else 
			dynamicGraphic[4].intData.operateType = 2; 
		 
		 
		dynamicGraphic[4].intData.color = JGMT_COLOR_CYAN; 
	} 
 
 
	dynamicGraphic[5].GraphicData.endX = dynamicGraphic[5].GraphicData.startX + 40*sin(((int8_t)canStateStruct.chassisAngle)/256.0f*6.28f); 
	dynamicGraphic[5].GraphicData.endY = dynamicGraphic[5].GraphicData.startY + 40*cos(((int8_t)canStateStruct.chassisAngle)/256.0f*6.28f); 
	dynamicGraphic[5].GraphicData.color = colorSet[canStateStruct.chassisMode]; 
	if(canStateStruct.chassisMode == 1 && canStateStruct.flexRotate) 
	{ 
		dynamicGraphic[5].GraphicData.color = JGMT_COLOR_GREEN; 
	} 
	 
	if(canStateStruct.VisionMode == 0)//普通视觉 
	{ 
		dynamicGraphic[6].GraphicData.color = COLOR_DISABLE; 
	} 
	else if(canStateStruct.VisionMode == 1)//打大符 
	{ 
		dynamicGraphic[6].GraphicData.color = COLOR_MODE1; //橙
	} 
	else if(canStateStruct.VisionMode == 2)//打小陀螺 
	{ 
		dynamicGraphic[6].GraphicData.color = COLOR_MODE2; //青色
	} 
	else if(canStateStruct.VisionMode == 3)//打小符 
	{ 
		dynamicGraphic[6].GraphicData.color = JGMT_COLOR_GREEN;//绿色 
	} 
	else if(canStateStruct.VisionMode == 4) //反符
	{
		dynamicGraphic[6].GraphicData.color = 5; //粉色
	}	
//	if(canStateStruct.commandguardFlag ==1)//放在语句末尾
//		dynamicGraphic[6].GraphicData.color = 4;
} 
 
void clientUpdataData2() 
{ 
	dynamicGraphic2[0].GraphicData.endX = dynamicGraphic2[0].GraphicData.startX + (powerRecvMsg.capacitance_percentage-30)*10; 
	 
	dynamicGraphic2[1].GraphicData.endX = dynamicGraphic2[0].GraphicData.startX + (50-30)*10; 
	if(canStateStruct.powerDownLimit-30 == 0) 
	{ 
		dynamicGraphic2[1].GraphicData.operateType = JGMT_OPERATER_DELETE; 
	} 
	else 
	{ 
		if(dynamicGraphic2[1].GraphicData.operateType == JGMT_OPERATER_DELETE) 
			dynamicGraphic2[1].GraphicData.operateType = 1; 
	} 
	 
	dynamicGraphic2[2].GraphicData.startX = dynamicGraphic2[0].GraphicData.startX + (canStateStruct.powerDownLimit-30)*10; 
	dynamicGraphic2[2].GraphicData.endX = dynamicGraphic2[2].GraphicData.startX; 
//		dynamicGraphic2[3].GraphicData.operateType = 2;
//		dynamicGraphic2[3].intData.data = guardstatus.Destination_num;//哨兵目标地
//	if(!guardstatus.navi_wake)
//	{
//		dynamicGraphic2[3].GraphicData.color = 6;	//没开导航青色
//	}
//	else
//	{
//		if(!guardstatus.navi_offline)
//		{dynamicGraphic2[3].GraphicData.color = 8;}	//正常白色
//		else
//		{dynamicGraphic2[3].GraphicData.color = 4;}//开导航但掉线紫红色
//	}

	//+(canStateStruct.frictionSpdA)*100+canStateStruct.frictionSpdB; //原为摩擦轮速
//	if(canStateStruct.frictionSpdA < 10 || canStateStruct.frictionSpdB < 10 || canStateStruct.frictionOffline || canStateStruct.forceOpenloop) 
//	{ 
//		dynamicGraphic2[3].intData.width = 5; 
//		dynamicGraphic2[3].intData.color = JGMT_COLOR_ORANGE; 
//		if(dynamicGraphic2[3].intData.operateType == 2 || dynamicGraphic2[3].intData.operateType == 3) 
//			dynamicGraphic2[3].intData.operateType = 1; 
//		else 
//			dynamicGraphic2[3].intData.operateType = 3; 
//	} 
//	else 
//	{ 
//		dynamicGraphic2[3].intData.width = 1; 
//		if(dynamicGraphic2[3].intData.operateType == 3) 
//			dynamicGraphic2[3].intData.operateType = 1; 
//		else 
//			dynamicGraphic2[3].intData.operateType = 2; 
//		dynamicGraphic2[3].intData.color = JGMT_COLOR_CYAN; 
//	} 
		 
	dynamicGraphic2[4].intData.data = canStateStruct.elevationAngle; 
	dynamicGraphic2[4].intData.color = JGMT_COLOR_CYAN; 
	 
//	dynamicGraphic2[5].intData.data = can1Feedback.gimbalPos[1]; //pitch轴
//	dynamicGraphic2[5].intData.color = JGMT_COLOR_CYAN; 

	dynamicGraphic2[5].intData.data  =canStateStruct.enemyID;
//	dynamicGraphic2[6].intData.data = GYdistance[LF]; 
 	if(canStateStruct.ammoCoverOpen) 
	{ 
		dynamicGraphic2[6].GraphicData.operateType=2;
		dynamicGraphic2[6].GraphicData.color=3;
		dynamicGraphic2[6].GraphicData.endAngle=270;
	}
	else
	{
		dynamicGraphic2[6].GraphicData.operateType=2;
		dynamicGraphic2[6].GraphicData.color=8;		
		dynamicGraphic2[6].GraphicData.endAngle=90;
	}
} 
 
 
void clientUpdataData3() 
{ 
	for(int i = 0; i < 4; i++) 
	{ 
		if(++can1Feedback.chassisOfflineCheck[i]>1) 
		{ 
			can1Feedback.chassisOffline[i]=1;   //底盘离线
			if(dynamicGraphic3[i].GraphicData.color == JGMT_COLOR_ORANGE) 
			{ 
				dynamicGraphic3[i].GraphicData.color = JGMT_COLOR_CYAN; 
			} 
			else 
			{ 
				dynamicGraphic3[i].GraphicData.color = JGMT_COLOR_ORANGE; 
			} 
		} 
		else 
		{ 
			if(can1Feedback.chassisTemperature[i] > 50) 
				dynamicGraphic3[i].GraphicData.color = JGMT_COLOR_BLACK; 
			else 
				dynamicGraphic3[i].GraphicData.color = JGMT_COLOR_WHITE; 
		} 
	} 
	 
	if(canStateStruct.visionLock) 
	{ 
		dynamicGraphic3[4].GraphicData.operateType = JGMT_OPERATER_NEW; 
	} 
	else 
	{ 
		dynamicGraphic3[4].GraphicData.operateType = JGMT_OPERATER_DELETE; 
	} 
	 
	if(canStateStruct.visionLock && canStateStruct.visionBeat) 
	{ 
		dynamicGraphic3[5].GraphicData.operateType = JGMT_OPERATER_NEW; 
	} 
	else 
	{ 
		dynamicGraphic3[5].GraphicData.operateType = JGMT_OPERATER_DELETE; 
	} 
	if(canStateStruct.Anti_Building_Flag==1 &&(dynamicGraphic3[6].GraphicData.operateType == 3))
	{
		dynamicGraphic3[6].GraphicData.operateType = 1;
	}
	else
	{
		dynamicGraphic3[6].GraphicData.operateType = 3;
	}
//	dynamicGraphic3[6].intData.data = GYdistance[LB]; 
} 
  
float localHeatPercent = 0; 
uint16_t customHeatLimit=0; 
int myColor =0;
void getHpPercent()
{
  //鉴定敌方血量
	if(jgmt_mesg.gameStatus.gameProgress<4)
		ifFinishJudge=0;//重置判断完成标志位
	if(jgmt_mesg.gameStatus.gameProgress==4)
	{
		myColor = jgmt_mesg.gameRobotStatus.robotId;
			if(myColor < 8)
		{
			HpArray[0]=jgmt_mesg.gameRobotHP.singleHp.blue_1RobotHp;
			HpArray[1]=jgmt_mesg.gameRobotHP.singleHp.blue_2RobotHp;
			HpArray[2]=jgmt_mesg.gameRobotHP.singleHp.blue_3RobotHp;
			HpArray[3]=jgmt_mesg.gameRobotHP.singleHp.blue_4RobotHp;
			HpArray[4]=jgmt_mesg.gameRobotHP.singleHp.blue_5RobotHp;
			HpArray[5]=600;//哨兵
		}
		else
		{
			HpArray[0]=jgmt_mesg.gameRobotHP.singleHp.red_1RobotHp;
			HpArray[1]=jgmt_mesg.gameRobotHP.singleHp.red_2RobotHp;
			HpArray[2]=jgmt_mesg.gameRobotHP.singleHp.red_3RobotHp;
			HpArray[3]=jgmt_mesg.gameRobotHP.singleHp.red_4RobotHp;
			HpArray[4]=jgmt_mesg.gameRobotHP.singleHp.red_5RobotHp;
			HpArray[5]=600;//哨兵
		}
	
		if(ifFinishJudge!=1)
		{
		//数值获取
		switch(HpArray[0])
		{
			case 200:
				HpArrayMax[0]=300;
				break;
			case 250:
				HpArrayMax[0]=450;
				break;
			default:
				HpArrayMax[0]=450;
				break;
		}
			switch(HpArray[1])
		{
			case 200:
				HpArrayMax[1]=300;
				break;
			case 250:
				HpArrayMax[1]=450;
				break;
			default:
				HpArrayMax[1]=0;//留工程
			break;
		}
		for(u8 i=2;i<=4;i++)
		{
			switch(HpArray[i])
			{
				case 150:
					HpArrayMax[i]=250;
					break;
				case 200:
					HpArrayMax[i]=400;
					break;
				case 300:
					HpArrayMax[i]=500;
					break;
				default:
					HpArrayMax[i]=500;
					break;
			}
		}
		ifFinishJudge =1;
		}
	}
	EnemyHPPercent = (float)(HpArray[canStateStruct.enemyID-1]) / (float)(HpArrayMax[canStateStruct.enemyID-1]) * 1.0f;
	EnemyHPPercent=LIMIT(EnemyHPPercent,0.01,0.99);
}
u8 LastSentryCtrl=0;
void clientUpdataData4() 
{ 
	if(canStateStruct.doubleShift == 0) 
	{ 
		dynamicGraphic4[0].GraphicData.operateType = 3; 
		dynamicGraphic4[1].GraphicData.operateType = 3; 
		dynamicGraphic4[2].GraphicData.operateType = 3; 
		dynamicGraphic4[3].GraphicData.operateType = 3; 
	} 
	else if(canStateStruct.doubleShift == 1) 
	{ 
		dynamicGraphic4[0].GraphicData.operateType = 1; 
		dynamicGraphic4[1].GraphicData.operateType = 1; 
		dynamicGraphic4[2].GraphicData.operateType = 3; 
		dynamicGraphic4[3].GraphicData.operateType = 3; 
	} 
	else if(canStateStruct.doubleShift == 2) 
	{ 
		dynamicGraphic4[0].GraphicData.operateType = 1; 
		dynamicGraphic4[1].GraphicData.operateType = 1; 
		dynamicGraphic4[2].GraphicData.operateType = 1; 
		dynamicGraphic4[3].GraphicData.operateType = 1; 
	} 
	//当前热量
	localHeatPercent = (float)canStateStruct.localHeat/(jgmt_mesg.gameRobotStatus.shooterId1_17mmCoolingLimit + customHeatLimit); 
	 
	dynamicGraphic4[4].GraphicData.startAngle = (1-localHeatPercent)*360; //起始角计算
	if(dynamicGraphic4[4].GraphicData.startAngle == 360)
		dynamicGraphic4[4].GraphicData.startAngle = 359;  //保证有UI
	 
	dynamicGraphic4[4].GraphicData.color = JGMT_COLOR_WHITE; 
	 
	if(localHeatPercent > 0.5) 
		dynamicGraphic4[4].GraphicData.color = JGMT_COLOR_YELLOW;	 
	if(localHeatPercent > 0.75) 
		dynamicGraphic4[4].GraphicData.color = JGMT_COLOR_ORANGE; 
	 
//	if(canStateStruct.heatChoose == 1) 
//	{ 
//		dynamicGraphic4[4].GraphicData.color = JGMT_COLOR_BLACK; 
//	} 
//	//GY动态数据
//	dynamicGraphic4[5].intData.data = GYdistance[FF]; 
	

	getHpPercent();
	dynamicGraphic4[5].GraphicData.operateType =2;
 	dynamicGraphic4[5].GraphicData.startAngle =(1-EnemyHPPercent)*360;
	
	
} 

u8 blockErrorTimer = 0; 
void blockErrorUpdate() 
{ 
	if(canStateStruct.blockError) 
	{ 
		blockErrorTimer = 2; 
	} 
	 
	if(blockErrorTimer > 0 && blockErrorGraphic.GraphicData.operateType == JGMT_OPERATER_DELETE) 
	{ 
		--blockErrorTimer; 
		blockErrorGraphic.GraphicData.operateType = JGMT_OPERATER_NEW; 
	} 
	else 
	{ 
		blockErrorGraphic.GraphicData.operateType = JGMT_OPERATER_DELETE; 
	} 
} 
 
//float ammoCoverOpenAngle=150;//开盖角度 
//float ammoCoverNowAngle=150;//当前角度 
//float openTime = 1;//开盖时间 
#define AMMO_COVER_CYCLE_INDEX 0 
void clientUpdataData5() 
{ 
	blockErrorUpdate(); 
	memcpy(&dynamicGraphic5[6], &blockErrorGraphic, sizeof(blockErrorGraphic)); 
	 
//	if(canStateStruct.ammoCoverOpen) 
//	{ 
//		ammoCoverNowAngle+=ammoCoverOpenAngle / openTime*Get_Cycle_T(AMMO_COVER_CYCLE_INDEX); 
//		ammoCoverNowAngle = MIN(ammoCoverNowAngle,ammoCoverOpenAngle); 
//	} 
//	else 
//	{ 
//		ammoCoverNowAngle-=ammoCoverOpenAngle / openTime*Get_Cycle_T(AMMO_COVER_CYCLE_INDEX); 
//		ammoCoverNowAngle = MAX(ammoCoverNowAngle,0); 
//	} 
//	 
//	if(ammoCoverNowAngle == 0) 
//	{		 
//		temperatureShow[0].intData.data = can1Feedback.gimbalTemperature[0]; 
////		temperatureShow[1].intData.data = can1Feedback.gimbalTemperature[1]; 
//		 
//		memcpy(dynamicGraphic5, temperatureShow, sizeof(temperatureShow)); 
//		if(schedule1ReDraw > 200) 
//		{ 
//			//仅电机温度新增 
//			for(int i = 0; i < 1; i++) 
//			{ 
//				dynamicGraphic5[i].GraphicData.operateType = JGMT_OPERATER_NEW; 
//			} 
//		} 
//		else 
//		{ 
//			for(int i = 0; i < 1; i++) 
//			{ 
//				dynamicGraphic5[i].GraphicData.operateType = JGMT_OPERATER_CHANGE; 
//			} 
//		} 
//		return; 
//	} 
//	 
//	float ammoCoverEachLineAngle = ammoCoverNowAngle; 
//	for(int i = 0; i < 6; i++, ammoCoverEachLineAngle+=60) 
//	{ 
//		if(i > 0) 
//		{ 
//			ammoCoverOpenLine[i].GraphicData.startX = ammoCoverOpenLine[i-1].GraphicData.endX; 
//			ammoCoverOpenLine[i].GraphicData.startY = ammoCoverOpenLine[i-1].GraphicData.endY; 
//		} 
//		ammoCoverOpenLine[i].GraphicData.endX = ammoCoverOpenLine[i].GraphicData.startX - 100*cosf(ammoCoverEachLineAngle/180*3.14); 
//		ammoCoverOpenLine[i].GraphicData.endY = ammoCoverOpenLine[i].GraphicData.startY + 100*sinf(ammoCoverEachLineAngle/180*3.14); 
//	} 
// 
//	memcpy(dynamicGraphic5, ammoCoverOpenLine, sizeof(ammoCoverOpenLine)); 
//	if(schedule1ReDraw > 200) 
//	{ 
//		//仅拨盘划线新增 
//		for(int i = 0; i < 6; i++) 
//		{ 
//			dynamicGraphic5[i].GraphicData.operateType = JGMT_OPERATER_NEW; 
//		} 
//	} 
//	else 
//	{ 
//		for(int i = 0; i < 6; i++) 
//		{ 
//			dynamicGraphic5[i].GraphicData.operateType = JGMT_OPERATER_CHANGE; 
//		} 
//	} 
// 
} 
int LastOffsetModifyMode=0;
void clientUpdataData6()
{
	for(u8 i=0;i<4;i++)
	{
		dynamicGraphic6[i].GraphicData.operateType=2;
	}
			dynamicGraphic6[0].intData.data =canStateStruct1.YawOffsetError;
			dynamicGraphic6[1].intData.data =canStateStruct1.PitchOffsetError;

			dynamicGraphic6[2].intData.data =canStateStruct1.PitchBuffOffset;
			dynamicGraphic6[3].intData.data =canStateStruct1.YawBuffOffset;
	/*****************取值批操作:6位负数*********************/
	for(u8 i=0;i<4;i++)
	{
		if(dynamicGraphic6[i].intData.data>>5==1)
		{
			dynamicGraphic6[i].intData.data=dynamicGraphic6[i].intData.data-64;
		}
	}


	if(canStateStruct1.OffsetModifyFlag)
	{
		dynamicGraphic6[canStateStruct1.OffsetModifyMode].intData.width=10;
		dynamicGraphic6[canStateStruct1.OffsetModifyMode].intData.startAngle=50;
		if(LastOffsetModifyMode!=canStateStruct1.OffsetModifyMode)
		{
			for(u8 i=0;i<4;i++)
			{
				dynamicGraphic6[i].intData.width=3;
				dynamicGraphic6[i].intData.startAngle=10;
			}
		}
			LastOffsetModifyMode=canStateStruct1.OffsetModifyMode;
	}
	else
	{
		for(u8 i=0;i<4;i++)
		{
			dynamicGraphic6[i].intData.width=3;
			dynamicGraphic6[i].intData.startAngle=10;
		}
	}
	
}
 
//////////////////////////////////////////////////////////////////////////// 
#define DYNAMIC_COUNT 6 
 
void draw_schedule1() 
{ 
	
	uint8_t manualUIInitFlag = canStateStruct.UI_Init;//手动UI初始化的标志位

	if(jgmt_mesg.gameStatus.gameProgress == 4) //对战中仅允许手动初始化
		UI_InitFlag = manualUIInitFlag;
	else if(jgmt_mesg.gameStatus.gameProgress == 3) //5s倒计时时强制初始化
		UI_InitFlag = 1;
	else //非对局中开启自动初始化，每30s触发2s的初始化
		UI_InitFlag = manualUIInitFlag || ((uint32_t)(sysTickUptime/1e3f)%30<2);

	
/************************************************************************************/
	schedule1ReDraw++; 
	if(schedule1ReDraw == 201)//静态 
	{ 
//		constGraphic[5].intData.data=1;
//		constGraphic[6].intData.data=7;
		for(int i = 0; i < sizeof(constGraphic)/sizeof(union GraphicDataUnion); i++) 
			constGraphic[i].GraphicData.operateType = 1+Bool(!UI_InitFlag); 
		memcpy(&jgmt_mesg.sendUnionData.unionData,constGraphic, sizeof(constGraphic)); 
		graphicDraw(sizeof(constGraphic)/sizeof(union GraphicDataUnion)); 
		return; 
	} 
	 
		 
	if(schedule1ReDraw == 202) 
	{ 
		clientUpdataData(); 
		for(int i = 0; i < sizeof(dynamicGraphic) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic[i].GraphicData.operateType = 1+Bool(!UI_InitFlag); 
		} 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic, sizeof(dynamicGraphic)); 
		graphicDraw(sizeof(dynamicGraphic)/sizeof(union GraphicDataUnion)); 
		 
		for(int i = 0; i < sizeof(dynamicGraphic) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic[i].GraphicData.operateType = 2-UI_InitFlag; 
		} 
		return; 
	} 
	 
	if(schedule1ReDraw == 203) 
	{ 
		clientUpdataData2(); 
 
		for(int i = 0; i < sizeof(dynamicGraphic2) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic2[i].GraphicData.operateType = 1+Bool(!UI_InitFlag); 
		} 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic2, sizeof(dynamicGraphic2)); 
		graphicDraw(sizeof(dynamicGraphic2)/sizeof(union GraphicDataUnion)); 
		 
		for(int i = 0; i < sizeof(dynamicGraphic2) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic2[i].GraphicData.operateType = 2-UI_InitFlag; 
		} 
		return; 
	} 
	 
	if(schedule1ReDraw == 204) 
	{ 
		for(int i = 0; i < sizeof(dynamicGraphic3) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic3[i].GraphicData.operateType = 1+Bool(!UI_InitFlag); 
		} 
		clientUpdataData3(); 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic3, sizeof(dynamicGraphic3)); 
		graphicDraw(sizeof(dynamicGraphic3)/sizeof(union GraphicDataUnion)); 
		 
		for(int i = 0; i < sizeof(dynamicGraphic3) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic3[i].GraphicData.operateType = 2-UI_InitFlag; 
		} 
		return; 
	} 
		 
	if(schedule1ReDraw == 205) 
	{ 
		//0-3是shift 
		clientUpdataData4(); 
		for(int i = 0; i < sizeof(dynamicGraphic4) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic4[i].GraphicData.operateType = 1+Bool(!UI_InitFlag); 
		} 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic4, sizeof(dynamicGraphic4)); 
		graphicDraw(sizeof(dynamicGraphic4)/sizeof(union GraphicDataUnion)); 
		for(int i = 0; i < sizeof(dynamicGraphic4) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic4[i].GraphicData.operateType = 2-UI_InitFlag; 
		} 
		return; 
	} 
	 
	if(schedule1ReDraw == 206) 
	{ 
		clientUpdataData5(); 
		for(int i = 0; i < sizeof(dynamicGraphic5) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic5[i].GraphicData.operateType = 1+Bool(!UI_InitFlag); 
		} 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic5, sizeof(dynamicGraphic5)); 
		graphicDraw(sizeof(dynamicGraphic5)/sizeof(union GraphicDataUnion)); 
		for(int i = 0; i < sizeof(dynamicGraphic5) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic5[i].GraphicData.operateType = 2-UI_InitFlag; 
		} 
		return; 
	} 
	 
	if(schedule1ReDraw == 207) 
	{ 
//		AuxiliaryUpdate();
		for(int i = 0; i < sizeof(Auxiliary) / sizeof(union GraphicDataUnion); i++) 
		{ 
			Auxiliary[i].GraphicData.operateType = 1+Bool(!UI_InitFlag); 
		} 
		memcpy(&jgmt_mesg.sendUnionData.unionData,Auxiliary, sizeof(Auxiliary)); 
		graphicDraw(sizeof(Auxiliary)/sizeof(union GraphicDataUnion)); 
		for(int i = 0; i < sizeof(Auxiliary) / sizeof(union GraphicDataUnion); i++) 
		{ 
			Auxiliary[i].GraphicData.operateType = 2-UI_InitFlag; 
		} 		
		return; 
	} 
	if(schedule1ReDraw == 208) 
	{ 
//		AuxiliaryUpdate();
		for(int i = 0; i < sizeof(Auxiliary1) / sizeof(union GraphicDataUnion); i++) 
		{ 
			Auxiliary1[i].GraphicData.operateType = 1+Bool(!UI_InitFlag); 
		} 
		memcpy(&jgmt_mesg.sendUnionData.unionData,Auxiliary1, sizeof(Auxiliary1)); 
		graphicDraw(sizeof(Auxiliary1)/sizeof(union GraphicDataUnion));  
		for(int i = 0; i < sizeof(Auxiliary1) / sizeof(union GraphicDataUnion); i++) 
		{ 
			Auxiliary1[i].GraphicData.operateType = 2-UI_InitFlag; 
		} 
		return; 
	} 
 	if(schedule1ReDraw == 209) 
	{ 
		clientUpdataData6(); 
		for(int i = 0; i < sizeof(dynamicGraphic6) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic6[i].GraphicData.operateType = 1+Bool(!UI_InitFlag); 
		} 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic6, sizeof(dynamicGraphic6)); 
		graphicDraw(sizeof(dynamicGraphic6)/sizeof(union GraphicDataUnion)); 
		for(int i = 0; i < sizeof(dynamicGraphic6) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic6[i].GraphicData.operateType = 2-UI_InitFlag; 
		} 
		
		schedule1ReDraw = 0; 
		return; 
	} 
 
	if(schedule2ReDraw % DYNAMIC_COUNT == 0) 
	{ 
		clientUpdataData(); 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic, sizeof(dynamicGraphic)); 
		graphicDraw(sizeof(dynamicGraphic)/sizeof(union GraphicDataUnion)); 
	} 
	 
	if(schedule2ReDraw % DYNAMIC_COUNT == 1) 
	{ 
		clientUpdataData2(); 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic2, sizeof(dynamicGraphic2)); 
		graphicDraw(sizeof(dynamicGraphic2)/sizeof(union GraphicDataUnion)); 
	} 
	 
	if(schedule2ReDraw % DYNAMIC_COUNT == 2) 
	{ 
		clientUpdataData3(); 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic3, sizeof(dynamicGraphic3)); 
		graphicDraw(sizeof(dynamicGraphic3)/sizeof(union GraphicDataUnion)); 
	} 
	 
	if(schedule2ReDraw % DYNAMIC_COUNT == 3) 
	{ 
		clientUpdataData4(); 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic4, sizeof(dynamicGraphic4)); 
		graphicDraw(sizeof(dynamicGraphic4)/sizeof(union GraphicDataUnion)); 
	}	 
 
	if(schedule2ReDraw % DYNAMIC_COUNT == 4) 
	{ 
		clientUpdataData5(); 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic5, sizeof(dynamicGraphic5)); 
		graphicDraw(sizeof(dynamicGraphic5)/sizeof(union GraphicDataUnion)); 
	}	 
	if(schedule2ReDraw % DYNAMIC_COUNT == 5) 
	{ 
		clientUpdataData6(); 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic6, sizeof(dynamicGraphic6)); 
		graphicDraw(sizeof(dynamicGraphic6)/sizeof(union GraphicDataUnion)); 
	}	 
	 
	schedule2ReDraw++; 
	if(schedule2ReDraw == DYNAMIC_COUNT) 
		schedule2ReDraw = 0; 
} 

sendtoguard_t sendtoguard;

void send_schedule1(void)
{
	//不发送ui时发送机间通讯
	sendtoguard.commend = commandtoguardstruct.move;
	memcpy(jgmt_mesg.sendUnionData.unionData.studentSendData,&sendtoguard, sizeof(sendtoguard)); 
	robotsCommunication(0X237, ID_GUARD, sizeof(sendtoguard));
}
void recv_COM_schedule(void)
{
	jgmt_mesg.studentRecviveData.senderId %=100;
	switch(jgmt_mesg.studentRecviveData.senderId)
	{
		case 7://guard
		memcpy(&guardstatus,&jgmt_mesg.studentRecviveData.data,sizeof(getguardstatus));
		break;
		default:
			break;
	}
}
