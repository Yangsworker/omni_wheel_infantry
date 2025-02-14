#include "Gimbal_task.h"
#include "com_task.h"
#include "myRC_task.h"
#include "dbus.h"
#include "vision.h"
#include "multi_imu.h"
#include "pid.h"

extern MultiImu *boardImu;

//Yaw轴电机ID、Pitch轴电机ID
Gimbal gimbal(0X207,0X206);

Motor *MT_motor;


PidParam ptzP_pidinner;
PidParam ptzP_pidouter;


//普通，跟随，打符，机械角
PidParam YawInner[PIDplanNum]={
	{230,0,0,100,30000},//手
	{300,0,0,1,30000},//静靶  //{250,0,0,1,30000},//静靶
	{250,0,0,1,30000},//陀螺
	{300,0,0,1000,30000} //开符
};
PidParam YawOuter[PIDplanNum]={
	{-15,-50,-0.6,0.2,1920},//手 
//	{-30,-500,-0.2,0.1,1920,0,0.5,0,1,1},  //打开微分先行
	{-21,-300,-0.4,0.08,1920,0,0,0,0,1}, 
	{-21,-100,-0.7,0.08,1920,0,0,0,0,1},
	{-19,-300,-0.05,0.05,1920,0,0,0,0,1}  //开符
};
PidParam PitchInner[PIDplanNum]={
	{220,0,0,1,30000},//手
	{250,0,0,1,30000},
	{280,10,0,1,30000},
	{300,0,0,0,30000}
};
PidParam PitchOuter[PIDplanNum]={
	{12,0,0,0.5,1920},//手
	{20,200,0.08,0.1,1920,0,0,0,0,1},//静靶
	{18,200,0.5,0.1,1920,0,0,0,0,1},  //陀螺
	{24,200,0.06,0.1,1920,0,0,0,0,1}   //开符
};
const float Gimbal::mechPitchLimit[2]= {-39,12}; //静态机械角限位[按SelfAlg值校准]
/**
 * @brief 馈值获取
*/
float s1, s2, s3, s4, s5;
void Gimbal::fbValUpdate()
{	
	s1 = eulerAlgSet[YAW];
	s2 = WorldAlg[YAW];
	s3 = eulerAlgSet[PITCH];
	s4 = WorldAlg[PITCH];
	s5 = vision_RecvStruct.beat;
	s1*=1; s2*=1;
	s3*=1; s4*=1;
	
	// 自身系反馈  （yaw-0  pitch-1）
	SelfDps [YAW] 	= Yaw->canInfo.speed;
	SelfDps [PITCH] = Pitch->canInfo.speed;
  SelfAlg [YAW]	  = Yaw->canInfo.totalAngle_f;	
	SelfAlg [PITCH] = Pitch->canInfo.totalAngle_f - 134.0f;//安装偏置(陀螺仪角度为0时pitch电机角度)
	//世界系反馈
	if(kfSwitch) //是否使用卡尔曼滤波
	{
		WorldDps[PITCH] = pitchDps.KalmanFilter(-1*boardImu->gyro.dps.data[0], pitchDpsProcessNoise_Q, pitchDpsMeasureNoise_R);
		WorldDps[YAW]	= yawDps.KalmanFilter(-1*boardImu->gyro.dps.data[1], yawDpsProcessNoise_Q, yawDpsMeasureNoise_R);
		WorldAlg[PITCH] = pitchAlg.KalmanFilter(boardImu->AHRS_data.Angle.pitch, pitchProcessNiose_Q, pitchMeasureNoise_R);
		WorldAlg[YAW]	= yawAlg.KalmanFilter(boardImu->AHRS_data.Angle.yaw, yawProcessNiose_Q, yawMeasureNoise_R);
	}
	else
	{
		WorldDps[PITCH] = boardImu->gyro.dps.data[0];
		WorldDps[YAW]	= boardImu->gyro.dps.data[1];
		WorldAlg[PITCH] = boardImu->AHRS_data.Angle.pitch;
		WorldAlg[YAW]	= boardImu->AHRS_data.Angle.yaw;
	}
	
	//pitch轴反馈值
	anglePitch=-(boardImu->AHRS_data.Angle.roll-90);
	pitchspeed=-boardImu->gyro.dps.data[0];
	
}
/**
 * @brief 遥控数据刷新
*/
void Gimbal::remoteCtrlUpdate()
{
	//控制模式选择(手动开启视觉并且视觉在线且发现目标-开始锁定)
	if((myRC.visionMode == 3) && !visionInfo.offlineFlag && !vision_RecvStruct.no_Obj)
		CtrlPlan = V_buff;     //打符
	else if((myRC.visionMode == 2) && !visionInfo.offlineFlag && !vision_RecvStruct.no_Obj)
		CtrlPlan = V_rotating; //反陀螺
	else if((myRC.visionMode == 1) && !visionInfo.offlineFlag && !vision_RecvStruct.no_Obj)
		CtrlPlan = V_static;   //击打静止/平移靶
	else
		CtrlPlan = Manual;     //手动

	//若模式切换，重置设定值
	if(CtrlPlan!=lastCtrlPlan||deforceFlag == 1)
	{
		mechAlgSet[PITCH] 	= SelfAlg[PITCH];
		mechAlgSet[YAW] 	= SelfAlg[YAW];
		eulerAlgSet[PITCH] 	= WorldAlg[PITCH];
		eulerAlgSet[YAW] 	= WorldAlg[YAW];
	}	
	//单云台电机丢失世界角设定值重置（防止云台离线疯车）
	if(Yaw->canInfo.lostFlag)
		eulerAlgSet[YAW] 	= WorldAlg[YAW];
	if(MT_motor->canInfo.lostFlag)
		eulerAlgSet[PITCH] 	= WorldAlg[PITCH];
	
	//根据模式更新设定值	
  if(CtrlPlan != Manual)
	{
		eulerAlgSet[YAW]   = vision_RecvStruct.Yaw;
		if(myRC.visionYawOnly)//只锁yaw，则设定值为手动值
			eulerAlgSet[PITCH] += myRC.mySpdSetP;
		else
			eulerAlgSet[PITCH] = -1*vision_RecvStruct.Pitch;
	}
	else
	{
		eulerAlgSet[YAW]  += myRC.mySpdSetZ;
		eulerAlgSet[PITCH]+= myRC.mySpdSetP;
	}
//	if(myRC.lookBack>lastLookBack && lookbackOK)//回头的触发
//	{
//		lookbackOK = 0;
//	}
//	else if(lookbackOK)//等待回头指令
//	{
//		lookbackBase = WorldAlg[YAW];//刷新基准角度
//		lookback.Clear();//清曲线
//	}
//	else//回头中
//	{
//		if(lookback.CurveModel(lookbackBase+180, &eulerAlgSet[YAW], LOOK_BACK_TIME))//计算完成，认为回头结束[预设旋转1s]
//			lookbackOK = 1;
//	}
//	lastLookBack = myRC.lookBack;
	

//pitch限位
		limitEulerPitch[0] = (mechPitchLimit[0] - SelfAlg[PITCH] + WorldAlg[PITCH]);  
		limitEulerPitch[1] = (mechPitchLimit[1] - SelfAlg[PITCH] + WorldAlg[PITCH]);
		eulerAlgSet[PITCH] = LIMIT(eulerAlgSet[PITCH],-40,28);  //LIMIT(val, min, max)
}


/**
 * @brief 电机控制
*/



//mt电机反馈值获取函数
void MT_getFabervalue(void)
{
	 MT_GetInfomation(MT_motor);
}


void Gimbal::motorCtrl(void)
{	
	Yaw  ->ctrlPosition(eulerAlgSet[YAW],CtrlPlan);
//	Pitch->ctrlPosition(eulerAlgSet[PITCH],CtrlPlan);
}

/**
 * @brief 云台类构造器
*/
Gimbal::Gimbal(uint16_t YawID,uint16_t PitchID)
{
	MT_motor=new Motor(GM6020,CAN2,0x141);
	MT_Motor_Init(MT_motor,CAN2,0x141);
	
	//p轴
	ptzP_pidouter.kp=20;
	ptzP_pidouter.ki=20;
	ptzP_pidouter.kd=0;
	ptzP_pidouter.integralErrorMax=0.3;
	ptzP_pidouter.resultMax=2000;
	
	ptzP_pidinner.kp=8;
	ptzP_pidinner.ki=0;
	ptzP_pidinner.kd=0;
	ptzP_pidinner.integralErrorMax=100;
	ptzP_pidinner.resultMax=5000;
	
	Yaw 	= new Motor(GM6020,CAN1,YawID);
	Pitch 	= new Motor(GM6020,CAN2,PitchID);
	//方案数设定
	Yaw  ->pidInner.setPlanNum(PIDplanNum);
	Yaw  ->pidOuter.setPlanNum(PIDplanNum);
	Pitch->pidInner.setPlanNum(PIDplanNum);
	Pitch->pidOuter.setPlanNum(PIDplanNum);
	//参源设定
	Yaw  ->pidInner.paramPtr = YawInner;
	Yaw  ->pidOuter.paramPtr = YawOuter;
	Pitch->pidInner.paramPtr = PitchInner;
	Pitch->pidOuter.paramPtr = PitchOuter;
	//馈源设定
	Yaw  ->pidInner.fbValuePtr[Manual] = &WorldDps[YAW]; //操作手自由模式
	Yaw  ->pidOuter.fbValuePtr[Manual] = &WorldAlg[YAW];
	Pitch->pidInner.fbValuePtr[Manual] = &WorldDps[PITCH];
	Pitch->pidOuter.fbValuePtr[Manual] = &WorldAlg[PITCH];
	
	Yaw  ->pidInner.fbValuePtr[V_static] = &WorldDps[YAW]; //视觉跟随模式
	Yaw  ->pidOuter.fbValuePtr[V_static] = &WorldAlg[YAW];
	Pitch->pidInner.fbValuePtr[V_static] = &WorldDps[PITCH];
	Pitch->pidOuter.fbValuePtr[V_static] = &WorldAlg[PITCH];
	
	Yaw  ->pidInner.fbValuePtr[V_rotating] = &WorldDps[YAW];   //视觉打符模式
	Yaw  ->pidOuter.fbValuePtr[V_rotating] = &WorldAlg[YAW];
	Pitch->pidInner.fbValuePtr[V_rotating] = &WorldDps[PITCH];
	Pitch->pidOuter.fbValuePtr[V_rotating] = &WorldAlg[PITCH];

	Yaw  ->pidInner.fbValuePtr[V_buff] = &WorldDps[YAW]; //机械角度模式
	Yaw  ->pidOuter.fbValuePtr[V_buff] = &WorldAlg[YAW];
	Pitch->pidInner.fbValuePtr[V_buff] = &WorldDps[PITCH];
	Pitch->pidOuter.fbValuePtr[V_buff] = &WorldAlg[PITCH];
  
	
	//p轴的云台
	MT_motor->setZeroValue(5315);  
  //设定  
  MT_motor->pidOuter.setPlanNum(PIDplanNum);
	MT_motor->pidInner.setPlanNum(PIDplanNum);  
	//写入pid
	MT_motor->pidOuter.paramPtr=&ptzP_pidouter;
	MT_motor->pidInner.paramPtr=&ptzP_pidinner;
	//反馈值
  MT_motor->pidOuter.fbValuePtr[0]=&gimbal.anglePitch;

  MT_motor->pidInner.fbValuePtr[0]=&gimbal.pitchspeed;
	
	

	yawProcessNiose_Q = 0.05; //Y过程噪声
	yawMeasureNoise_R = 1; //Y测量噪声
	pitchProcessNiose_Q = 0.1; //P过程噪声
	pitchMeasureNoise_R = 20; //P测量噪声
	yawDpsProcessNoise_Q = 1; //Y过程噪声
	yawDpsMeasureNoise_R = 1; //Y测量噪声
	pitchDpsProcessNoise_Q = 1; //P过程噪声
	pitchDpsMeasureNoise_R = 1; //P测量噪声
	kfSwitch = 1; //默认使用卡尔曼滤波

	CtrlPlan = Manual; //默认控制模式是操作手自由控制
}


/**
 * @brief 云台控制任务
*/
void gimbal_task(void)
{
	MT_getFabervalue();
	gimbal.fbValUpdate();//馈值解算
	gimbal.remoteCtrlUpdate();//遥控解算
	gimbal.motorCtrl();//电机控制
	
	gimbal.lastCtrlPlan = gimbal.CtrlPlan;
	gimbal.lastDeforce = deforceFlag;
}
