#include "motor_MT_DLC.h"

MotorList MT_motorlist[2][32];
MT_Can MT_can1;
MT_Can MT_can2;
u8 MT_IMcnt;

//脉塔电机初始化函数
void MT_Motor_Init(Motor* MT_Motor,CAN_TypeDef *_Canx,uint32_t _Std_ID)
{
	MT_Motor->setOutFunction(MT_ctrlCurrent);
	if(_Canx == CAN1)
	{
		MT_motorlist[Can_1][_Std_ID-0x141].motorPoint = MT_Motor;
		MT_motorlist[Can_1][_Std_ID-0x141].enableFlag = 1;
		MT_motorlist[Can_1][_Std_ID-0x141].sendEnableFlag = 1;
	}
	else if(_Canx == CAN2)
	{
		MT_motorlist[Can_2][_Std_ID-0x141].motorPoint = MT_Motor;
		MT_motorlist[Can_2][_Std_ID-0x141].enableFlag = 1;
		MT_motorlist[Can_2][_Std_ID-0x141].sendEnableFlag = 1;
	}
}

//用于电机反馈值的获取	放进schedule里	500Hz
void MT_GetInfomation(Motor * MT_Motor)
{
	if(MT_IMcnt == 0)
	{
		u8 data[8];
		data[0] = 0x9c;
		canTx(data,CAN1,MT_Motor->getStd_Id());
		canTx(data,CAN2,MT_Motor->getStd_Id());
		MT_IMcnt = 1;
	}
	else
	{
		u8 data[8];
		data[0] = 0x90;
		canTx(data,CAN1,MT_Motor->getStd_Id());
		canTx(data,CAN2,MT_Motor->getStd_Id());
		MT_IMcnt = 0;
	}
}

//电机控制(电流)
void MT_ctrlCurrent(float current,Motor * MT_Motor)
{
	if(current>2000)
		current = 2000;
	if(current<-2000)
		current = -2000;
	
	int16_t current1 = current;
	
	u8 data[8] = {0};
	data[0] = 0xA1;
	data[4] = *(u8 *)(&current1);
	data[5] = *((u8 *)(&current1)+1);
	
	if(MT_IMcnt == 1)
		canTx(data,CAN2,0x141);
}

//setZeroValue		禁用

/**
  * @brief MT电机上电初始化
  * @param[in] can_x CAN1/CAN2
  * @param[in] _CanRxMsg Can原始数据地址
  */
void MT_Can::Motor_Offset(u8 can_x, CanRxMsg *_CanRxMsg)
{
	//获取数据输出地址
	Info = &(MT_motorlist[can_x][_CanRxMsg->StdId - 0x241].motorPoint->canInfo);
	
	//初始角度
	Info->encoder = (int16_t)((_CanRxMsg->Data[7] << 8) | (_CanRxMsg->Data[6])); //机械角度
	Info->lastEncoder = Info->encoder;											 //解决上电圈数不对问题
	Info->totalRound = 0;														 //电机角度归零时用
	Info->totalAngle = 0;
	Info->totalEncoder_SI = 0;
	Info->totalEncoder = 0;
}

//can返回信息计算	放进can接收中断中
void MT_Can::Motor_Information_Calculate(u8 can_x, CanRxMsg *_CanRxMsg)
{
	//是大疆标准电机，直接退出
	if (IsDJIMotorCheck(*_CanRxMsg) == 1)
	{
		return;
	}
	
	//如果该can口的电机没有使能，则报error并退出
	if (MT_motorlist[can_x][_CanRxMsg->StdId - 0x241].enableFlag == 0)
	{
		return;
	}
	if (&(MT_motorlist[can_x][_CanRxMsg->StdId - 0x241].motorPoint->canInfo) != 0)
	{
		//获取数据输出地址
		Info = &(MT_motorlist[can_x][_CanRxMsg->StdId - 0x241].motorPoint->canInfo);
	}

	//初始化校准
	if (++Info->msgCnt < 3)
	{
		Motor_Offset(can_x, _CanRxMsg);
		return;
	}

	Info->msgCnt = 100;

	/*获取电机转速及机械角度*/
	if(_CanRxMsg->Data[0] == 0x90)
	{
		Info->encoder = (uint16_t)((_CanRxMsg->Data[5] << 8) | (_CanRxMsg->Data[4]));	//位置
		Info->encoderCalibration = Info->encoder;
		/*计算电机旋转总圈数*/
		if (Info->encoderCalibration - Info->lastEncoderCalibration > 8192)
		{
			Info->totalRound--;
		}
		else if (Info->encoderCalibration - Info->lastEncoderCalibration < -8192)
		{
			Info->totalRound++;
		}
		/*计算电机旋转总机械角度值*/
		Info->totalEncoder = (Info->totalRound * 16384 + Info->encoderCalibration);
		/*计算电机旋转总角度*/
		Info->totalAngle = Info->totalEncoder * 0.02197265625f;	 //(强制转换成整型)
		Info->totalAngle_f = Info->totalEncoder * 0.02197265625f; //(浮点型)
		/*记录此次机械角度*/
		Info->lastEncoder = Info->encoder;
		Info->lastTotalEncoder = Info->totalEncoder;
		Info->lastEncoderCalibration = Info->encoderCalibration;
	}
	else
	{
		Info->speed = (int16_t)((_CanRxMsg->Data[5] << 8) | (_CanRxMsg->Data[4]));		 //速度
		Info->trueCurrent = (int16_t)((_CanRxMsg->Data[3] << 8) | (_CanRxMsg->Data[2])); //实际电流
		Info->temperature = (int16_t)(_CanRxMsg->Data[1]);								 //温度
	}
	

	/*计算云台电机旋转速度=机械角度值之差与RPM换算*/
	Info->dps = Info->speed * 6.0f; //(浮点型)

	Info->totalEncoder_SI += Info->speed;
	//速度积分和位置反馈做互补滤波！！！

	/*电机离线检测部分*/
	Info->lostCnt = 0;	//清空计数器
	Info->lostFlag = 0; //电机在线
}


/**
  * @brief 大疆电机检查
  * @param[in] _CanRxMsg 通过判断ID来确定是不是大疆电机
  * @return 1:是，0：不是
  */
uint8_t MT_Can::IsDJIMotorCheck(CanRxMsg _CanRxMsg)
{
	if (_CanRxMsg.StdId > 0x200 && _CanRxMsg.StdId < 0x209)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

