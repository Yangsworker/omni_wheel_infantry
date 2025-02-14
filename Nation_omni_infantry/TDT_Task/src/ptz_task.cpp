#include "ptz_task.h"
#include "dbus.h"
#include "my_math.h"
#include "vision.h"
#include "state_task.h"
#include "flash_var.h"

/* 
* 本程序因之前用V4主控，导致pitch轴多处用roll代替，还未作出修改，注意    
*/
float send_roll_offset = 0;  //视觉发送值处理
 

PidParam YawInner[PIDplanNum], YawOuter[PIDplanNum],
         RollInner[PIDplanNum], RollOuter[PIDplanNum],
				 LeftInner, LeftOuter;
PTZ ptz;
VisionMode_t VM; //视觉模式枚举

u8 ptz_error[4] = {100, 100, 100, 100};

//******************不会用*************************//
u8 kfSwitch = 1;
Kf visionYawAngle, visionPitchAngle;//卡尔曼类的视觉角度对象
Lpf2p fbFilter_Yaw, fbFilter_Pitch;//低通滤波类的视觉角度对象
//操作手状态
#define OPERATOR_NORMAL 0	 //正常模式
#define OPERATOR_FINE_TUNE 1 //微调模式
#define OPERATOR_SILENCE 2	 //静默模式
//*************************************************//

float YawStick_Sensitivity = -0.001f; //y摇杆灵敏度
float RollStick_Sensitivity =  -0.001f; //r摇杆灵敏度
float YawMouse_Sensitivity = -0.003f;//y鼠标灵敏度
float RollMouse_Sensitivity = -0.003f;//r鼠标灵敏度

void Ptz_task(void)
{  
  ptz.fbValUpdate();   //数据刷新
  ptz.ModeJudge_PTZ(); //云台模式选择
	ptz.Modeswitch_run();//云台运动函数
	ptz.getautofire();   //获取是否可以自动开火
}

void PTZ::Ptz_Task_Init(void)//构造yaw, roll轴电机 0x206  0x206
 {
	//构造电机
	Yaw     = new Motor (GM6020, CAN1, 0x205);
	Roll    = new Motor (GM6020, CAN2, 0x207);
	//参数赋值
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
	
	RollInner[Vision].kp = 250;    //{200， 80， 0.01， 5}
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
	
	RollOuter[Vision].kp = 20; //{25， 8， 0.1， 1， 500， 0， 2.5， 1}
	RollOuter[Vision].ki = 200;
	RollOuter[Vision].kd = 0.01;
	RollOuter[Vision].integralErrorMax = 0.15;
	RollOuter[Vision].resultMax = Roll -> getMotorSpeedLimit();
	RollOuter[VisionSniper].kp = 19;
	RollOuter[VisionSniper].ki = 200;
	RollOuter[VisionSniper].kd = 0.01;
	RollOuter[VisionSniper].integralErrorMax = 0.1;
	RollOuter[VisionSniper].resultMax = Roll -> getMotorSpeedLimit();
	//遥控数据刷新
	fbValUpdate(); 
	//初始化初始位置
	Yaw->setZeroValue(960);   
	Roll->setZeroValue(5500); 
	//方案数设定
	Yaw   ->pidInner.setPlanNum(PIDplanNum);
	Yaw   ->pidOuter.setPlanNum(PIDplanNum);
	Roll  ->pidInner.setPlanNum(PIDplanNum);
	Roll  ->pidOuter.setPlanNum(PIDplanNum);
	//pid参数来源
	Yaw->  pidInner.paramPtr = YawInner;
	Yaw->  pidOuter.paramPtr = YawOuter;
	Roll-> pidInner.paramPtr = RollInner;
	Roll-> pidOuter.paramPtr = RollOuter;
	//pid反馈值来源
	Yaw   ->pidInner.fbValuePtr[Buff] = &WorldDps[YAW]; //陀螺仪
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


PTZ::PTZ()                   //云台构造函数(构造抬升电机)
{
	visionmodejudge = new doubleKickJudge;   //构造双击检测对象
}

void PTZ::fbValUpdate()      //数据刷新函数
{
	//角速度反馈
	SelfDps[ROLL]  = Roll ->canInfo.speed;
	SelfDps[YAW]   = Yaw  ->canInfo.speed;
  WorldDps[ROLL] = boardImu->gyro.dps.data[0];
	WorldDps[YAW]  = boardImu->gyro.dps.data[2];
  //角度反馈
  SelfAlg[ROLL]  = Roll ->canInfo.totalAngle_f ;
  SelfAlg[YAW]   = Yaw  ->canInfo.totalAngle_f;	
	WorldAlg[ROLL] =boardImu->imuView->Angle.roll;
	WorldAlg[YAW]  =boardImu->imuView->Angle.yaw;
}


#define FIND_ENEMY_OBJ (!vision_RecvStruct.no_Obj && !visionInfo.offlineFlag)//宏定义发现敌方单位条件
bool lastif_findenemy = 1;  //初值赋1方便开始时执行
enum CtrlPlan_t  CtrlPlan, lastCtrlPlan;
int a = 0;
float J_yaw = 0, J_pitch = 0;
float R_yaw = 0, R_pitch = 0;
float Y_error = 0, R_error = 0, Strandard_s = 0.1, Strandard_x = -0.1;
//云台运动
int current_ctrl = 0;
void PTZ::Modeswitch_run(void)
{
	if(RC.Key.CTRL && RC.Key.Z)
	{ptz_error[0] = 100;
	ptz_error[1] = 100;
	ptz_error[2] = 100;
	ptz_error[3] = 100;}
	recordYawAngle = WorldAlg[YAW]; //时刻纪录云台位置
	recordRollAngle = WorldAlg[ROLL];
	R_yaw = WorldAlg[YAW];
	R_pitch = WorldAlg[ROLL];
	Y_error = Yaw->pidOuter.error;
	R_error = Roll->pidOuter.error;
//	if(WorldAlg[ROLL] > 0)  //软件重力补偿
//	{
//		current_ctrl = 2500 - (ABS(WorldAlg[ROLL]) * 40);
//	}
//	if(WorldAlg[ROLL] < 0)
//	{
//		current_ctrl = 2500 + (ABS(WorldAlg[ROLL]) * 30);
//	}
	//如果脱力，一直读取，如果上力，位置环先调整
	if(deforceFlag  || ptz.Yaw->canInfo.lostFlag)
	{  
		ptz.angle_change_allow = 1;
	}
      //小陀螺偏置(不视觉情况稳云台)
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
		
		//如果不小陀螺，或者不开视觉，不启用新视觉偏置
		if(chassis.rotateFlag == 0 || lastCtrlPlan != Vision) 
		{
			vision_SendStruct.selfspining = 0;
		}
	if(CtrlPlan == Free)  //自由控制模式
	{
		if(angle_change_allow == 1)  //如果上力，瞬间改变一次位置初值
	  {
		  motorPos2[YAW]  = recordYawAngle;
			motorPos2[ROLL] = 1; //初始值（每次上力后）
		  angle_change_allow = 0;
	  }
		//遥控控制
	  PTZPos_set[0] = RC.Key.CH[0] * YawStick_Sensitivity;
	  PTZPos_set[1] = RC.Key.CH[1] * RollStick_Sensitivity;
	  //键盘控制
	  PTZPos_set[0] += RC.Key.CH[6] * YawMouse_Sensitivity;   
	  PTZPos_set[1] += RC.Key.CH[7] * RollMouse_Sensitivity;
		
		motorPos2[YAW] += PTZPos_set[0] ;
	  motorPos2[ROLL]-= PTZPos_set[1] ; 

		change_direction();  //仅非视觉模式可以换向
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
    //视觉开符模式并且发现敌人
		if((VM == bigBUFF || VM == smallBUFF ) && FIND_ENEMY_OBJ)
		{
			PTZPos_set[0] = vision_RecvStruct.Yaw * 1;   //视觉信息传递
	    PTZPos_set[1] = vision_RecvStruct.Pitch * 1;
			
		  motorPos2[YAW] = PTZPos_set[0]; 
			motorPos2[ROLL]= -PTZPos_set[1];  //陀螺仪正负切换
      lastif_findenemy = 1;
			
      motorPos2[ROLL] = LIMIT(motorPos2[ROLL], -25, 30); //roll轴移动范围 	
			
				Yaw ->ctrlPosition(motorPos2[YAW] - 10 + ((float)ptz_error[buff_yaw]/10), Buff);
	      Roll->ctrlPosition(motorPos2[ROLL] - 5 + ((float)ptz_error[buff_pitch]/20), Buff);  
//				Roll->pidOuter.Calculate(motorPos2[ROLL] - 5 + ((float)ptz_error[buff_pitch]/20),Buff);
//			  Roll->pidOuter.Calculate(motorPos2[ROLL],Buff);
//		    Roll->pidInner.Calculate(Roll->pidOuter.result,Buff);
//		    Roll->ctrlCurrent(Roll->pidInner.result + current_ctrl);
							
		} 
		//视觉平移模式/反符模式并且发现敌人
		if((VM == ARMOR || VM == ANTIBUFF) && FIND_ENEMY_OBJ)
		{
			if(chassis.rotateFlag && lastCtrlPlan != Vision) //先开小陀螺，再开视觉时发送
			{
				vision_SendStruct.selfspining = 1;
			}
			PTZPos_set[0] = vision_RecvStruct.Yaw * 1;   //视觉信息传递
	    PTZPos_set[1] = vision_RecvStruct.Pitch * 1;

		  motorPos2[YAW] = PTZPos_set[0]; //赋值，防切换云台抖
	    motorPos2[ROLL]= -PTZPos_set[1] ;					
			
      lastif_findenemy = 1;
			
      motorPos2[ROLL] = LIMIT(motorPos2[ROLL], -25, 30); //roll轴移动范围
 			 
			Yaw ->ctrlPosition(motorPos2[YAW] - 10 + ((float)ptz_error[vision_yaw]/10), Vision);
	    Roll->ctrlPosition(motorPos2[ROLL] - 5 + ((float)ptz_error[vision_pitch]/20), Vision); 
//				Roll->pidOuter.Calculate(motorPos2[ROLL] - 5 + ((float)ptz_error[buff_pitch]/20),Vision);
//		    Roll->pidInner.Calculate(Roll->pidOuter.result,Vision);
//		    Roll->ctrlCurrent(Roll->pidInner.result + current_ctrl);       			
		}
		//视觉陀螺模式并且发现敌人
		if(VM == rotate && FIND_ENEMY_OBJ)
		{
			PTZPos_set[0] = vision_RecvStruct.Yaw * 1;   //视觉信息传递
	    PTZPos_set[1] = vision_RecvStruct.Pitch * 1;
			
		  motorPos2[YAW] = PTZPos_set[0]; //赋值，防切换云台抖
	    motorPos2[ROLL]= -PTZPos_set[1] ;	 
			
      lastif_findenemy = 1;
			
      motorPos2[ROLL] = LIMIT(motorPos2[ROLL], -25, 30); //roll轴移动范围
 			 
			Yaw ->ctrlPosition(motorPos2[YAW], VisionSniper);
	    Roll->ctrlPosition(motorPos2[ROLL], VisionSniper);  
//				Roll->pidOuter.Calculate(motorPos2[ROLL],VisionSniper);
//		    Roll->pidInner.Calculate(Roll->pidOuter.result,VisionSniper);
//		    Roll->ctrlCurrent(Roll->pidInner.result + current_ctrl);			
		}
		//如果开视觉模式但是没有发现敌人
		if(!FIND_ENEMY_OBJ)
		{
			if(lastif_findenemy || lastCtrlPlan != Vision)  //上次识别到目标,或者上次模式不是视觉模式 
			{
			  motorPos2[YAW] = recordYawAngle; //转换为现在值
	      motorPos2[ROLL]= recordRollAngle;  
			}
		 //遥控控制
			PTZPos_set[0] = RC.Key.CH[0] * YawStick_Sensitivity;
	    PTZPos_set[1] = RC.Key.CH[1] * RollStick_Sensitivity; 
		 //键盘控制
	    PTZPos_set[0] += RC.Key.CH[6] * YawMouse_Sensitivity;   
	    PTZPos_set[1] += RC.Key.CH[7] * RollMouse_Sensitivity;
			
			motorPos2[YAW] += PTZPos_set[0] ;
	    motorPos2[ROLL]-= PTZPos_set[1] ; 
			
			lastif_findenemy = 0;
			
			motorPos2[ROLL] = LIMIT(motorPos2[ROLL], -25, 30); //roll轴移动范围
		  Yaw ->ctrlPosition(motorPos2[YAW], Free);
//	    Roll->ctrlPosition(motorPos2[ROLL], Free); 
		Roll->pidOuter.Calculate(motorPos2[ROLL],Free);
		Roll->pidInner.Calculate(Roll->pidOuter.result,Free);
		Roll->ctrlCurrent(Roll->pidInner.result + current_ctrl);
		}
	 
	}
	J_yaw = motorPos2[YAW];
	J_pitch = motorPos2[ROLL];
	lastCtrlPlan = CtrlPlan;  //记录上次的模式
}
//换向函数 (1s cd)
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
//获取是否可以自动开火
void PTZ::getautofire(void)
{
	autofireFlag = 0;  //自动开火标志位刷新
	YawErrorToVision = ABS(ptz.Yaw->pidOuter.error);
	PitchErrorToVision = ABS(ptz.Roll->pidOuter.error);
	if(CtrlPlan == Vision && FIND_ENEMY_OBJ) //仅在模式为视觉模式并且发现目标时判断
	{
		if(YawErrorToVision < 0.1 && PitchErrorToVision < 0.2)
		{
			autofireFlag = 1; 
		}
	}
}
//键位模式判断
u8 static_armor = 0, static_rotate = 0, static_buff = 0;
u8 mode_static_smallbuff = 0; //打符模式固定
u8 mode_static_bigbuff = 0;
u8 mode_static_antibuff = 0;
u8 mode_static_rotate = 0;
void PTZ::ModeJudge_PTZ()  
{
	CtrlPlan = Free;   //默认模式为自由移动
	
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
	//保存偏置（脱力不按shift并且ctrl+s）
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

	if((RC.Key.CTRL && RC.Key.G ) || mode_static_smallbuff == 1) //ctrl+G--开符模式
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

	if(RC.Key.Q && !RC.Key.CTRL)  //长按Q--反符模式
	{
		CtrlPlan = Vision; VM = ANTIBUFF;
	}
	
	
	if(RC.Key.Z) //按Z退出视觉模式，且视觉模式恢复默认（装甲）模式
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



