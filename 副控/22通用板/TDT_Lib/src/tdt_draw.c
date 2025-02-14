#include "tdt_draw.h" 
#include "board.h" 
#include "judgement.h" 
#include "string.h" 
#include "gy53.h" 
#include "can.h" 
#include "mymath.h"

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
 
//		15		18		30 
//		[0]左右 [1]上下 [2]长度 
//竖线	[0][0]	[1][0]	[2][0] 
//1m	[0][1]	[1][1]	[2][1] 
//3m	[0][2]	[1][2]	[2][2] 
//5m	[0][3]	[1][3]	[2][3] 
int16_t quasiHeart[3][4][3]= 
{ 
	{{1,2},{3,1},{1,2}}, 
	{{1,2},{3,1},{1,2}}, 
	{{1,2},{3,1},{1,2}}, 
}; 
 
union GraphicDataUnion dynamicGraphic[]= 
{ 
	//CPU offline; visionOfflineFlag; normal 
	{0, 1, 1, 1, 1, 1, 6, 21, 22, 24, 100+14, DISPLAY_HIGHT-200-14, 26, 150-14, DISPLAY_HIGHT-250+14}, 
	//superPowerOffline; superPowerState; Normal 
	{0, 1, 2, 1, 1, 1, 6, 21, 22, 24, 150+14, DISPLAY_HIGHT-200-14, 26, 200-14, DISPLAY_HIGHT-250+14}, 
	//BoostShoot; ; Normal 
	{0, 1, 3, 1, 1, 1, 6, 21, 22, 24, 200+14, DISPLAY_HIGHT-200-14, 26, 250-14, DISPLAY_HIGHT-250+14}, 
	//superPowerRemainP 
	{0, 1, 4, 1, 6, 1, 6, 50, 3, 3, 1600, DISPLAY_HIGHT-500, 0.0f}, 
	//lastBullet 
	{0,4,2,1,6,2,8,50,2,1,DISPLAY_WIDTH/2+100,DISPLAY_HIGHT/2-50,200}, 
	//steering wheel head 
	{0,4,1,1,0,2,8,11,12,5,DISPLAY_WIDTH/2,200,16,DISPLAY_WIDTH/2,240}, 
	//visionMode 
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
	//frictionSpeed 
	{0, 1, 6, 1, 6, 1, 6, 50, 1, 3, 1500, DISPLAY_HIGHT - 300, 0.0f}, 
	//pitch 
	{0, 1, 7, 1, 6, 1, 6, 50, 1, 3, 50, DISPLAY_HIGHT - 300, 0.0f}, 
		//敌方ID
	{0,3,5,1,6,1,1,30,1,3,MIDDLE_X-250,MIDDLE_Y,0.0f},
	//弹仓盖灯
	{0, 3, 20, 1, 6, 1, 8, 50, NULL, 3,MIDDLE_X-650,DISPLAY_HIGHT - 200,0},	
//	//yaw 
//	{0, 1, 8, 1, 6, 1, 6, 50, 1, 3, 50, DISPLAY_HIGHT - 400, 0.0f}, 
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
	{0, 1, 19, 1, 6, 1, 8, 50, NULL, 3,MIDDLE_X-12,MIDDLE_Y+100,0},	
//	//GY53 FF Distance 
//	{0, 1, 13, 1, 6, 1, 8, 10, 1, 1, DISPLAY_WIDTH/2+50, 230, 0.0f}, 
 
}; 
/*
union GraphicDataUnion ammoCoverOpenLine[6]= 
{ 
	//弹仓盖*6，120° 
//	{0,7,0,0,0,4,8,0,0,1,MIDDLE_X,MIDDLE_Y,0,MIDDLE_X,MIDDLE_Y-100}, 
//	{0,7,1,0,0,4,8,0,0,1,MIDDLE_X,MIDDLE_Y-100,0,MIDDLE_X+87,MIDDLE_Y-150}, 
//	{0,7,2,0,0,4,8,0,0,1,MIDDLE_X+87,MIDDLE_Y-150,0,MIDDLE_X+173,MIDDLE_Y-100}, 
//	{0,7,3,0,0,4,8,0,0,1,MIDDLE_X+173,MIDDLE_Y-100,0,MIDDLE_X+173,MIDDLE_Y}, 
//	{0,7,4,0,0,4,8,0,0,1,MIDDLE_X+173,MIDDLE_Y,0,MIDDLE_X+87,MIDDLE_Y+50}, 
//	{0,7,5,0,0,4,8,0,0,1,MIDDLE_X+87,MIDDLE_Y+50,0,MIDDLE_X,MIDDLE_Y}, 
	 
	//0° 
	{0, 7, 0, 1, 0, 1, 8, 0, 0, 1,MIDDLE_X,MIDDLE_Y,0,MIDDLE_X+50,MIDDLE_Y+87}, 
	{0, 7, 1, 1, 0, 1, 8, 0, 0, 1,MIDDLE_X+50,MIDDLE_Y+87,0,MIDDLE_X,MIDDLE_Y+173}, 
	{0, 7, 2, 1, 0, 1, 8, 0, 0, 1,MIDDLE_X,MIDDLE_Y+173,0,MIDDLE_X-100,MIDDLE_Y+173}, 
	{0, 7, 3, 1, 0, 1, 8, 0, 0, 1,MIDDLE_X-100,MIDDLE_Y+173,0,MIDDLE_X-150,MIDDLE_Y+87}, 
	{0, 7, 4, 1, 0, 1, 8, 0, 0, 1,MIDDLE_X-150,MIDDLE_Y+87,0,MIDDLE_X-100,MIDDLE_Y}, 
	{0, 7, 5, 1, 0, 1, 8, 0, 0, 1,MIDDLE_X-100,MIDDLE_Y,0,MIDDLE_X,MIDDLE_Y}, 
}; 
 */
union GraphicDataUnion temperatureShow[6]= 
{ 
	//云台温度*2 + 底盘最高电机温度 
	//pitch 
	{0, 7, 0, 1, 6, 1, 6, 20, NULL, 3, 50, DISPLAY_HIGHT - 375, 100}, 
	//yaw 
	{0, 7, 1, 1, 6, 1, 6, 20, NULL, 3, 50, DISPLAY_HIGHT - 400, 200}, 
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
 
 
union GraphicDataUnion dynamicGraphic5[7];//= 
//{ 
//	//弹仓盖*6 || 云台温度*2 + 底盘最高电机温度 
//}; 
 
 
 
 
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
	{0,2,6,1,6,5,8,450,NULL,1,MIDDLE_X-450/**/,DISPLAY_HIGHT-420,100},//1
	{0,2,7,1,6,5,8,450,NULL,1,MIDDLE_X+50/**/,DISPLAY_HIGHT-420,100},//7 
}; 
 
#define calibrationX (MIDDLE_X-15)
#define L2for15_X (calibrationX-25)
#define L3for15_X (calibrationX-20)
#define L4for15_X (calibrationX-13)
#define L45for15_X (calibrationX-13)
#define L5for15_X (calibrationX-11)
#define L55for15_X (calibrationX-10)
#define L6for15_X (calibrationX-9)

union GraphicDataUnion Auxiliary[] = { //射表
	{0, 8, 0, 1, 0, 2, 2, 0, 0, 1, calibrationX, 470, 0, calibrationX, 300},//偏右

	{0, 8, 1, 1, 0, 2, 2, 0, 0, 2, L2for15_X, 445, 0, calibrationX*2-L2for15_X, 445}, //2m激光限-随弹速调整
	{0, 8, 3, 1, 0, 2, 3, 0, 0, 1, L3for15_X, 425, 0, calibrationX*2-L3for15_X, 425}, //3m-15mps
//	{0, 8, 4, 1, 0, 2, 2, 0, 0, 1, 950, 413, 0, 990, 413}, //3.5m-15mps
	{0, 8, 5, 1, 0, 2, 6, 0, 0, 1, L4for15_X, 408, 0, calibrationX*2-L4for15_X, 408}, //4m-15mps
	
	{0, 8, 6, 1, 0, 2, 6, 0, 0, 1, L45for15_X, 390, 0, calibrationX*2-L45for15_X, 390}, //4.5m-15mps 
	{0, 8, 2, 1, 0, 2, 2, 0, 0, 1, L55for15_X, 362, 0, calibrationX*2-L55for15_X, 362}, //5.5m-15mps	
};  

union GraphicDataUnion Auxiliary1[] = { //射表

	{0, 8, 10, 1, 0, 2, 5, 0, 0, 2, L5for15_X, 380, 0, calibrationX*2-L5for15_X, 380}, //5m-15mps & 近战下限
	{0, 8, 11, 1, 0, 2, 6, 0, 0, 1, L6for15_X, 350, 0, calibrationX*2-L6for15_X, 350}, //6m-15mps 

};   
union GraphicDataUnion AuxiliaryNum[] = { //射表数字,15,ps左30mps右
	{0, 8, 7, 1, 6, 2, 3, 10, NULL, 2, L3for15_X, 425,3}, //3m-15mps
	{0, 8, 8, 1, 6, 2, 5, 10, NULL, 2, L5for15_X, 380, 5}, //5m-15mps
	{0, 8, 9, 1, 6, 2, 6, 10, NULL, 2, L6for15_X, 350, 6}, //6m-15mps
}; 

 
u8 speedToIndex() 
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
	 
	if(superPowerOffline)//为1则功率离线 
	{ 
		dynamicGraphic[1].GraphicData.color = COLOR_MODE1; 
	} 
	else if(canStateStruct.ULTSMode) 
	{ 
		dynamicGraphic[1].GraphicData.color = COLOR_MODE2; 
	} 
	else 
	{ 
		dynamicGraphic[1].GraphicData.color = COLOR_DISABLE; 
	} 
	 
		if(canStateStruct.fireFrequencyMode) //摩擦轮
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
		dynamicGraphic[6].GraphicData.color = COLOR_MODE1; 
	} 
	else if(canStateStruct.VisionMode == 2)//小陀螺 
	{ 
		dynamicGraphic[6].GraphicData.color = COLOR_MODE2; 
	} 
	else if(canStateStruct.VisionMode == 3)//打小符 
	{ 
		dynamicGraphic[6].GraphicData.color = JGMT_COLOR_GREEN; 
	} 
	else if(canStateStruct.VisionMode == 4) //反符
	{
		dynamicGraphic[6].GraphicData.color = 5; 
	}	

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
	}
	else
	{
		dynamicGraphic2[6].GraphicData.operateType=2;
		dynamicGraphic2[6].GraphicData.color=8;		
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
//	dynamicGraphic3[6].intData.data = GYdistance[LB]; 
} 
 
float localHeatPercent = 0; 
uint16_t customHeatLimit=0; 
int myColor = 0;
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
 u8 LastSentryCtrl = 0;
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
	 
	dynamicGraphic4[4].GraphicData.startAngle = (1-localHeatPercent)*360; 
	if(dynamicGraphic4[4].GraphicData.startAngle == 360) 
		dynamicGraphic4[4].GraphicData.startAngle = 359; 
	 
	dynamicGraphic4[4].GraphicData.color = JGMT_COLOR_WHITE; 
	 
	if(localHeatPercent > 0.5) 
		dynamicGraphic4[4].GraphicData.color = JGMT_COLOR_YELLOW;	 
	if(localHeatPercent > 0.75) 
		dynamicGraphic4[4].GraphicData.color = JGMT_COLOR_ORANGE; 
	 
//	if(canStateStruct.heatChoose == 1) 
//	{ 
//		dynamicGraphic4[4].GraphicData.color = JGMT_COLOR_BLACK; 
//	} 
//	 
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
//		temperatureShow[1].intData.data = can1Feedback.gimbalTemperature[1]; 
//		 
//		memcpy(dynamicGraphic5, temperatureShow, sizeof(temperatureShow)); 
//		if(schedule1ReDraw > 200) 
//		{ 
//			//仅电机温度新增 
//			for(int i = 0; i < 3; i++) 
//			{ 
//				dynamicGraphic5[i].GraphicData.operateType = JGMT_OPERATER_NEW; 
//			} 
//		} 
//		else 
//		{ 
//			for(int i = 0; i < 3; i++) 
//			{ 
//				dynamicGraphic5[i].GraphicData.operateType = JGMT_OPERATER_CHANGE; 
//			} 
//		} 
//		return; 
//	} 
	 
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
 
} 
 
 
 
#define DYNAMIC_COUNT 5 
 
void draw_schedule1() 
{ 
	schedule1ReDraw++; 
	if(schedule1ReDraw == 201) 
	{ 
		for(int i = 0; i < sizeof(constGraphic)/sizeof(union GraphicDataUnion); i++) 
			constGraphic[i].GraphicData.operateType = 1; 
		memcpy(&jgmt_mesg.sendUnionData.unionData,constGraphic, sizeof(constGraphic)); 
		graphicDraw(sizeof(constGraphic)/sizeof(union GraphicDataUnion)); 
		return; 
	} 
	 
		 
	if(schedule1ReDraw == 202) 
	{ 
		clientUpdataData(); 
		for(int i = 0; i < sizeof(dynamicGraphic) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic[i].GraphicData.operateType = 1; 
		} 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic, sizeof(dynamicGraphic)); 
		graphicDraw(sizeof(dynamicGraphic)/sizeof(union GraphicDataUnion)); 
		 
		for(int i = 0; i < sizeof(dynamicGraphic) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic[i].GraphicData.operateType = 2; 
		} 
		return; 
	} 
	 
	if(schedule1ReDraw == 203) 
	{ 
		clientUpdataData2(); 
 
		for(int i = 0; i < sizeof(dynamicGraphic2) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic2[i].GraphicData.operateType = 1; 
		} 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic2, sizeof(dynamicGraphic2)); 
		graphicDraw(sizeof(dynamicGraphic2)/sizeof(union GraphicDataUnion)); 
		 
		for(int i = 0; i < sizeof(dynamicGraphic2) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic2[i].GraphicData.operateType = 2; 
		} 
		return; 
	} 
	 
	if(schedule1ReDraw == 204) 
	{ 
		for(int i = 0; i < sizeof(dynamicGraphic3) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic3[i].GraphicData.operateType = 1; 
		} 
		clientUpdataData3(); 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic3, sizeof(dynamicGraphic3)); 
		graphicDraw(sizeof(dynamicGraphic3)/sizeof(union GraphicDataUnion)); 
		 
		for(int i = 0; i < sizeof(dynamicGraphic3) / sizeof(union GraphicDataUnion); i++) 
		{ 
			dynamicGraphic3[i].GraphicData.operateType = 2; 
		} 
		return; 
	} 
		 
	if(schedule1ReDraw == 205) 
	{ 
		//0-3是shift 
		dynamicGraphic4[4].GraphicData.operateType = 1; 
		dynamicGraphic4[5].GraphicData.operateType = 1; 
		clientUpdataData4(); 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic4, sizeof(dynamicGraphic4)); 
		graphicDraw(sizeof(dynamicGraphic4)/sizeof(union GraphicDataUnion)); 
		dynamicGraphic4[4].GraphicData.operateType = 2; 
		dynamicGraphic4[5].GraphicData.operateType = 2; 
		return; 
	} 
	 
	if(schedule1ReDraw == 206) 
	{ 
		clientUpdataData5(); 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic5, sizeof(dynamicGraphic5)); 
		graphicDraw(sizeof(dynamicGraphic5)/sizeof(union GraphicDataUnion)); 
		return; 
	} 
	 
	if(schedule1ReDraw == 207) 
	{ 
		memcpy(&jgmt_mesg.sendUnionData.unionData,Auxiliary, sizeof(Auxiliary)); 
		graphicDraw(sizeof(Auxiliary)/sizeof(union GraphicDataUnion)); 
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
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic4, sizeof(dynamicGraphic3)); 
		graphicDraw(sizeof(dynamicGraphic4)/sizeof(union GraphicDataUnion)); 
	}	 
 
	if(schedule2ReDraw % DYNAMIC_COUNT == 4) 
	{ 
		clientUpdataData5(); 
		memcpy(&jgmt_mesg.sendUnionData.unionData,dynamicGraphic5, sizeof(dynamicGraphic5)); 
		graphicDraw(sizeof(dynamicGraphic5)/sizeof(union GraphicDataUnion)); 
	}	 
	 
	schedule2ReDraw++; 
	if(schedule2ReDraw == DYNAMIC_COUNT) 
		schedule2ReDraw = 0; 
} 
