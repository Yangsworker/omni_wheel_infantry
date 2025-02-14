#include "draw_task.h"
#include "com_task.h"
#include "chassis_task.h"
#include "judgement.h"
#include "power.h"

bool UI_InitFlag = 1;//UI���ڳ�ʼ����ʶλ

/*��������*/
/*  ͼ��:����id  ||  ID0:����id  ||  ID1:��״id  ||  ID2:ˢ������  */
/*  ˢ������: 0-��̬ 1-��Ƶ 2-��Ƶ 3-����  */

//ͼ����Ԥ���
static Judgement::GraphicDataStruct Demo_Graphic	{0,0,0,1,0,0,0,0,0,3, 960, 540,10,0,0};

static Judgement::GraphicDataStruct Shotable_L0		{1,1,0,		1,0,5,2,	0,0,	2,	SHOTABLE_X_BASE,	500, 0, SHOTABLE_X_BASE,	150};/*��*/
static Judgement::GraphicDataStruct Shotable_L1		{1,2,0,		1,0,5,2,	0,0,	2,	SHOTABLE_X_BASE-173,162, 0,	SHOTABLE_X_BASE+173,162};/*��*/
static Judgement::GraphicDataStruct Shotable_L2		{1,3,0,		1,0,5,2,	0,0,	2,	SHOTABLE_X_BASE-47,	456, 0,	SHOTABLE_X_BASE+47,	456};/*��*/
static Judgement::GraphicDataStruct Shotable_L3		{1,4,0,		1,0,5,2,	0,0,	2,	SHOTABLE_X_BASE-26,	459, 0, SHOTABLE_X_BASE+26,	459};/*��*/
static Judgement::GraphicDataStruct Shotable_L4		{1,5,0,		1,0,5,2,	0,0,	2,	SHOTABLE_X_BASE-18,	438, 0, SHOTABLE_X_BASE+18,	438};/*��*/
static Judgement::GraphicDataStruct Shotable_L5		{1,6,0,		1,0,5,2,	0,0,	2,	SHOTABLE_X_BASE-15,	427, 0, SHOTABLE_X_BASE+15,	427};/*��*/
static Judgement::GraphicDataStruct Shotable_L6		{1,7,0,		1,0,5,2,	0,0,	2,	SHOTABLE_X_BASE-9,	400, 0, SHOTABLE_X_BASE+9,	400};/*��*/

static Judgement::GraphicDataStruct WidthLine_L		{2,1,0,		1,0,5,8,	0,0,	1,	712, 600, 0, 400,   0};/*��*/
static Judgement::GraphicDataStruct WidthLine_R		{2,2,0,		1,0,5,8,	0,0,	1,	1230, 600, 0,1591,   0};/*��*/

static Judgement::GraphicDataStruct VisionFrame		{3,1,0,		1,1,5,8,	0,0,	1,	650, 180,	0,	1250, 730};/*��*/
static Judgement::GraphicDataStruct VisionFrameLong	{3,2,0,		1,1,5,8,	0,0,	1,	768, 0,	0,	1152, 1080};

	   Judgement::GraphicDataStruct VisionSight  	{4,1,2,		1,4,5,1,	45,315,	3,	960, 540, 8,8,8};/*��*/
	   Judgement::GraphicDataStruct EnemyHP_Circle  {4,2,1,		1,4,5,2,	160,0,	8,	ENEMYINFO_X_BASE, ENEMYINFO_Y_BASE,40,40,40};/*��*/

static Judgement::GraphicDataStruct Cap_Border		{5,1,0,		1,1,5,7,	0,0,	4,	CAPINFO_X_BASE,		CAPINFO_Y_BASE-20,	0,	CAPINFO_X_BASE+600,CAPINFO_Y_BASE+20};/*��*/
static Judgement::GraphicDataStruct Cap_BackPack	{5,2,0,		1,0,5,1,	0,0,	36,	CAPINFO_X_BASE+2,	CAPINFO_Y_BASE,		0,	CAPINFO_X_BASE+180,CAPINFO_Y_BASE};/*��*/
	   Judgement::GraphicDataStruct Cap_Cap			{5,3,2,		1,0,5,8,	0,0,	36,	CAPINFO_X_BASE+180,	CAPINFO_Y_BASE,		0,	CAPINFO_X_BASE+540,CAPINFO_Y_BASE};/*��*/
	   Judgement::GraphicDataStruct Cap_Limit		{5,4,1,		1,1,5,4,	0,0,	4,	CAPINFO_X_BASE+510,	CAPINFO_Y_BASE-20-4,0,	CAPINFO_X_BASE+570,CAPINFO_Y_BASE+20+4};/*��*/

//static Judgement::GraphicDataStruct AttitudeBall	{6,1,0,		1,2,5,8,	0,0,	4,	ATTITUDE_X_BASE, ATTITUDE_Y_BASE,50,0,0};/*��*/
       Judgement::GraphicDataStruct AttitudeLine	{6,1,1,		1,0,5,3,	0,0,	4,	ATTITUDE_X_BASE-300, ATTITUDE_Y_BASE,24,ATTITUDE_X_BASE+300,ATTITUDE_Y_BASE};/*��*/

//       Judgement::GraphicDataStruct CoverFlag		{7,1,1,		1,2,5,6,  	0,0,	3,	COVER_X_BASE, COVER_Y_BASE,10,0,0};/*��*/

static Judgement::GraphicDataStruct FeedFrame		{8,1,0,		1,2,5,7,  	0,0,	4,	FEED_X_BASE, FEED_Y_BASE,27,0, 0};/*��*/
       Judgement::GraphicDataStruct FeedPos			{8,2,1,		1,2,5,2,  	0,0,	3,	FEED_X_BASE, FEED_Y_BASE+12,12,0, 0};/*��*/

static Judgement::GraphicDataStruct SpeedMeterFrame	{9,1,0,		1,2,5,0,	0,0,	5,	METER_X_BASE,METER_Y_BASE,100, 0,0};/*��*/
static Judgement::GraphicDataStruct SpeedMeterScale1{9,2,0,		1,0,5,0,	0,0,	2,	_1X1,_1Y1,0,_1X2,_1Y2};/*��*/
static Judgement::GraphicDataStruct SpeedMeterScale2{9,3,0,		1,0,5,0,	0,0,	2,	_2X1,_2Y1,0,_2X2,_2Y2};/*��*/
static Judgement::GraphicDataStruct SpeedMeterScale3{9,4,0,		1,0,5,0,	0,0,	2,	_3X1,_3Y1,0,_3X2,_3Y2};/*��*/
static Judgement::GraphicDataStruct SpeedMeterScale4{9,5,0,		1,0,5,0,	0,0,	2,	_4X1,_4Y1,0,_4X2,_4Y2};/*��*/
static Judgement::GraphicDataStruct SpeedMeterScale5{9,6,0,		1,0,5,0,	0,0,	2,	_5X1,_5Y1,0,_5X2,_5Y2};/*��*/
static Judgement::GraphicDataStruct SpeedMeterScale6{9,7,0,		1,0,5,0,	0,0,	2,	_6X1,_6Y1,0,_6X2,_6Y2};/*��*/
static Judgement::GraphicDataStruct SpeedMeterScale7{9,8,0,		1,0,5,0,	0,0,	2,	_7X1,_7Y1,0,_7X2,_7Y2};/*��*/
static Judgement::GraphicDataStruct SpeedMeterScale8{9,9,0,		1,0,5,0,	0,0,	2,	_8X1,_8Y1,0,_8X2,_8Y2};/*��*/
       Judgement::GraphicDataStruct SpeedMeterTorque{9,10,2,	1,0,5,3,	0,0,	6,	METER_X_BASE,METER_Y_BASE,0,METER_X_BASE,METER_Y_BASE+70};/*��*/

static Judgement::GraphicDataStruct Battery_Border_1{10,1,0,	1,1,5,8,  	0,0,	5,	BATTERY_X_BASE-100,BATTERY_Y_BASE-10, 0, BATTERY_X_BASE-85, BATTERY_Y_BASE+10};/*��*/
static Judgement::GraphicDataStruct Battery_Border_2{10,2,0,	1,1,5,8,  	0,0,	5,	BATTERY_X_BASE-85, BATTERY_Y_BASE-30, 0, BATTERY_X_BASE+5, 	BATTERY_Y_BASE+30};/*��*/
       Judgement::GraphicDataStruct Battery_Line_1	{10,3,1,	1,0,5,2,  	0,0,	15,	BATTERY_X_BASE-10, BATTERY_Y_BASE-22, 0, BATTERY_X_BASE-10, BATTERY_Y_BASE+22};
       Judgement::GraphicDataStruct Battery_Line_2	{10,4,1,	1,0,5,2,  	0,0,	15,	BATTERY_X_BASE-30, BATTERY_Y_BASE-22, 0, BATTERY_X_BASE-30, BATTERY_Y_BASE+22};
       Judgement::GraphicDataStruct Battery_Line_3	{10,5,1,	1,0,5,2,  	0,0,	15,	BATTERY_X_BASE-50, BATTERY_Y_BASE-22, 0, BATTERY_X_BASE-50, BATTERY_Y_BASE+22};
       Judgement::GraphicDataStruct Battery_Line_4	{10,6,1,	1,0,5,2,  	0,0,	15,	BATTERY_X_BASE-70, BATTERY_Y_BASE-22, 0, BATTERY_X_BASE-70, BATTERY_Y_BASE+22};

static Judgement::GraphicDataStruct Follow_Frame	{11,1,0,	1,2,5,8,	0,0,	5,	FOLLOW_X_BASE, FOLLOW_Y_BASE,40,0,0};/*��*/
	   Judgement::GraphicDataStruct Follow_Wheel_L	{11,2,2,	1,4,5,8,	0,180,	3,	FOLLOW_X_BASE, FOLLOW_Y_BASE+20,20,20,20};/*��*/
	   Judgement::GraphicDataStruct Follow_Wheel_R	{11,3,2,	1,4,5,8,	180,0,	3,	FOLLOW_X_BASE, FOLLOW_Y_BASE-20,20,20,20};/*��*/
	   Judgement::GraphicDataStruct Follow_Point_F	{11,4,2,	1,2,5,7,	0,0,	10,	FOLLOW_X_BASE, FOLLOW_Y_BASE+20,1,1,1};/*��*/
	   Judgement::GraphicDataStruct Follow_Point_B	{11,5,2,	1,2,5,8,	0,0,	10,	FOLLOW_X_BASE, FOLLOW_Y_BASE-20,1,1,1};/*��*/

       Judgement::GraphicDataStruct LegLine1		{12,1,2,	1,0,5,0,	0,0,	2,	LEG_X_BASE-60, 	LEG_Y_BASE+140,0, LEG_X_BASE-100, 	LEG_Y_BASE+100};/**/
       Judgement::GraphicDataStruct LegLine2		{12,2,2,	1,0,5,0,	0,0,	2,	LEG_X_BASE-100, LEG_Y_BASE+100,0, LEG_X_BASE, 		LEG_Y_BASE};/**/
       Judgement::GraphicDataStruct LegLine3		{12,3,2,	1,0,5,0,	0,0,	2,	LEG_X_BASE, 	LEG_Y_BASE	,0,	  LEG_X_BASE+100,	LEG_Y_BASE+100};/**/
       Judgement::GraphicDataStruct LegLine4		{12,4,2,	1,0,5,0,	0,0,	2,	LEG_X_BASE+100, LEG_Y_BASE+100,0, LEG_X_BASE+60,	LEG_Y_BASE+140};/**/
       Judgement::GraphicDataStruct Joint1_Flag		{12,5,2,	1,2,5,1,	0,0,	6,	LEG_X_BASE+100, LEG_Y_BASE+100,5,0,0};/**/
       Judgement::GraphicDataStruct Joint2_Flag		{12,6,2,	1,2,5,1,	0,0,	6,	LEG_X_BASE-100, LEG_Y_BASE+100,5,0,0};/**/
       Judgement::GraphicDataStruct Wheel_Flag		{12,7,1,	1,2,5,8,	0,0,	20,	LEG_X_BASE, 	LEG_Y_BASE,14,0,0};/**/

static Judgement::GraphicDataStruct Idler_Frame		{13,1,0,	1,1,5,8,	0,0,	5,	IDLER_X_BASE-20, IDLER_Y_BASE-50,0,IDLER_X_BASE+20,IDLER_Y_BASE+50};/*��*/
static Judgement::GraphicDataStruct Idler_Line1		{13,2,0,	1,0,5,8,	0,0,	3,	IDLER_X_BASE-10, IDLER_Y_BASE-30,0,IDLER_X_BASE+10,IDLER_Y_BASE-30};/*��*/
static Judgement::GraphicDataStruct Idler_Line2		{13,3,0,	1,0,5,8,	0,0,	3,	IDLER_X_BASE-10, IDLER_Y_BASE-10,0,IDLER_X_BASE+10,IDLER_Y_BASE-10};/*��*/
static Judgement::GraphicDataStruct Idler_Line3		{13,4,0,	1,0,5,8,	0,0,	3,	IDLER_X_BASE-10, IDLER_Y_BASE+10,0,IDLER_X_BASE+10,IDLER_Y_BASE+10};/*��*/
static Judgement::GraphicDataStruct Idler_Line4		{13,5,0,	1,0,5,8,	0,0,	3,	IDLER_X_BASE-10, IDLER_Y_BASE+30,0,IDLER_X_BASE+10,IDLER_Y_BASE+30};/*��*/
	
	   Judgement::GraphicDataStruct MotorTempLine	{17,2,1,	1,4,5,2,	45,135,	5,	960, 540, 380,380,380};/*��*/

//������Ԥ���[��32λΪ����]
static Judgement::GraphicDataStruct Demo_Num		{0,0,0,1,5,0,0,0,0,3,960,540};

	   Judgement::GraphicDataStruct enemyHP_Num  	{4,3,1,		1,6,5,8,	15,4,	3,	ENEMYINFO_X_BASE+30, ENEMYINFO_Y_BASE-54,0};/*��*/
	   Judgement::GraphicDataStruct enemyID  		{4,4,1,		1,6,5,1,	30,2,	3,	ENEMYINFO_X_BASE-8, ENEMYINFO_Y_BASE+12,0};/*��*/

	   Judgement::GraphicDataStruct Cap_Percent		{5,5,1,		1,5,5,1,	30,4,	3,	CAPINFO_X_BASE+540, CAPINFO_Y_BASE-35,0};/*��*/

	   Judgement::GraphicDataStruct PitchCode		{14,1,1,	1,6,5,1,	50,4,	3,	HEAD_X_BASE, HEAD_Y_BASE-30,0};/*��*/
	   Judgement::GraphicDataStruct YawCode			{14,2,1,	1,6,5,1,	50,4,	3,	HEAD_X_BASE, HEAD_Y_BASE+30,0};/*��*/

	   Judgement::GraphicDataStruct FriLSpd			{15,1,1,	1,6,5,6,	30,4,	3,	FRI_X_BASE, FRI_Y_BASE+20,0};/*��*/
	   Judgement::GraphicDataStruct FriRSpd			{15,2,1,	1,6,5,6,	30,4,	3,	FRI_X_BASE, FRI_Y_BASE-20,0};/*��*/

	   Judgement::GraphicDataStruct SpeedMeterSpeed	{9,11,1,	1,6,5,8,	30,5,	3,	METER_X_BASE-40,METER_Y_BASE-30,0};/*��*/
	   
	   Judgement::GraphicDataStruct IdlerID			{13,6,1,	1,6,5,8,	15,30,	2,	IDLER_X_BASE+30, IDLER_Y_BASE,0};
	   Judgement::GraphicDataStruct IdlerValue		{13,7,1,	1,5,5,8,	15,30,	2,	IDLER_X_BASE+30, IDLER_Y_BASE-20,0};
	   
	   Judgement::GraphicDataStruct MotorTemperature{17,1,1,	1,5,5,2,	30,4,	3,	TEMP_X_BASE,TEMP_Y_BASE,0};/*��*/

//�ַ���Ԥ���[15b+30b]
static Judgement::ClientCustomCharacter Demo_Chars	{0,0,0,1,7,0,0,8,30,3,0,500,10,0,0,"ABCDEFGHIJKLMNOPQRSTUVWXYZ/?!"};

//	   Judgement::ClientCustomCharacter IdlerInfo	{13,6,3,	1,7,5,8,	15,30,	2,	IDLER_X_BASE+30, IDLER_Y_BASE,10,0,0,"HEIGHT"};
	   Judgement::ClientCustomCharacter ChssisState	{16,1,3,	1,7,5,4,	45,30,	4,	CHASSIS_STATE_X_BASE, CHASSIS_STATE_Y_BASE,10,0,0,"POOLGUY"};

uint16_t getEnemyHP();//�õ�Ŀ��ʣ��Ѫ��

//�Ը�ƵUI���б������ơ��졿
//����:�Ӿ�׼�ġ���׼��������ͼ��������������ͼ���������ʶ
//֡λ7/7
void HighFreqUI_Update_0(void)
{
	u8 i = 0;

	/*�Ӿ�׼��*/
	VisionSight.operateType = 1 + !UI_InitFlag;
	switch(DisplayPack.visState)
	{
		case 1:		//�Ӿ�������[��]
			VisionSight.color = 6;
			break;
		case 2:		//�Ӿ�������Ŀ��[��]
			VisionSight.color = 5;
			break;
		case 3:		//�Ӿ����Կ���[��]
			VisionSight.color = 2;
			break;
		default:	//�Ӿ�����[��]
			VisionSight.color = 8;		
	}
	VisionSight.graphicType = 4-2*(DisplayPack.visionMode > 3);//��С��ģʽ��Բ
	VisionSight.radius = (DisplayPack.visionMode == 4)?4:8;//С��ʱԲȦ��С
	VisionSight.startAngle = 40+180*(DisplayPack.visionMode == 3);//����ģʽ�°�Բ
	VisionSight.startAngle+= (DisplayPack.visionMode == 2)*((uint32_t)(sysTickUptime/2.0)%360);//С����ģʽ��תԲ��
	VisionSight.endAngle   = (VisionSight.startAngle+280)%360;
	VisionSight.startX = 960 - (DisplayPack.visState>1)*DisplayPack.visYawError*15;//����Ŀ���Ÿ���,��ָʾYaw��
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&VisionSight,15);
	
	/*��׼������*/
	EnemyHP_Circle.operateType = 1 + !UI_InitFlag;
	EnemyHP_Circle.startAngle = 360-LIMIT(getEnemyHP(),1,500)/500.0f*360;//��׼��ͨ���浥λʱ��һȦ����500Ѫ
//	EnemyHP_Circle.color = ;//TODO
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&EnemyHP_Circle,15);

	/*��������*/
	Cap_Cap.operateType = 1 + !UI_InitFlag;
	
	if(power.whereIsPower == Power::offline)	//��������ʱ���������
		Cap_Cap.color = 7;
	else if(!power.useCap)					//��ֱͨʱ����������
		Cap_Cap.color = 4;
	else if(!power.capHealth)					//�����쳣ʱ���������
		Cap_Cap.color = 1;
	else if(power.usingBackup)				//ʹ�ñ��ù���ʱ���������
		Cap_Cap.color = 3;
	else										//����������Ϊ��ɫ
		Cap_Cap.color = 8;
	
	Cap_Cap.endX = CAPINFO_X_BASE+6*power.capacitance_percentage;//ֱ����ʾ���ݰٷֱ�
	Cap_Cap.startX = (power.capacitance_percentage>30) ? CAPINFO_X_BASE+30*6 : CAPINFO_X_BASE;
	
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Cap_Cap,15);
	
//	/*�����ʶ*/
//	Follow_Wheel_L.operateType = 1 + !UI_InitFlag;
//	Follow_Wheel_R.operateType = 1 + !UI_InitFlag;
//	Follow_Point_F.operateType = 1 + !UI_InitFlag;
//	Follow_Point_B.operateType = 1 + !UI_InitFlag;

//	Follow_Point_F.startX 	= FOLLOW_X_BASE+20*my_sin(chassis.followAglFb_s*RAD_PER_DEG);
//	Follow_Point_F.startY	= FOLLOW_Y_BASE+20*my_cos(chassis.followAglFb_s*RAD_PER_DEG);
//	Follow_Point_B.startX 	= FOLLOW_X_BASE*2-Follow_Point_F.startX;
//	Follow_Point_B.startY	= FOLLOW_Y_BASE*2-Follow_Point_F.startY;

//	Follow_Wheel_L.startX = Follow_Point_F.startX;
//	Follow_Wheel_L.startY = Follow_Point_F.startY;
//	Follow_Wheel_R.startX = Follow_Point_B.startX;
//	Follow_Wheel_R.startY = Follow_Point_B.startY;

//	Follow_Wheel_L.startAngle 	= (uint16_t)chassis.followAglFb_s;
//	Follow_Wheel_L.endAngle		= ((uint16_t)chassis.followAglFb_s+180)%360;
//	Follow_Wheel_R.startAngle 	= Follow_Wheel_L.endAngle;
//	Follow_Wheel_R.endAngle		= Follow_Wheel_L.startAngle;
//	
//	Follow_Wheel_L.color = CtrlPack.AvoidMode+1;//0-��������|1-��������|2-��������|3-ҡ��
//	Follow_Wheel_R.color = Follow_Wheel_L.color;

//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Follow_Wheel_L,15);
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Follow_Wheel_R,15);
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Follow_Point_F,15);
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Follow_Point_B,15);
	
	judgement.graphicDraw(i);
}


//�Ը�ƵUI���б������ơ��ơ�
//����:�ٶ��Ǳ�ָ�롢�Ȳ���ʾ
//֡λ7/7
void HighFreqUI_Update_1(void)
{
	u8 i = 0;

	/*Ť���Ǳ�ָ��[��ʵ���ٶ�]*/
//	//30�� һ�� 1.0m/s
//	SpeedMeterTorque.operateType = 1 + !UI_InitFlag;
//	SpeedMeterTorque.endX = 70*my_sin(chassis.os->speedGoFb/100.0f*3*RAD_PER_DEG) + METER_X_BASE;
//	SpeedMeterTorque.endY = 70*my_cos(chassis.os->speedGoFb/100.0f*3*RAD_PER_DEG) + METER_Y_BASE;
//	SpeedMeterTorque.color = 3+3*chassis.staticCtrlFlag;
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&SpeedMeterTorque,15);

//	/*�Ȳ���ʾ*/
//	LegLine1.operateType = 1 + !UI_InitFlag;
//	LegLine2.operateType = LegLine1.operateType;
//	LegLine3.operateType = LegLine1.operateType;
//	LegLine4.operateType = LegLine1.operateType;
//	Joint1_Flag.operateType = LegLine1.operateType;
//	Joint2_Flag.operateType = LegLine1.operateType;
//	
//	//����ʱ�ȱ�ڣ�ǿ�ƿ���ʱ�ȱ��ϣ����������ʾ������ɫ
//	if(!CtrlPack.onforceFlag)
//		LegLine1.color = 8;
//	else if(CtrlPack.forceLegFlag)
//		LegLine1.color = 4;
//	else
//		LegLine1.color = 0;
//	
//	LegLine2.color = LegLine1.color;
//	LegLine3.color = LegLine1.color;
//	LegLine4.color = LegLine1.color;

//	LegLine1.startX = LEG_X_BASE-WaistLen/2.0f/2.0f-chassis.leg[0]->legEndFbX/2.0f;
//	LegLine1.startY = LEG_Y_BASE+chassis.leg[0]->legEndFbY/2.0f;
//	LegLine4.endX = LEG_X_BASE+WaistLen/2.0f/2.0f-chassis.leg[0]->legEndFbX/2.0f;
//	LegLine4.endY = LEG_Y_BASE+chassis.leg[0]->legEndFbY/2.0f;

//	LegLine1.endX = LegLine1.startX - ThighLen/2.0f*my_cos(chassis.leg[0]->jointAglFb[0]*RAD_PER_DEG);
//	LegLine1.endY = LegLine1.startY - ThighLen/2.0f*my_sin(chassis.leg[0]->jointAglFb[0]*RAD_PER_DEG);
//	LegLine4.startX = LegLine4.endX + ThighLen/2.0f*my_cos(chassis.leg[0]->jointAglFb[1]*RAD_PER_DEG);
//	LegLine4.startY = LegLine4.endY - ThighLen/2.0f*my_sin(chassis.leg[0]->jointAglFb[1]*RAD_PER_DEG);

//	LegLine2.startX = LegLine1.endX;
//	LegLine2.startY = LegLine1.endY;
//	LegLine3.endX = LegLine4.startX;
//	LegLine3.endY = LegLine4.startY;

//	Joint1_Flag.startX = LegLine1.startX;
//	Joint1_Flag.startY = LegLine1.startY;
//	Joint2_Flag.startX = LegLine4.endX;
//	Joint2_Flag.startY = LegLine4.endY;

//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&LegLine1,15);
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&LegLine2,15);
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&LegLine3,15);
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&LegLine4,15);
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Joint1_Flag,15);
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Joint2_Flag,15);

	judgement.graphicDraw(i);
}


//�Ե�ƵUI���б������ơ��̡�
//����:��׼�����������֣�����׼��װ��ID������������������PY��е�ǡ�Ħ����ת��
//֡λ7/7
void LowFreqUI_Update_0(void)
{
	u8 i = 0;
	
	/*��׼������*/
	enemyHP_Num.operateType = 1 + !UI_InitFlag;
	myUInumLoad(&enemyHP_Num,getEnemyHP());
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&enemyHP_Num,15);

	/*��׼��װ��ID*/
	enemyID.operateType = 1 + !UI_InitFlag;
	myUInumLoad(&enemyID,DisplayPack.enemyID);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&enemyID,15);

	/*��������*/
	Cap_Percent.operateType = 1 + !UI_InitFlag;
	Cap_Percent.color = 1+2*(power.ULTS_Mode);//������ɫ��ULTS�³�ɫ
	myUInumLoad(&Cap_Percent,power.capacitance_percentage*1000);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Cap_Percent,15);

	/*PY��е��*/
//	PitchCode.operateType = 1 + !UI_InitFlag;
//	YawCode.operateType = 1 + !UI_InitFlag;
//	PitchCode.color = 1+6*OffsetPack.PitchOffline;//����ʱ��ɫ��P������ʱ��ɫ
//	YawCode.color = 1+6*chassis.Yaw->canInfo.lostFlag;//����ʱ��ɫ��Y������ʱ��ɫ
//	myUInumLoad(&PitchCode,DisplayPack.pitchCode);
//	myUInumLoad(&YawCode,chassis.Yaw->canInfo.encoder);
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&PitchCode,15);
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&YawCode,15);

	/*Ħ����ת��*/
//	FriLSpd.operateType = 1 + !UI_InitFlag;
//	FriRSpd.operateType = 1 + !UI_InitFlag;
//	FriLSpd.color = 6+OffsetPack.FriLOffline;//����ʱ��ɫ����Ħ��������ʱ��ɫ
//	FriRSpd.color = 6+OffsetPack.FriROffline;//����ʱ��ɫ����Ħ��������ʱ��ɫ
//	myUInumLoad(&FriLSpd,DisplayPack.friLSpd);
//	myUInumLoad(&FriRSpd,DisplayPack.friRSpd);
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&FriLSpd,15);
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&FriRSpd,15);
	
	judgement.graphicDraw(i);
}


//�Ե�ƵUI���б������ơ�����
//����:���ݸ��ꡢ�����ƽ�����ո�״̬������λ�á�������������
//֡λ7/7
void LowFreqUI_Update_1(void)
{
	u8 i = 0;
	
	/*���ݸ���*/
	static uint8_t bsCnt;
	bsCnt = !bsCnt;
	Cap_Limit.operateType = 1 + !UI_InitFlag;
	Cap_Limit.startX = CAPINFO_X_BASE+6*power.powerDownLimit;
	Cap_Limit.endX = Cap_Limit.startX+6*power.powerLimitRange;
	Cap_Limit.width = 4-power.speedUpLevel*bsCnt*2;//����ʱ����
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Cap_Limit,15);
	
	/*�����ƽ*/
//	AttitudeLine.operateType = 1 + !UI_InitFlag;
//	AttitudeLine.startY = ATTITUDE_Y_BASE + chassis.os->pitchFb*10 + chassis.os->rollFb*5;
//	AttitudeLine.endY = ATTITUDE_Y_BASE + chassis.os->pitchFb*10 - chassis.os->rollFb*5;
//	if(ABS(chassis.os->pitchFb)>POOLGUY_THRESHOUD)//�˽�
//		AttitudeLine.color = 5;
//	else if(ABS(chassis.os->pitchFb)>STRONG_BALANCE_AGL)//����ƽ��
//		AttitudeLine.color = 3;
//	else//����
//		AttitudeLine.color = 8;
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&AttitudeLine,15);
	
	/*���ո�״̬*/
//	CoverFlag.operateType = 1 + !UI_InitFlag;
//	CoverFlag.width = DisplayPack.coverState?3:0;
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&CoverFlag,15);
	
	/*����λ��*/
//	FeedPos.operateType = 1 + !UI_InitFlag;
//	FeedPos.startX = 12*my_sin(DisplayPack.feedPos*60*RAD_PER_DEG) + FEED_X_BASE;
//	FeedPos.startY = 12*my_cos(DisplayPack.feedPos*60*RAD_PER_DEG) + FEED_Y_BASE;
//	FeedPos.color = (DisplayPack.heatFreeState == 2)?4:((DisplayPack.heatFreeState == 1)?3:6);//0��-�� 1��-�� 2���-�Ϻ�
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&FeedPos,15);
	
	/*������*/
//	Wheel_Flag.operateType = 1 + !UI_InitFlag;
//	if(chassis.wheel[0]->motorOfflineFlag && chassis.wheel[1]->motorOfflineFlag)
//		Wheel_Flag.color = 7;
//	else if(chassis.wheel[0]->motorOfflineFlag)
//		Wheel_Flag.color = 4;
//	else if(chassis.wheel[1]->motorOfflineFlag)
//		Wheel_Flag.color = 5;
//	else
//		Wheel_Flag.color = 8;
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Wheel_Flag,15);
	
	/*����[��ʵ��Ť��]*/
//	SpeedMeterSpeed.operateType = 1 + !UI_InitFlag;
//	myUInumLoad(&SpeedMeterSpeed,ABS(chassis.wheel[0]->feedbackPack.iq_t) + ABS(chassis.wheel[1]->feedbackPack.iq_t));
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&SpeedMeterSpeed,15);
	
	/*�ؽڵ���¶���ʾ*/
//	float ttmmpp = chassis.leg[0]->motor[0]->MOTOR_recv.Temp;
//	ttmmpp += chassis.leg[0]->motor[1]->MOTOR_recv.Temp;
//	ttmmpp += chassis.leg[1]->motor[0]->MOTOR_recv.Temp;
//	ttmmpp += chassis.leg[1]->motor[1]->MOTOR_recv.Temp;
//	ttmmpp /= 4.0f;
	
//	MotorTempLine.operateType = 1 + !UI_InitFlag;
//	if(ttmmpp < 40)
//		MotorTempLine.color = 6;//�¶�С��40����ˬ
//	else if(ttmmpp < 60)
//		MotorTempLine.color = 2;//�¶�С��60������
//	else if(ttmmpp < 100)
//		MotorTempLine.color = 3;//�¶�С��100������
//	else
//		MotorTempLine.color = 4;//�¶ȴ���100������
//	MotorTempLine.startAngle   = LIMIT(135 - (int)((ttmmpp-30)/5*9),46,134);//30��~80�� -> 135~45
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&MotorTempLine,15);
	
	judgement.graphicDraw(i);
}


//�Ե�ƵUI���б������ơ��ϡ�
//����:�������
//����ˢ��Ƶ��
//֡λ7/7
void LowFreqUI_Update_2(void)
{
	u8 i = 0;
	static uint8_t ld;

	/*�������*/
	//LV.5|LV.4��ɫ LV.3��ɫ LV.2|LV.1��ɫ LV.0��ʧ
	
	Battery_Line_1.operateType = 1 + !UI_InitFlag;
	Battery_Line_2.operateType = 1 + !UI_InitFlag;
	Battery_Line_3.operateType = 1 + !UI_InitFlag;
	Battery_Line_4.operateType = 1 + !UI_InitFlag;
	
	Battery_Line_1.color = (judgement.batteryLevel>3)?2:((judgement.batteryLevel>2)?3:((judgement.batteryLevel>1)?4:7));
	Battery_Line_2.color = Battery_Line_1.color;
	Battery_Line_3.color = Battery_Line_1.color;
	Battery_Line_4.color = Battery_Line_1.color;

	Battery_Line_1.width = (judgement.batteryLevel>4)?15:0;
	Battery_Line_2.width = (judgement.batteryLevel>3)?15:0;
	Battery_Line_3.width = (judgement.batteryLevel>2)?15:0;
	Battery_Line_4.width = (judgement.batteryLevel>1)?15:0;
	if(judgement.batteryLevel == 1)
	{
		ld = !ld;
		Battery_Line_1.width = 15*ld;
		Battery_Line_2.width = Battery_Line_1.width;
		Battery_Line_3.width = Battery_Line_1.width;
		Battery_Line_4.width = Battery_Line_1.width;
	}
	
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Battery_Line_1,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Battery_Line_2,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Battery_Line_3,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Battery_Line_4,15);
	
	
	/* �ؽڵ���¶���ʾ */
//	float ttmmpp = chassis.leg[0]->motor[0]->MOTOR_recv.Temp;
//	ttmmpp += chassis.leg[0]->motor[1]->MOTOR_recv.Temp;
//	ttmmpp += chassis.leg[1]->motor[0]->MOTOR_recv.Temp;
//	ttmmpp += chassis.leg[1]->motor[1]->MOTOR_recv.Temp;
//	ttmmpp /= 4.0f;
//	
//	if(ttmmpp < 40)
//		MotorTemperature.color = 6;//�¶�С��40����ˬ
//	else if(ttmmpp < 60)
//		MotorTemperature.color = 2;//�¶�С��60������
//	else if(ttmmpp < 100)
//		MotorTemperature.color = 3;//�¶�С��100������
//	else
//		MotorTemperature.color = 4;//�¶ȴ���100������
//	
//	MotorTemperature.operateType = 1 + !UI_InitFlag;
//	myUInumLoad(&MotorTemperature,(int)(ttmmpp*1000));
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&MotorTemperature,15);
//	
//	/*��ǰƫ������ʾ*/
//	IdlerID.operateType = 1 + !UI_InitFlag;
//	IdlerID.color = OffsetPack.whatWheelCtrl + 1;
//	switch(OffsetPack.whatWheelCtrl)
//	{
//		case 0:
//			myUInumLoad(&IdlerID,(int)(810000000));
//			break;
//		case 1:
//			myUInumLoad(&IdlerID,(int)(801000000));
//			break;
//		case 2:
//			myUInumLoad(&IdlerID,(int)(800100000));
//			break;
//		case 3:
//			myUInumLoad(&IdlerID,(int)(800010000));
//			break;
//		case 4:
//			myUInumLoad(&IdlerID,(int)(800001000));
//			break;
//		case 5:
//			myUInumLoad(&IdlerID,(int)(800000100));
//			break;
//		case 6:
//			myUInumLoad(&IdlerID,(int)(800000010));
//			break;
//		case 7:
//			myUInumLoad(&IdlerID,(int)(800000001));
//			break;
//	}
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&IdlerID,15);
	
	/*��ǰƫ��ֵ��ʾ*/
//	IdlerValue.operateType = 1 + !UI_InitFlag;
//	IdlerValue.color = OffsetPack.whatWheelCtrl + 1;
//	switch(OffsetPack.whatWheelCtrl)
//	{
//		case 0:	//���̸߶�
//			myUInumLoad(&IdlerValue,(int)(chassis.ci->heightSet * 1000));
//			break;
//		case 1: //����Roll��
//			myUInumLoad(&IdlerValue,(int)(chassis.ci->rollSet * 1000));
//			break;
//		case 2: //���P��
//			myUInumLoad(&IdlerValue,(int)(OffsetPack.BuffPOffset * 1000));
//			break;
//		case 3: //���Y��
//			myUInumLoad(&IdlerValue,(int)(OffsetPack.BuffYOffset * 1000));
//			break;
//		case 4://����P��
//			myUInumLoad(&IdlerValue,(int)(OffsetPack.VisPOffset * 1000));
//			break;
//		case 5://����Y��
//			myUInumLoad(&IdlerValue,(int)(OffsetPack.VisYOffset * 1000));
//			break;
//		case 6:
//			myUInumLoad(&IdlerValue,(int)(0));
//			break;
//		case 7:
//			myUInumLoad(&IdlerValue,(int)(0));
//			break;
//	}
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&IdlerValue,15);

	judgement.graphicDraw(i);
}


//�Գ�פ��̬UI���б�������
//����:���
void StaticUI_Update_0(void)
{
	u8 i = 0;
	
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Shotable_L0	,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Shotable_L1	,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Shotable_L2	,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Shotable_L3	,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Shotable_L4	,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Shotable_L5	,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Shotable_L6	,15);
	
	judgement.graphicDraw(i);
}


//�Գ�פ��̬UI���б�������
//����:ʾ���ߡ��Ӿ���Ұ��������򡢱�����������̬����򡢲������
void StaticUI_Update_1(void)
{
	u8 i = 0;
	
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&WidthLine_L,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&WidthLine_R,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Cap_Border,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Cap_BackPack,15);
//	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&AttitudeBall,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&FeedFrame,15);
	
	judgement.graphicDraw(i);
}


//�Գ�פ��̬UI���б�������
//����:�Ǳ�����Ǳ�̶�p1
void StaticUI_Update_2(void)
{
	u8 i = 0;

	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&SpeedMeterFrame,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&SpeedMeterScale1,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&SpeedMeterScale2,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&SpeedMeterScale3,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&SpeedMeterScale4,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&SpeedMeterScale5,15);	
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&SpeedMeterScale6,15);	

	judgement.graphicDraw(i);
}


//�Գ�פ��̬UI���б�������
//����:�Ǳ�̶�p2��������򡢵���С�򡢸������
void StaticUI_Update_3(void)
{
	u8 i = 0;

	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&SpeedMeterScale7,15);	
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&SpeedMeterScale8,15);	
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Battery_Border_1,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Battery_Border_2,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Follow_Frame,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&VisionFrame,15);
	
	judgement.graphicDraw(i);
}


//�Գ�פ��̬UI���б�������
//����:������״
void StaticUI_Update_4(void)
{
	u8 i = 0;

	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Idler_Frame,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Idler_Line1,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Idler_Line2,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Idler_Line3,15);
	memcpy(judgement.sendUnionData.graphicDataStruct+i++,&Idler_Line4,15);
	
	judgement.graphicDraw(i);
}


////�Գ�פ��̬UI���б�������
////����:�������ֳ�ʼ��
//void CharUI_Init_0(void)
//{
//	IdlerInfo.graphic_data_struct.operateType = 1;
//	Chars_Update(IdlerInfo);
//}


//�Գ�פ��̬UI���б�������
//����:״̬���ֳ�ʼ��
void CharUI_Init_1(void)
{
	ChssisState.graphic_data_struct.operateType = 1;
	Chars_Update(ChssisState);
}


////������UI���б�������
////����:�Ƿ��������UI����
////����֡��������ʾ��Ϣ
//bool ConditionUI_Update_0(void)
//{
//	static uint8_t lastWheelCtrl;
//	u8 tmp = 0;
//	
//	if(lastWheelCtrl-OffsetPack.whatWheelCtrl)
//	{
//		switch(OffsetPack.whatWheelCtrl)
//		{
//			case 0:
//				memcpy(IdlerInfo.data,"HEIGHT   ",9);
//				break;
//			case 1:
//				memcpy(IdlerInfo.data,"ROLL     ",9);
//				break;
//			case 2:
//				memcpy(IdlerInfo.data,"OFFSET   ",9);
//				break;
//			case 3:
//				memcpy(IdlerInfo.data,"SIBALAXI ",9);
//				break;
//		}
//		IdlerInfo.graphic_data_struct.operateType = 2;
//		Chars_Update(IdlerInfo);
//		tmp = 1;
//	}
//	lastWheelCtrl = OffsetPack.whatWheelCtrl;
//	return tmp;
//}


//������UI���б�������
//����:�Ƿ��������UI����
//����֡������״̬
bool ConditionUI_Update_1(void)
{
//	static Ctrler::_chassisState lastChassisState;
//	u8 tmp = 0;
//	
//	if(lastChassisState-chassis.chassisState)
//	{
//		switch(chassis.chassisState)
//		{
//			case Ctrler::DEFORCE:
//				memcpy(ChssisState.data,"DEFORCE   ",10);
//				ChssisState.graphic_data_struct.color = 7;
//				break;
//			case Ctrler::LEG_INPUT:
//				memcpy(ChssisState.data,"LEG_INPUT ",10);
//				ChssisState.graphic_data_struct.color = 8;
//				break;
//			case Ctrler::STAND_UP:
//				memcpy(ChssisState.data,"STAND_UP  ",10);
//				ChssisState.graphic_data_struct.color = 3;
//				break;
//			case Ctrler::BALANCE:
//				memcpy(ChssisState.data,"BALANCE   ",10);
//				ChssisState.graphic_data_struct.color = 2;
//				break;
//			case Ctrler::POOR_GUY:
//				memcpy(ChssisState.data,"POOR_GUY  ",10);
//				ChssisState.graphic_data_struct.color = 4;
//				break;
//			case Ctrler::IN_AIR:
//				memcpy(ChssisState.data,"IN_AIR    ",10);
//				ChssisState.graphic_data_struct.color = 6;
//				break;
//		}
//		ChssisState.graphic_data_struct.operateType = 2;
//		Chars_Update(ChssisState);
//		tmp = 1;
//	}
//	lastChassisState = chassis.chassisState;
	return 1;
}

/********************************************************************************************/

/**
 *	@breif UI�����ܵ��� 
 */
void Draw_Task(void)//30msִ��һ��
{
	static uint8_t ringID;
	
	if(judgement.gameStatus.gameProgress == 4)//��ս�н������ֶ���ʼ��
		UI_InitFlag = CtrlPack.UI_Init;
	else if(judgement.gameStatus.gameProgress == 3)//5s����ʱʱǿ�Ƴ�ʼ��
		UI_InitFlag = 1;
	else			//�ǶԾ��п����Զ���ʼ����ÿ30s����1s�ĳ�ʼ��
		UI_InitFlag = !!(CtrlPack.UI_Init+((uint32_t)(sysTickUptime/1e3f)%30<2));
	
	if(UI_InitFlag)
	{
		switch(ringID++)//����UIˢ��
		{	
			//0֡����������1��ʼ
			case 0:
			case 1:
				HighFreqUI_Update_0();//11Hz
				break;
			case 2:
				HighFreqUI_Update_1();//11Hz
				break;
			case 3:
				LowFreqUI_Update_0();//2Hz
				break;
			case 4:
				LowFreqUI_Update_1();//5Hz
				break;
			case 5://��Ȼ֡ĩ֡
				LowFreqUI_Update_2();//1Hz
				break;
			case 6:
				StaticUI_Update_0();
				break;
			case 7:
				StaticUI_Update_1();
				break;
			case 8:
				StaticUI_Update_2();
				break;
			case 9:
				StaticUI_Update_3();
				break;
			case 10:
				StaticUI_Update_4();
				break;
			case 11:
//				CharUI_Init_0();
			case 12:
			default:
				CharUI_Init_1();
				ringID = 0;
		}
	}
	else
	{
		/*****************�ж�֡*****************///���ȼ���ߣ������ϳ���֡�������
//		if(ConditionUI_Update_0())return;
//		if(ConditionUI_Update_1())return;
		
		switch(ringID++)//����UIˢ��
		{	
		/*****************ˢ��֡*****************/
			//0֡����������1��ʼ
			case 0:
			case 4:
			case 7:
			case 10:
			case 12:
			case 13:
			case 16:
			case 19:
			case 22:
			case 25:
			case 28:
			case 1:
				HighFreqUI_Update_0();//11Hz
				break;
			case 5:
			case 8:
			case 11:
			case 14:
			case 17:
			case 20:
			case 23:
			case 24:
			case 26:
			case 29:
			case 2:
				HighFreqUI_Update_1();//11Hz
				break;
			case 9:
			case 15:
			case 21:
			case 27:
			case 3:
				LowFreqUI_Update_0();//2Hz
				break;
			case 18:
			case 6:
				LowFreqUI_Update_1();//5Hz
				break;
			case 30://��Ȼ֡ĩ֡
			default:
				LowFreqUI_Update_2();//1Hz
				ringID = 0;
		}
	}
}


//�ַ���ˢ��
void Chars_Update(Judgement::ClientCustomCharacter Demo_Chars)
{
	memcpy(&judgement.sendUnionData.clientCustomCharacter,&Demo_Chars,45);
	judgement.characterDraw();
}


//�������
void myUInumLoad(Judgement::GraphicDataStruct *GraphicDataStruct,int32_t num)
{
	GraphicDataStruct->endY   = (num>>21) & 0X7FF;//��11λ
	GraphicDataStruct->endX   = (num>>10) & 0X7FF;//��11λ
	GraphicDataStruct->radius =  num & 0X3FF;// ĩ10λ
}


//	judgement.graphicDraw(7);/// ����sendGraphics��ͼ��
//	judgement.characterDraw();/// �����ַ���
//	judgement.graphicDel();/// ɾ��ͼ��
/**
 *	judgement.sendUnionData.xxx
 *							graphicDataStruct[7];//ͼ�λ���ģʽ
 *							clientCustomCharacter;//�ַ�����ģʽ
 *							clientCustomGraphicDelete;//ͼ��ɾ��ģʽ
 */

/****************************************************************************/

//�õ�Ŀ��ʣ��Ѫ��
uint16_t getEnemyHP()
{
	uint8_t mycolor = judgement.gameRobotStatus.robotId<100;//�õ��Լ���ɫ
	if(DisplayPack.enemyID == 0)
		return 0;
	else
		return judgement.gameRobotHP.teamHp[mycolor][DisplayPack.enemyID-1];//����ID����ɫ���Ѫ��
}

