#include "Gimbal_task.h"
#include "com_task.h"
#include "myRC_task.h"
#include "dbus.h"
#include "vision.h"
#include "multi_imu.h"
#include "pid.h"

extern MultiImu *boardImu;

//Yaw����ID��Pitch����ID
Gimbal gimbal(0X207,0X206);

Motor *MT_motor;


PidParam ptzP_pidinner;
PidParam ptzP_pidouter;


//��ͨ�����棬�������е��
PidParam YawInner[PIDplanNum]={
	{230,0,0,100,30000},//��
	{300,0,0,1,30000},//����  //{250,0,0,1,30000},//����
	{250,0,0,1,30000},//����
	{300,0,0,1000,30000} //����
};
PidParam YawOuter[PIDplanNum]={
	{-15,-50,-0.6,0.2,1920},//�� 
//	{-30,-500,-0.2,0.1,1920,0,0.5,0,1,1},  //��΢������
	{-21,-300,-0.4,0.08,1920,0,0,0,0,1}, 
	{-21,-100,-0.7,0.08,1920,0,0,0,0,1},
	{-19,-300,-0.05,0.05,1920,0,0,0,0,1}  //����
};
PidParam PitchInner[PIDplanNum]={
	{220,0,0,1,30000},//��
	{250,0,0,1,30000},
	{280,10,0,1,30000},
	{300,0,0,0,30000}
};
PidParam PitchOuter[PIDplanNum]={
	{12,0,0,0.5,1920},//��
	{20,200,0.08,0.1,1920,0,0,0,0,1},//����
	{18,200,0.5,0.1,1920,0,0,0,0,1},  //����
	{24,200,0.06,0.1,1920,0,0,0,0,1}   //����
};
const float Gimbal::mechPitchLimit[2]= {-39,12}; //��̬��е����λ[��SelfAlgֵУ׼]
/**
 * @brief ��ֵ��ȡ
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
	
	// ����ϵ����  ��yaw-0  pitch-1��
	SelfDps [YAW] 	= Yaw->canInfo.speed;
	SelfDps [PITCH] = Pitch->canInfo.speed;
  SelfAlg [YAW]	  = Yaw->canInfo.totalAngle_f;	
	SelfAlg [PITCH] = Pitch->canInfo.totalAngle_f - 134.0f;//��װƫ��(�����ǽǶ�Ϊ0ʱpitch����Ƕ�)
	//����ϵ����
	if(kfSwitch) //�Ƿ�ʹ�ÿ������˲�
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
	
	//pitch�ᷴ��ֵ
	anglePitch=-(boardImu->AHRS_data.Angle.roll-90);
	pitchspeed=-boardImu->gyro.dps.data[0];
	
}
/**
 * @brief ң������ˢ��
*/
void Gimbal::remoteCtrlUpdate()
{
	//����ģʽѡ��(�ֶ������Ӿ������Ӿ������ҷ���Ŀ��-��ʼ����)
	if((myRC.visionMode == 3) && !visionInfo.offlineFlag && !vision_RecvStruct.no_Obj)
		CtrlPlan = V_buff;     //���
	else if((myRC.visionMode == 2) && !visionInfo.offlineFlag && !vision_RecvStruct.no_Obj)
		CtrlPlan = V_rotating; //������
	else if((myRC.visionMode == 1) && !visionInfo.offlineFlag && !vision_RecvStruct.no_Obj)
		CtrlPlan = V_static;   //����ֹ/ƽ�ư�
	else
		CtrlPlan = Manual;     //�ֶ�

	//��ģʽ�л��������趨ֵ
	if(CtrlPlan!=lastCtrlPlan||deforceFlag == 1)
	{
		mechAlgSet[PITCH] 	= SelfAlg[PITCH];
		mechAlgSet[YAW] 	= SelfAlg[YAW];
		eulerAlgSet[PITCH] 	= WorldAlg[PITCH];
		eulerAlgSet[YAW] 	= WorldAlg[YAW];
	}	
	//����̨�����ʧ������趨ֵ���ã���ֹ��̨���߷賵��
	if(Yaw->canInfo.lostFlag)
		eulerAlgSet[YAW] 	= WorldAlg[YAW];
	if(MT_motor->canInfo.lostFlag)
		eulerAlgSet[PITCH] 	= WorldAlg[PITCH];
	
	//����ģʽ�����趨ֵ	
  if(CtrlPlan != Manual)
	{
		eulerAlgSet[YAW]   = vision_RecvStruct.Yaw;
		if(myRC.visionYawOnly)//ֻ��yaw�����趨ֵΪ�ֶ�ֵ
			eulerAlgSet[PITCH] += myRC.mySpdSetP;
		else
			eulerAlgSet[PITCH] = -1*vision_RecvStruct.Pitch;
	}
	else
	{
		eulerAlgSet[YAW]  += myRC.mySpdSetZ;
		eulerAlgSet[PITCH]+= myRC.mySpdSetP;
	}
//	if(myRC.lookBack>lastLookBack && lookbackOK)//��ͷ�Ĵ���
//	{
//		lookbackOK = 0;
//	}
//	else if(lookbackOK)//�ȴ���ͷָ��
//	{
//		lookbackBase = WorldAlg[YAW];//ˢ�»�׼�Ƕ�
//		lookback.Clear();//������
//	}
//	else//��ͷ��
//	{
//		if(lookback.CurveModel(lookbackBase+180, &eulerAlgSet[YAW], LOOK_BACK_TIME))//������ɣ���Ϊ��ͷ����[Ԥ����ת1s]
//			lookbackOK = 1;
//	}
//	lastLookBack = myRC.lookBack;
	

//pitch��λ
		limitEulerPitch[0] = (mechPitchLimit[0] - SelfAlg[PITCH] + WorldAlg[PITCH]);  
		limitEulerPitch[1] = (mechPitchLimit[1] - SelfAlg[PITCH] + WorldAlg[PITCH]);
		eulerAlgSet[PITCH] = LIMIT(eulerAlgSet[PITCH],-40,28);  //LIMIT(val, min, max)
}


/**
 * @brief �������
*/



//mt�������ֵ��ȡ����
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
 * @brief ��̨�๹����
*/
Gimbal::Gimbal(uint16_t YawID,uint16_t PitchID)
{
	MT_motor=new Motor(GM6020,CAN2,0x141);
	MT_Motor_Init(MT_motor,CAN2,0x141);
	
	//p��
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
	//�������趨
	Yaw  ->pidInner.setPlanNum(PIDplanNum);
	Yaw  ->pidOuter.setPlanNum(PIDplanNum);
	Pitch->pidInner.setPlanNum(PIDplanNum);
	Pitch->pidOuter.setPlanNum(PIDplanNum);
	//��Դ�趨
	Yaw  ->pidInner.paramPtr = YawInner;
	Yaw  ->pidOuter.paramPtr = YawOuter;
	Pitch->pidInner.paramPtr = PitchInner;
	Pitch->pidOuter.paramPtr = PitchOuter;
	//��Դ�趨
	Yaw  ->pidInner.fbValuePtr[Manual] = &WorldDps[YAW]; //����������ģʽ
	Yaw  ->pidOuter.fbValuePtr[Manual] = &WorldAlg[YAW];
	Pitch->pidInner.fbValuePtr[Manual] = &WorldDps[PITCH];
	Pitch->pidOuter.fbValuePtr[Manual] = &WorldAlg[PITCH];
	
	Yaw  ->pidInner.fbValuePtr[V_static] = &WorldDps[YAW]; //�Ӿ�����ģʽ
	Yaw  ->pidOuter.fbValuePtr[V_static] = &WorldAlg[YAW];
	Pitch->pidInner.fbValuePtr[V_static] = &WorldDps[PITCH];
	Pitch->pidOuter.fbValuePtr[V_static] = &WorldAlg[PITCH];
	
	Yaw  ->pidInner.fbValuePtr[V_rotating] = &WorldDps[YAW];   //�Ӿ����ģʽ
	Yaw  ->pidOuter.fbValuePtr[V_rotating] = &WorldAlg[YAW];
	Pitch->pidInner.fbValuePtr[V_rotating] = &WorldDps[PITCH];
	Pitch->pidOuter.fbValuePtr[V_rotating] = &WorldAlg[PITCH];

	Yaw  ->pidInner.fbValuePtr[V_buff] = &WorldDps[YAW]; //��е�Ƕ�ģʽ
	Yaw  ->pidOuter.fbValuePtr[V_buff] = &WorldAlg[YAW];
	Pitch->pidInner.fbValuePtr[V_buff] = &WorldDps[PITCH];
	Pitch->pidOuter.fbValuePtr[V_buff] = &WorldAlg[PITCH];
  
	
	//p�����̨
	MT_motor->setZeroValue(5315);  
  //�趨  
  MT_motor->pidOuter.setPlanNum(PIDplanNum);
	MT_motor->pidInner.setPlanNum(PIDplanNum);  
	//д��pid
	MT_motor->pidOuter.paramPtr=&ptzP_pidouter;
	MT_motor->pidInner.paramPtr=&ptzP_pidinner;
	//����ֵ
  MT_motor->pidOuter.fbValuePtr[0]=&gimbal.anglePitch;

  MT_motor->pidInner.fbValuePtr[0]=&gimbal.pitchspeed;
	
	

	yawProcessNiose_Q = 0.05; //Y��������
	yawMeasureNoise_R = 1; //Y��������
	pitchProcessNiose_Q = 0.1; //P��������
	pitchMeasureNoise_R = 20; //P��������
	yawDpsProcessNoise_Q = 1; //Y��������
	yawDpsMeasureNoise_R = 1; //Y��������
	pitchDpsProcessNoise_Q = 1; //P��������
	pitchDpsMeasureNoise_R = 1; //P��������
	kfSwitch = 1; //Ĭ��ʹ�ÿ������˲�

	CtrlPlan = Manual; //Ĭ�Ͽ���ģʽ�ǲ��������ɿ���
}


/**
 * @brief ��̨��������
*/
void gimbal_task(void)
{
	MT_getFabervalue();
	gimbal.fbValUpdate();//��ֵ����
	gimbal.remoteCtrlUpdate();//ң�ؽ���
	gimbal.motorCtrl();//�������
	
	gimbal.lastCtrlPlan = gimbal.CtrlPlan;
	gimbal.lastDeforce = deforceFlag;
}
