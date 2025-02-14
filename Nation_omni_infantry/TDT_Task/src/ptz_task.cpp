#include "ptz_task.h"
#include "dbus.h"
#include "my_math.h"
#include "vision.h"
#include "state_task.h"
#include "flash_var.h"

/* 
* ��������֮ǰ��V4���أ�����pitch��ദ��roll���棬��δ�����޸ģ�ע��    
*/
float send_roll_offset = 0;  //�Ӿ�����ֵ����
 

PidParam YawInner[PIDplanNum], YawOuter[PIDplanNum],
         RollInner[PIDplanNum], RollOuter[PIDplanNum],
				 LeftInner, LeftOuter;
PTZ ptz;
VisionMode_t VM; //�Ӿ�ģʽö��

u8 ptz_error[4] = {100, 100, 100, 100};

//******************������*************************//
u8 kfSwitch = 1;
Kf visionYawAngle, visionPitchAngle;//����������Ӿ��Ƕȶ���
Lpf2p fbFilter_Yaw, fbFilter_Pitch;//��ͨ�˲�����Ӿ��Ƕȶ���
//������״̬
#define OPERATOR_NORMAL 0	 //����ģʽ
#define OPERATOR_FINE_TUNE 1 //΢��ģʽ
#define OPERATOR_SILENCE 2	 //��Ĭģʽ
//*************************************************//

float YawStick_Sensitivity = -0.001f; //yҡ��������
float RollStick_Sensitivity =  -0.001f; //rҡ��������
float YawMouse_Sensitivity = -0.003f;//y���������
float RollMouse_Sensitivity = -0.003f;//r���������

void Ptz_task(void)
{  
  ptz.fbValUpdate();   //����ˢ��
  ptz.ModeJudge_PTZ(); //��̨ģʽѡ��
	ptz.Modeswitch_run();//��̨�˶�����
	ptz.getautofire();   //��ȡ�Ƿ�����Զ�����
}

void PTZ::Ptz_Task_Init(void)//����yaw, roll���� 0x206  0x206
 {
	//������
	Yaw     = new Motor (GM6020, CAN1, 0x205);
	Roll    = new Motor (GM6020, CAN2, 0x207);
	//������ֵ
	YawInner[Free].kp = 200;  //{150,600,0.01,1,16380}
	YawInner[Free].ki = 600;
	YawInner[Free].kd = 0;
	YawInner[Free].integralErrorMax = 0;
	YawInner[Free].resultMax = Yaw-> getMotorCurrentLimit();
	YawInner[Buff].kp = 240;
	YawInner[Buff].ki = 100;
	YawInner[Buff].kd = 0;
	YawInner[Buff].integralErrorMax = 10;
	YawInner[Buff].resultMax = Yaw-> getMotorCurrentLimit(); 
	YawInner[Fastbuff].kp = 250;
	YawInner[Fastbuff].ki = 100;
	YawInner[Fastbuff].kd = 0;
	YawInner[Fastbuff].integralErrorMax = 10;
	YawInner[Fastbuff].resultMax = Yaw-> getMotorCurrentLimit(); 
	
	YawInner[Vision].kp = 240; //{110, 300, 0, 1} //300
	YawInner[Vision].ki = 100;
	YawInner[Vision].kd = 0;
	YawInner[Vision].integralErrorMax = 10;
	YawInner[Vision].resultMax = Yaw-> getMotorCurrentLimit();
	YawInner[VisionSniper].kp = 240;
	YawInner[VisionSniper].ki = 100;
	YawInner[VisionSniper].kd = 0;
	YawInner[VisionSniper].integralErrorMax = 0;
	YawInner[VisionSniper].resultMax = Yaw-> getMotorCurrentLimit();
	
	YawOuter[Free].kp = 17; //{17,8,0.7,2,1000}
	YawOuter[Free].ki = 50;
	YawOuter[Free].kd = 0.8; //0.6
	YawOuter[Free].integralErrorMax = 0.2;
	YawOuter[Free].resultMax = 1000;
	YawOuter[Buff].kp = 20;
	YawOuter[Buff].ki = 250;
	YawOuter[Buff].kd = 0.05;
	YawOuter[Buff].integralErrorMax = 0.1;
	YawOuter[Buff].resultMax = 1000;
	YawOuter[Fastbuff].kp = 17;
	YawOuter[Fastbuff].ki = 100;
	YawOuter[Fastbuff].kd = 0.05;
	YawOuter[Fastbuff].integralErrorMax = 0.3;
	YawOuter[Fastbuff].resultMax = 1000;
	
	YawOuter[Vision].kp = 19; //(15 100 0.1 1.15 1000)
	YawOuter[Vision].ki = 150;//150
	YawOuter[Vision].kd = 0.01; 
	YawOuter[Vision].integralErrorMax = 0.1;
	YawOuter[Vision].resultMax = 1000;
	YawOuter[VisionSniper].kp = 18;//{25,100,0,1,1920,0,1,0,1,0}
	YawOuter[VisionSniper].ki = 150;
	YawOuter[VisionSniper].kd = 0.01;
	YawOuter[VisionSniper].integralErrorMax = 0.05;
	YawOuter[VisionSniper].resultMax = Yaw-> getMotorSpeedLimit();

	RollInner[Free].kp = 210; //{130,30,0,10,16280} {175, 200, 0.01, 0, 16280}
	RollInner[Free].ki = 200;
	RollInner[Free].kd = 0.01;
	RollInner[Free].integralErrorMax = 0;
	RollInner[Free].resultMax = Roll -> getMotorCurrentLimit();
	RollInner[Buff].kp = 210;
	RollInner[Buff].ki = 100;
	RollInner[Buff].kd = 0;
	RollInner[Buff].integralErrorMax = 0;
	RollInner[Buff].resultMax = Roll -> getMotorCurrentLimit();
	RollInner[Fastbuff].kp = 250;
	RollInner[Fastbuff].ki = 0;
	RollInner[Fastbuff].kd = 0;
	RollInner[Fastbuff].integralErrorMax = 0;
	RollInner[Fastbuff].resultMax = Roll -> getMotorCurrentLimit();
	
	RollInner[Vision].kp = 250;    //{200�� 80�� 0.01�� 5}
	RollInner[Vision].ki = 150; 
	RollInner[Vision].kd = 0.1;//0.1
	RollInner[Vision].integralErrorMax = 10;//30
	RollInner[Vision].resultMax = Roll -> getMotorCurrentLimit();
	RollInner[VisionSniper].kp = 250;
	RollInner[VisionSniper].ki = 150;
	RollInner[VisionSniper].kd = 0.1;
	RollInner[VisionSniper].integralErrorMax = 10;
	RollInner[VisionSniper].resultMax = Roll -> getMotorCurrentLimit();
	
	RollOuter[Free].kp = 14.5; //{10,6,0.05,1,1920}{13,100,0.1,0.15,700}
	RollOuter[Free].ki = 100;
	RollOuter[Free].kd = 0.15;
	RollOuter[Free].integralErrorMax = 0.1;
	RollOuter[Free].resultMax = Roll -> getMotorSpeedLimit();  
	RollOuter[Buff].kp = 20;
	RollOuter[Buff].ki = 250;
	RollOuter[Buff].kd = 0.01;
	RollOuter[Buff].integralErrorMax = 0.1;
	RollOuter[Buff].resultMax = Roll -> getMotorSpeedLimit();
	RollOuter[Fastbuff].kp = 18;
	RollOuter[Fastbuff].ki = 200;
	RollOuter[Fastbuff].kd = 0;
	RollOuter[Fastbuff].integralErrorMax = 0.1;
	RollOuter[Fastbuff].resultMax = Roll -> getMotorSpeedLimit();
	
	RollOuter[Vision].kp = 20; //{25�� 8�� 0.1�� 1�� 500�� 0�� 2.5�� 1}
	RollOuter[Vision].ki = 200;
	RollOuter[Vision].kd = 0.01;
	RollOuter[Vision].integralErrorMax = 0.15;
	RollOuter[Vision].resultMax = Roll -> getMotorSpeedLimit();
	RollOuter[VisionSniper].kp = 19;
	RollOuter[VisionSniper].ki = 200;
	RollOuter[VisionSniper].kd = 0.01;
	RollOuter[VisionSniper].integralErrorMax = 0.1;
	RollOuter[VisionSniper].resultMax = Roll -> getMotorSpeedLimit();
	//ң������ˢ��
	fbValUpdate(); 
	//��ʼ����ʼλ��
	Yaw->setZeroValue(960);   
	Roll->setZeroValue(5500); 
	//�������趨
	Yaw   ->pidInner.setPlanNum(PIDplanNum);
	Yaw   ->pidOuter.setPlanNum(PIDplanNum);
	Roll  ->pidInner.setPlanNum(PIDplanNum);
	Roll  ->pidOuter.setPlanNum(PIDplanNum);
	//pid������Դ
	Yaw->  pidInner.paramPtr = YawInner;
	Yaw->  pidOuter.paramPtr = YawOuter;
	Roll-> pidInner.paramPtr = RollInner;
	Roll-> pidOuter.paramPtr = RollOuter;
	//pid����ֵ��Դ
	Yaw   ->pidInner.fbValuePtr[Buff] = &WorldDps[YAW]; //������
	Yaw   ->pidOuter.fbValuePtr[Buff] = &WorldAlg[YAW];
  Roll  ->pidInner.fbValuePtr[Buff] = &WorldDps[ROLL];
	Roll  ->pidOuter.fbValuePtr[Buff] = &WorldAlg[ROLL];
	
	Yaw   ->pidInner.fbValuePtr[Fastbuff] = &WorldDps[YAW];
	Yaw   ->pidOuter.fbValuePtr[Fastbuff] = &WorldAlg[YAW];
  Roll  ->pidInner.fbValuePtr[Fastbuff] = &WorldDps[ROLL];
	Roll  ->pidOuter.fbValuePtr[Fastbuff] = &WorldAlg[ROLL];
	
	Yaw   ->pidInner.fbValuePtr[Free] = &WorldDps[YAW];
	Yaw   ->pidOuter.fbValuePtr[Free] = &WorldAlg[YAW];
  Roll  ->pidInner.fbValuePtr[Free] = &WorldDps[ROLL];
	Roll  ->pidOuter.fbValuePtr[Free] = &WorldAlg[ROLL];
	
	Yaw   ->pidInner.fbValuePtr[Vision] = &WorldDps[YAW];
	Yaw   ->pidOuter.fbValuePtr[Vision] = &WorldAlg[YAW];
  Roll  ->pidInner.fbValuePtr[Vision] = &WorldDps[ROLL];
	Roll  ->pidOuter.fbValuePtr[Vision] = &WorldAlg[ROLL];
	
	Yaw   ->pidInner.fbValuePtr[VisionSniper] = &WorldDps[YAW];
	Yaw   ->pidOuter.fbValuePtr[VisionSniper] = &WorldAlg[YAW];
  Roll  ->pidInner.fbValuePtr[VisionSniper] = &WorldDps[ROLL];
	Roll  ->pidOuter.fbValuePtr[VisionSniper] = &WorldAlg[ROLL];
	
	IFlash.link(ptz_error, sizeof(ptz_error), 8);
	IFlash.read();
}


PTZ::PTZ()                   //��̨���캯��(����̧�����)
{
	visionmodejudge = new doubleKickJudge;   //����˫��������
}

void PTZ::fbValUpdate()      //����ˢ�º���
{
	//���ٶȷ���
	SelfDps[ROLL]  = Roll ->canInfo.speed;
	SelfDps[YAW]   = Yaw  ->canInfo.speed;
  WorldDps[ROLL] = boardImu->gyro.dps.data[0];
	WorldDps[YAW]  = boardImu->gyro.dps.data[2];
  //�Ƕȷ���
  SelfAlg[ROLL]  = Roll ->canInfo.totalAngle_f ;
  SelfAlg[YAW]   = Yaw  ->canInfo.totalAngle_f;	
	WorldAlg[ROLL] =boardImu->imuView->Angle.roll;
	WorldAlg[YAW]  =boardImu->imuView->Angle.yaw;
}


#define FIND_ENEMY_OBJ (!vision_RecvStruct.no_Obj && !visionInfo.offlineFlag)//�궨�巢�ֵз���λ����
bool lastif_findenemy = 1;  //��ֵ��1���㿪ʼʱִ��
enum CtrlPlan_t  CtrlPlan, lastCtrlPlan;
int a = 0;
float J_yaw = 0, J_pitch = 0;
float R_yaw = 0, R_pitch = 0;
float Y_error = 0, R_error = 0, Strandard_s = 0.1, Strandard_x = -0.1;
//��̨�˶�
int current_ctrl = 0;
void PTZ::Modeswitch_run(void)
{
	if(RC.Key.CTRL && RC.Key.Z)
	{ptz_error[0] = 100;
	ptz_error[1] = 100;
	ptz_error[2] = 100;
	ptz_error[3] = 100;}
	recordYawAngle = WorldAlg[YAW]; //ʱ�̼�¼��̨λ��
	recordRollAngle = WorldAlg[ROLL];
	R_yaw = WorldAlg[YAW];
	R_pitch = WorldAlg[ROLL];
	Y_error = Yaw->pidOuter.error;
	R_error = Roll->pidOuter.error;
//	if(WorldAlg[ROLL] > 0)  //�����������
//	{
//		current_ctrl = 2500 - (ABS(WorldAlg[ROLL]) * 40);
//	}
//	if(WorldAlg[ROLL] < 0)
//	{
//		current_ctrl = 2500 + (ABS(WorldAlg[ROLL]) * 30);
//	}
	//���������һֱ��ȡ�����������λ�û��ȵ���
	if(deforceFlag  || ptz.Yaw->canInfo.lostFlag)
	{  
		ptz.angle_change_allow = 1;
	}
      //С����ƫ��(���Ӿ��������̨)
		  if(chassis.rotateFlag)  
		  {
			  if(power.speedUpLevel == 0)
			    a = 2;
			  else if(power.speedUpLevel == 1)
				  a = 3;
			  else if(power.speedUpLevel == 2)
			    a = 4;
		  }
		  else  
			  a = 0;
		
		//�����С���ݣ����߲����Ӿ������������Ӿ�ƫ��
		if(chassis.rotateFlag == 0 || lastCtrlPlan != Vision) 
		{
			vision_SendStruct.selfspining = 0;
		}
	if(CtrlPlan == Free)  //���ɿ���ģʽ
	{
		if(angle_change_allow == 1)  //���������˲��ı�һ��λ�ó�ֵ
	  {
		  motorPos2[YAW]  = recordYawAngle;
			motorPos2[ROLL] = 1; //��ʼֵ��ÿ��������
		  angle_change_allow = 0;
	  }
		//ң�ؿ���
	  PTZPos_set[0] = RC.Key.CH[0] * YawStick_Sensitivity;
	  PTZPos_set[1] = RC.Key.CH[1] * RollStick_Sensitivity;
	  //���̿���
	  PTZPos_set[0] += RC.Key.CH[6] * YawMouse_Sensitivity;   
	  PTZPos_set[1] += RC.Key.CH[7] * RollMouse_Sensitivity;
		
		motorPos2[YAW] += PTZPos_set[0] ;
	  motorPos2[ROLL]-= PTZPos_set[1] ; 

		change_direction();  //�����Ӿ�ģʽ���Ի���
		motorPos2[ROLL] = LIMIT(motorPos2[ROLL], -25 , 30);
		
		Yaw ->ctrlPosition(motorPos2[YAW] + a, Free) ;
		
	  Roll->ctrlPosition(motorPos2[ROLL] , Free);
//		Roll->pidOuter.Calculate(motorPos2[ROLL],Free);
//		Roll->pidInner.Calculate(Roll->pidOuter.result,Free);
//		Roll->ctrlCurrent(Roll->pidInner.result + current_ctrl);
//		Roll->ctrlCurrent(current_ctrl);
	}
	if(CtrlPlan == Vision)
	{
		
		//motorPos2[YAW] = visionYawAngle.KalmanFilter(vision_RecvStruct.Yaw, yawProcessNiose_Q, yawMeasureNoise_R);
		//motorPos2[ROLL] = visionPitchAngle.KalmanFilter(vision_RecvStruct.Pitch, pitchProcessNiose_Q, pitchMeasureNoise_R);
    //�Ӿ�����ģʽ���ҷ��ֵ���
		if((VM == bigBUFF || VM == smallBUFF ) && FIND_ENEMY_OBJ)
		{
			PTZPos_set[0] = vision_RecvStruct.Yaw * 1;   //�Ӿ���Ϣ����
	    PTZPos_set[1] = vision_RecvStruct.Pitch * 1;
			
		  motorPos2[YAW] = PTZPos_set[0]; 
			motorPos2[ROLL]= -PTZPos_set[1];  //�����������л�
      lastif_findenemy = 1;
			
      motorPos2[ROLL] = LIMIT(motorPos2[ROLL], -25, 30); //roll���ƶ���Χ 	
			
				Yaw ->ctrlPosition(motorPos2[YAW] - 10 + ((float)ptz_error[buff_yaw]/10), Buff);
	      Roll->ctrlPosition(motorPos2[ROLL] - 5 + ((float)ptz_error[buff_pitch]/20), Buff);  
//				Roll->pidOuter.Calculate(motorPos2[ROLL] - 5 + ((float)ptz_error[buff_pitch]/20),Buff);
//			  Roll->pidOuter.Calculate(motorPos2[ROLL],Buff);
//		    Roll->pidInner.Calculate(Roll->pidOuter.result,Buff);
//		    Roll->ctrlCurrent(Roll->pidInner.result + current_ctrl);
							
		} 
		//�Ӿ�ƽ��ģʽ/����ģʽ���ҷ��ֵ���
		if((VM == ARMOR || VM == ANTIBUFF) && FIND_ENEMY_OBJ)
		{
			if(chassis.rotateFlag && lastCtrlPlan != Vision) //�ȿ�С���ݣ��ٿ��Ӿ�ʱ����
			{
				vision_SendStruct.selfspining = 1;
			}
			PTZPos_set[0] = vision_RecvStruct.Yaw * 1;   //�Ӿ���Ϣ����
	    PTZPos_set[1] = vision_RecvStruct.Pitch * 1;

		  motorPos2[YAW] = PTZPos_set[0]; //��ֵ�����л���̨��
	    motorPos2[ROLL]= -PTZPos_set[1] ;					
			
      lastif_findenemy = 1;
			
      motorPos2[ROLL] = LIMIT(motorPos2[ROLL], -25, 30); //roll���ƶ���Χ
 			 
			Yaw ->ctrlPosition(motorPos2[YAW] - 10 + ((float)ptz_error[vision_yaw]/10), Vision);
	    Roll->ctrlPosition(motorPos2[ROLL] - 5 + ((float)ptz_error[vision_pitch]/20), Vision); 
//				Roll->pidOuter.Calculate(motorPos2[ROLL] - 5 + ((float)ptz_error[buff_pitch]/20),Vision);
//		    Roll->pidInner.Calculate(Roll->pidOuter.result,Vision);
//		    Roll->ctrlCurrent(Roll->pidInner.result + current_ctrl);       			
		}
		//�Ӿ�����ģʽ���ҷ��ֵ���
		if(VM == rotate && FIND_ENEMY_OBJ)
		{
			PTZPos_set[0] = vision_RecvStruct.Yaw * 1;   //�Ӿ���Ϣ����
	    PTZPos_set[1] = vision_RecvStruct.Pitch * 1;
			
		  motorPos2[YAW] = PTZPos_set[0]; //��ֵ�����л���̨��
	    motorPos2[ROLL]= -PTZPos_set[1] ;	 
			
      lastif_findenemy = 1;
			
      motorPos2[ROLL] = LIMIT(motorPos2[ROLL], -25, 30); //roll���ƶ���Χ
 			 
			Yaw ->ctrlPosition(motorPos2[YAW], VisionSniper);
	    Roll->ctrlPosition(motorPos2[ROLL], VisionSniper);  
//				Roll->pidOuter.Calculate(motorPos2[ROLL],VisionSniper);
//		    Roll->pidInner.Calculate(Roll->pidOuter.result,VisionSniper);
//		    Roll->ctrlCurrent(Roll->pidInner.result + current_ctrl);			
		}
		//������Ӿ�ģʽ����û�з��ֵ���
		if(!FIND_ENEMY_OBJ)
		{
			if(lastif_findenemy || lastCtrlPlan != Vision)  //�ϴ�ʶ��Ŀ��,�����ϴ�ģʽ�����Ӿ�ģʽ 
			{
			  motorPos2[YAW] = recordYawAngle; //ת��Ϊ����ֵ
	      motorPos2[ROLL]= recordRollAngle;  
			}
		 //ң�ؿ���
			PTZPos_set[0] = RC.Key.CH[0] * YawStick_Sensitivity;
	    PTZPos_set[1] = RC.Key.CH[1] * RollStick_Sensitivity; 
		 //���̿���
	    PTZPos_set[0] += RC.Key.CH[6] * YawMouse_Sensitivity;   
	    PTZPos_set[1] += RC.Key.CH[7] * RollMouse_Sensitivity;
			
			motorPos2[YAW] += PTZPos_set[0] ;
	    motorPos2[ROLL]-= PTZPos_set[1] ; 
			
			lastif_findenemy = 0;
			
			motorPos2[ROLL] = LIMIT(motorPos2[ROLL], -25, 30); //roll���ƶ���Χ
		  Yaw ->ctrlPosition(motorPos2[YAW], Free);
//	    Roll->ctrlPosition(motorPos2[ROLL], Free); 
		Roll->pidOuter.Calculate(motorPos2[ROLL],Free);
		Roll->pidInner.Calculate(Roll->pidOuter.result,Free);
		Roll->ctrlCurrent(Roll->pidInner.result + current_ctrl);
		}
	 
	}
	J_yaw = motorPos2[YAW];
	J_pitch = motorPos2[ROLL];
	lastCtrlPlan = CtrlPlan;  //��¼�ϴε�ģʽ
}
//������ (1s cd)
int change_direction_CD = 0;
float change_time = 0;
u8 allow_change = 0;
void PTZ::change_direction(void)
{
////	direction_change_flag = 0;
//	if(RC.Key.CTRL && RC.Key.Q && !change_direction_CD)
//	{
////		direction_change_flag = 1;
//		motorPos2[YAW] += 180;
//		change_direction_CD = 1;
//	}
//	if(change_direction_CD){
//	  change_direction_CD++;
//		if(change_direction_CD > 500){change_direction_CD = 0;}
//	}

 direction_change_flag = 0;
	if((RC.Key.CTRL && RC.Key.Q && !change_direction_CD) || allow_change)
	{
		direction_change_flag = 1;
		change_time += 1;
		allow_change = 1;
		motorPos2[YAW] += 0.8;
		change_direction_CD = 1;
	}
	if(change_time >= 225)
	{
		change_time = 0;
		allow_change = 0;
	}
	if(change_direction_CD){
	  change_direction_CD++;
		if(change_direction_CD > 400){change_direction_CD = 0;}
	}
}
//��ȡ�Ƿ�����Զ�����
void PTZ::getautofire(void)
{
	autofireFlag = 0;  //�Զ������־λˢ��
	YawErrorToVision = ABS(ptz.Yaw->pidOuter.error);
	PitchErrorToVision = ABS(ptz.Roll->pidOuter.error);
	if(CtrlPlan == Vision && FIND_ENEMY_OBJ) //����ģʽΪ�Ӿ�ģʽ���ҷ���Ŀ��ʱ�ж�
	{
		if(YawErrorToVision < 0.1 && PitchErrorToVision < 0.2)
		{
			autofireFlag = 1; 
		}
	}
}
//��λģʽ�ж�
u8 static_armor = 0, static_rotate = 0, static_buff = 0;
u8 mode_static_smallbuff = 0; //���ģʽ�̶�
u8 mode_static_bigbuff = 0;
u8 mode_static_antibuff = 0;
u8 mode_static_rotate = 0;
void PTZ::ModeJudge_PTZ()  
{
	CtrlPlan = Free;   //Ĭ��ģʽΪ�����ƶ�
	
	VMjudge = visionmodejudge->doubleKickVal(RC.Key.CH[9]);
	switch(VMjudge)
	{
		case 0:
			CtrlPlan = Free;
		  VM = ARMOR;
			break;
		case 1:
			CtrlPlan = Vision;
			VM = ARMOR;
			break;
		case 2:
			CtrlPlan = Vision;
			VM = rotate;
			break;
	}
	//����ƫ�ã���������shift����ctrl+s��
	if(deforceFlag && RC.Key.CTRL && RC.Key.S && !RC.Key.SHIFT)
	{
		IFlash.save();
	}
	if(deforceFlag && RC.Key.SW1 == RCS::Mid)
	{
		CtrlPlan = Free;
		VM = ARMOR;
		chassis.ifChassisNoFollow = 0;
		static_rotate = 0;
		static_armor = 0;
		static_buff = 0;
		
	}
	if((ABS(RC.Key.CH[11]) >= 400 && deforceFlag) || static_armor)
	{
		CtrlPlan = Vision;
		VM = ARMOR;
	  static_armor = 1;	
	}
	if((RC.Key.SW1 == RCS::Up && deforceFlag) || static_rotate)
	{
		CtrlPlan = Vision;
		VM = ANTIBUFF;
	  static_rotate = 1;	
	}
	if((RC.Key.SW1 == RCS::Down && deforceFlag) || static_buff)
	{
		CtrlPlan = Vision;
		VM = smallBUFF;
		chassis.ifChassisNoFollow = 1;
	  static_buff = 1;	
	}

	if((RC.Key.CTRL && RC.Key.G ) || mode_static_smallbuff == 1) //ctrl+G--����ģʽ
	{
		CtrlPlan = Vision; 
//		if((can1Feedback.game_progress == 4) && (can1Feedback.stageRemainTime > 180))
		VM = smallBUFF;
    chassis.ifChassisNoFollow = 1;
    mode_static_smallbuff = 1;
	}
	if((RC.Key.CTRL && RC.Key.G && RC.Key.SHIFT) || mode_static_bigbuff)
	{
		CtrlPlan = Vision; 
		VM = bigBUFF;
    chassis.ifChassisNoFollow = 1;
    mode_static_bigbuff = 1;
	}

	if(RC.Key.Q && !RC.Key.CTRL)  //����Q--����ģʽ
	{
		CtrlPlan = Vision; VM = ANTIBUFF;
	}
	
	
	if(RC.Key.Z) //��Z�˳��Ӿ�ģʽ�����Ӿ�ģʽ�ָ�Ĭ�ϣ�װ�ף�ģʽ
	{
		CtrlPlan = Free; VM = ARMOR;
		chassis.ifChassisNoFollow = 0;
		mode_static_smallbuff = 0; 
		mode_static_bigbuff = 0;
 //   mode_static_antibuff = 0;
		static_rotate = 0;
		static_armor = 0;
		static_buff = 0;
	}
	
}



