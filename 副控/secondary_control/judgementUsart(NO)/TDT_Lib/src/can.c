#include "can.h" 
#include "string.h" 
#include "gy53.h" 
#include "vision.h" 
/*----CAN1_TX-----PA12----*/ 
/*----CAN1_RX-----PA11----*/ 
u8 EnemyColor = 0; 
u8 useCap = 0; //是否开启电容，默认不开启 
u8 ULTS_mode = 0; 
u8 Check_mode = 0; 
struct Enemyposition enemyposition; 
struct PowerSendMsg powerSendMsg; 
struct CanStateStruct canStateStruct; 
struct CanStateStruct1 canStateStruct1; 
struct OrgPowerRecvMsg orgPowerRecvMsg;
struct CommandToGuardStruct commandtoguardstruct;
struct PowerRecvMsg powerRecvMsg; 
struct Enemyradius enemyradius; 
void Can0_Init(void) 
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	CAN_InitTypeDef CAN_InitStructure;
	CAN_FilterInitTypeDef CAN_FilterInitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1,	ENABLE);

	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化IO
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //上拉输入
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化IO

	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	// 主优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	// 次优先级为0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	CAN_InitStructure.CAN_TTCM = DISABLE; //非时间触发通信模式
	CAN_InitStructure.CAN_ABOM = DISABLE; //软件自动离线管理
	CAN_InitStructure.CAN_AWUM = DISABLE; //睡眠模式通过软件唤醒(清除CAN->MCR的SLEEP位)
	CAN_InitStructure.CAN_NART = ENABLE; //禁止报文自动传送
	CAN_InitStructure.CAN_RFLM = DISABLE; //报文不锁定,新的覆盖旧的
	CAN_InitStructure.CAN_TXFP = DISABLE; //优先级由报文标识符决定
	CAN_InitStructure.CAN_Mode = 0; //模式设置： mode:0,普通模式;1,回环模式;
	//can波特率：1M
	CAN_InitStructure.CAN_SJW = CAN_SJW_2tq; //重新同步跳跃宽度(Tsjw)为tsjw+1个时间单位
	CAN_InitStructure.CAN_BS1 = CAN_BS1_5tq; //Tbs1=tbs1+1个时间单位CAN_BS1_1tq ~CAN_BS1_16tq
	CAN_InitStructure.CAN_BS2 = CAN_BS2_3tq; //Tbs2=tbs2+1个时间单位CAN_BS2_1tq ~CAN_BS2_8tq
	CAN_InitStructure.CAN_Prescaler = 4; //分频系数(Fdiv)为brp+1
	CAN_Init(CAN1, &CAN_InitStructure); //初始化CAN1

	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask; //屏蔽位模式
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; //32位宽
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0; //过滤器0关联到FIFO0
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE; //激活过滤器0
	CAN_FilterInitStructure.CAN_FilterNumber = 0; //过滤器0
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000; //32位ID
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000; //32位MASK
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;

	CAN_FilterInit(&CAN_FilterInitStructure); //滤波器初始化
	CAN_ITConfig(CAN1, CAN_IT_FMP0 | CAN_IT_TME, ENABLE); //FIFO0消息挂号中断允许.

} 
 u8 t;
can1_feedback can1Feedback; 
CanRxMsg Can1RxMsg; 
extern u8 gimbalOffline; 
extern int gimbalOffineCheck; 
extern u8 superPowerOffline, superPowerOfflineCheck; 
void USB_LP_CAN1_RX0_IRQHandler(void) 
{ 
	if (CAN_GetITStatus(CAN1, CAN_IT_FMP0) != RESET) 
	{ 
		CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0); 
		CAN_Receive(CAN1, CAN_FIFO0, &Can1RxMsg); 
	switch (Can1RxMsg.StdId) 
	{ 
		case 0x141:   //收取can状态机消息画ui
	{ 
		gimbalOffline = 0; 
		gimbalOffineCheck = 0; 
		 
		 
		memcpy(&canStateStruct,Can1RxMsg.Data,8); 
		 
		useCap = canStateStruct.useCap; 
		ULTS_mode = canStateStruct.ULTSMode; 
		Check_mode = canStateStruct.checkMode; 
	} 
	break; 
		case 0x251:
		{
			memcpy(&commandtoguardstruct,Can1RxMsg.Data,8); 
			break;
		}	
	case 0x170: 
	{ 
	memcpy(&enemyposition,Can1RxMsg.Data,8); 
	} 
	break; 
	case 0x175: 
	{ 
		memcpy(&enemyradius,Can1RxMsg.Data,8); 
	} 
	break;	 
	case 0x201: 
		can1Feedback.chassisOffline[0] = 0; 
		can1Feedback.chassisOfflineCheck[0] = 0; 
		can1Feedback.chassisTemperature[0] = Can1RxMsg.Data[6]; 
	break; 
	case 0x202: 
		can1Feedback.chassisOffline[1] = 0; 
		can1Feedback.chassisOfflineCheck[1] = 0; 
		can1Feedback.chassisTemperature[1] = Can1RxMsg.Data[6]; 
	break; 
	case 0x203: 
		can1Feedback.chassisOffline[2] = 0; 
		can1Feedback.chassisOfflineCheck[2] = 0; 
		can1Feedback.chassisTemperature[2] = Can1RxMsg.Data[6]; 
	break; 
	case 0x204: 
		can1Feedback.chassisOffline[3] = 0; 
		can1Feedback.chassisOfflineCheck[3] = 0; 
		can1Feedback.chassisTemperature[3] = Can1RxMsg.Data[6]; 
	break; 
	case 0x206: //yaw轴电机
		can1Feedback.gimbalPos[0] = (int16_t)((Can1RxMsg.Data[0]<<8)|(Can1RxMsg.Data[1])); 
		can1Feedback.gimbalTemperature[0] = Can1RxMsg.Data[6]; 
	break; 
//	case 0x205: //原抬升云台
//		can1Feedback.gimbalPos[1] = (int16_t)((Can1RxMsg.Data[0]<<8)|(Can1RxMsg.Data[1])); 
//		can1Feedback.gimbalTemperature[1] = Can1RxMsg.Data[6]; 
//	break; 
				case 0x130: 
			{ 
				superPowerOffline = 0; 
				superPowerOfflineCheck = 0; 
				orgPowerRecvMsg = *((struct OrgPowerRecvMsg*)(Can1RxMsg.Data)); 
				powerRecvMsg.capacitance_percentage = orgPowerRecvMsg.capacitance_percentage/300.0; 
				powerRecvMsg.currentMax= orgPowerRecvMsg.currentMax/2000.0; 
				powerRecvMsg.currentNow= orgPowerRecvMsg.currentNow/2000.0; 
				powerRecvMsg.capHealth = orgPowerRecvMsg.capHealth; 
				powerRecvMsg.useCapNow = orgPowerRecvMsg.useCapNow; 
				 
				 
				if(powerRecvMsg.useCapNow && !powerRecvMsg.useCapLast) 
					powerRecvMsg.useCapNowBuffer += 1; 
				powerRecvMsg.useCapLast = powerRecvMsg.useCapNow; 
				++ powerRecvMsg.useCapIndex; 
				if(powerRecvMsg.useCapIndex >= 100) 
				{ 
					powerRecvMsg.useCapNowVar = powerRecvMsg.useCapNowBuffer; 
					powerRecvMsg.useCapIndex = 0; 
					powerRecvMsg.useCapNowBuffer = 0; 
				} 
				break; 
			} 
			case 0x151:
				memcpy(&canStateStruct1,Can1RxMsg.Data,sizeof(canStateStruct1)); 
			break;
	default: 
		break; 
	} 
		} 
 
} 
 
void USB_HP_CAN1_TX_IRQHandler(void) 
{ 
			CAN_ClearITPendingBit(CAN1, CAN_IT_TME); 

	if (CAN_GetITStatus(CAN1, CAN_IT_TME) != RESET) 
	{ 
		CAN_ClearITPendingBit(CAN1, CAN_IT_TME); 
	} 
} 
 
//1ms 
void Send_Judgement_Robot_Data() 
{ 
	CanTxMsg Can1TxMsg = {0}; 
 
	Can1TxMsg.IDE = 0;		 //标准帧 
	Can1TxMsg.RTR = 0;		 //数据帧 
	Can1TxMsg.DLC = 8;		 //帧长度 
	Can1TxMsg.StdId = 0x111; //ID:0x402 
 
	Can1TxMsg.Data[0] = (u8)((int16_t)judge_shoot_num >> 8); //子弹数目 
	Can1TxMsg.Data[1] = (u8)((int16_t)judge_shoot_num); 
	Can1TxMsg.Data[2] = (u8)((int16_t)(jgmt_mesg.shootData.bulletSpeed * 100) >> 8); //出膛速度 
	Can1TxMsg.Data[3] = (u8)((int16_t)(jgmt_mesg.shootData.bulletSpeed * 100)); 
	Can1TxMsg.Data[4] = (u8)((int16_t)jgmt_mesg.powerHeatData.shooterId1_17mmCoolingHeat >> 8); //实时热量数据 
	Can1TxMsg.Data[5] = (u8)((int16_t)jgmt_mesg.powerHeatData.shooterId1_17mmCoolingHeat); 
	Can1TxMsg.Data[6] = (u8)((int16_t)jgmt_mesg.powerHeatData.chassisPowerBuffer >> 8); //缓冲能量 
	Can1TxMsg.Data[7] = (u8)((int16_t)jgmt_mesg.powerHeatData.chassisPowerBuffer); 

	if (JgmtOffline == 0) 
		CAN_Transmit(CAN1, &Can1TxMsg); 
} 
 
//void Send_Judgement_Robot_Data2()  //双枪另一枪id 
//{ 
//	CanTxMsg Can1TxMsg; 
// 
//	Can1TxMsg.IDE = 0;		 //标准帧 
//	Can1TxMsg.RTR = 0;		 //数据帧 
//	Can1TxMsg.DLC = 8;		 //帧长度 
//	Can1TxMsg.StdId = 0x115; //ID:0x402 
// 
//	Can1TxMsg.Data[0] = (u8)((int16_t)jgmt_mesg.gameRobotStatus.shooterId2_17mmCoolingLimit>> 8); //热量上限 
//	Can1TxMsg.Data[1] = (u8)((int16_t)jgmt_mesg.gameRobotStatus.shooterId2_17mmCoolingLimit); 
//	Can1TxMsg.Data[2] = (u8)((int16_t)(jgmt_mesg.gameRobotStatus.shooterId1_17mmSpeedLimit) >> 8); //射速上限 
//	Can1TxMsg.Data[3] = (u8)((int16_t)(jgmt_mesg.gameRobotStatus.shooterId1_17mmSpeedLimit)); 
//	Can1TxMsg.Data[4] = (u8)((int16_t)jgmt_mesg.powerHeatData.shooterId2_17mmCoolingHeat >> 8); //实时热量数据 
//	Can1TxMsg.Data[5] = (u8)((int16_t)jgmt_mesg.powerHeatData.shooterId2_17mmCoolingHeat); 
//	Can1TxMsg.Data[6] = (u8)((int16_t)jgmt_mesg.gameRobotStatus.shooterId2_17mmCoolingRate >> 8); //2冷却速率 
//	Can1TxMsg.Data[7] = (u8)((int16_t)jgmt_mesg.gameRobotStatus.shooterId2_17mmCoolingRate); 
// 
//	if (JgmtOffline == 0) 
//		CAN_Transmit(CAN1, &Can1TxMsg); 
//} 
 
 
 
//2ms 
u8 oldstate; 
void Send_Judgement_Game_Data() 
{ 
	CanTxMsg Can1TxMsg; 
 
	Can1TxMsg.IDE = 0;		 //标准帧 
	Can1TxMsg.RTR = 0;		 //数据帧 
	Can1TxMsg.DLC = 8;		 //帧长度 
	Can1TxMsg.StdId = 0x112; //ID:0x401 
 
	if (jgmt_mesg.gameStatus.gameProgress == 4 && oldstate == 3) 
	{ 
		MidKp = 1.0f; 
		LowKp = 1.0f; 
	} 
	Can1TxMsg.Data[0] = (u8)(jgmt_mesg.gameStatus.gameProgress);				//比赛状态：	0： 未开始比赛； 1： 准备阶段； 2： 自检阶段； 
																				//			3： 5s 倒计时； 4： 对战中； 5： 比赛结算中 
	Can1TxMsg.Data[1] = (u8)(jgmt_mesg.robotHurt.armorId);	//其中数值为 0-4 号代表机器人的五个装甲片，其他血量变化类型，该变量数值为 0 
	Can1TxMsg.Data[2] = (u8)(jgmt_mesg.robotHurt.hurtType);//0: 装甲板 1：模块离线 2：超射速 3：超热量 4：超功率 5：撞击 
	Can1TxMsg.Data[3] = (u8)(*(u8 *)&jgmt_mesg.buff); //bit 2： 机器人防御加成, bit 3： 机器人攻击加成 
 
	Can1TxMsg.Data[4] = (u8)((int16_t)jgmt_mesg.powerHeatData.chassisPower >> 8); //底盘功率w 
	Can1TxMsg.Data[5] = (u8)((int16_t)jgmt_mesg.powerHeatData.chassisPower); 
 
	if(JgmtOffline) 
		EnemyColor = 1; //特殊 
	else if (jgmt_mesg.gameRobotStatus.robotId < 8) 
		EnemyColor = 0; //己方红色，敌方蓝色 
	else if (jgmt_mesg.gameRobotStatus.robotId > 9) 
		EnemyColor = 2;												//己方蓝色，敌方红色 
	Can1TxMsg.Data[6] = EnemyColor << 4 | jgmt_mesg.gameRobotStatus.robotLevel; //等级 
 
	oldstate = jgmt_mesg.gameStatus.gameProgress; 
	if (JgmtOffline == 0) 
		CAN_Transmit(CAN1, &Can1TxMsg); 
} 
 
//5ms 
void Send_Distance_Data() 
{ 
	CanTxMsg Can1TxMsg; 
 
	Can1TxMsg.IDE = 0;		 //标准帧 
	Can1TxMsg.RTR = 0;		 //数据帧 
	Can1TxMsg.DLC = 8;		 //帧长度 
	Can1TxMsg.StdId = 0x103; //ID:0x301 
//	Can1TxMsg.Data[0] = (u8)((int16_t)(GYdistance[LF]*200)>>8); 
//	Can1TxMsg.Data[1] = (u8)((int16_t)(GYdistance[LF]*200)); 
//	Can1TxMsg.Data[2] = (u8)((int16_t)(GYdistance[LB]*200)>>8); 
//	Can1TxMsg.Data[3] = (u8)((int16_t)(GYdistance[LB]*200)); 
//	Can1TxMsg.Data[4] = (u8)((int16_t)(GYdistance[FF]*200)>>8); 
//	Can1TxMsg.Data[5] = (u8)((int16_t)(GYdistance[FF]*200)); 
	Can1TxMsg.Data[6] = (u8)((int16_t)(jgmt_mesg.shootData.shooterId)); 
	Can1TxMsg.Data[7] = (u8)((int16_t)(jgmt_mesg.shootData.bulletFreq)); 
	CAN_Transmit(CAN1, &Can1TxMsg); 
} 
 
//20ms 
void Send_Judgement_RobotBasicState_Data() 
{ 
	CanTxMsg Can1TxMsg; 
 
	Can1TxMsg.IDE = 0;		 //标准帧 
	Can1TxMsg.RTR = 0;		 //数据帧 
	Can1TxMsg.DLC = 8;		 //帧长度 
	Can1TxMsg.StdId = 0x403; //ID:0x403 
 
	Can1TxMsg.Data[0] = (u8)((int16_t)jgmt_mesg.gameRobotStatus.maxHp >> 8); //最高血量 
	Can1TxMsg.Data[1] = (u8)((int16_t)jgmt_mesg.gameRobotStatus.maxHp); 
	Can1TxMsg.Data[2] = (u8)((int16_t)jgmt_mesg.gameRobotStatus.shooterId1_17mmCoolingRate >> 8); //冷却速率 
	Can1TxMsg.Data[3] = (u8)((int16_t)jgmt_mesg.gameRobotStatus.shooterId1_17mmCoolingRate); 
	Can1TxMsg.Data[4] = (u8)((int16_t)jgmt_mesg.gameRobotStatus.shooterId1_17mmCoolingLimit >> 8); //热量上限 
	Can1TxMsg.Data[5] = (u8)((int16_t)jgmt_mesg.gameRobotStatus.shooterId1_17mmCoolingLimit); 
	if (JgmtOffline == 0) 
		CAN_Transmit(CAN1, &Can1TxMsg); 
} 
 //0x120

void Send_LIMIT_Data() 
{ 
	CanTxMsg Can1TxMsg; 
 
	Can1TxMsg.IDE = 0;		 //标准帧 
	Can1TxMsg.RTR = 0;		 //数据帧 
	Can1TxMsg.DLC = 8;		 //帧长度 
	Can1TxMsg.StdId = 0x404; //ID:0x404 
 
	Can1TxMsg.Data[0] = (u8)((int16_t)jgmt_mesg.gameRobotStatus.chassisPowerLimit >> 8); //功率上限 
	Can1TxMsg.Data[1] = (u8)((int16_t)jgmt_mesg.gameRobotStatus.chassisPowerLimit); 
	Can1TxMsg.Data[2] = (u8)((int16_t)jgmt_mesg.gameRobotStatus.shooterId1_17mmSpeedLimit >> 8); //射速上限 
	Can1TxMsg.Data[3] = (u8)((int16_t)jgmt_mesg.gameRobotStatus.shooterId1_17mmSpeedLimit); 
	Can1TxMsg.Data[4] = (u8)((int16_t)jgmt_mesg.gameStatus.stageRemainTime >> 8); 
	Can1TxMsg.Data[5] = (u8)((int16_t)jgmt_mesg.gameStatus.stageRemainTime); 
	Can1TxMsg.Data[6] = (u8)((int16_t)0); 
	Can1TxMsg.Data[7] = (u8)((int16_t)0); 
 
	if (JgmtOffline == 0) 
		CAN_Transmit(CAN1, &Can1TxMsg); 
} 
 
 
void canTx(u8 data[8], CAN_TypeDef *can_x, uint32_t id) 
{ 
	CanTxMsg Can1TxMsg; 
 
	Can1TxMsg.IDE = CAN_Id_Standard; //标准帧 CAN_Id_Standard 使用标准标识符 CAN_Id_Extended 使用标准标识符 + 扩展标识符 
	Can1TxMsg.RTR = CAN_RTR_Data;	 //数据帧 CAN_RTR_Data 数据帧 CAN_RTR_Remote 远程帧 
	Can1TxMsg.DLC = 8;				 //帧长度 范围是 0 到 0x8 
 
	Can1TxMsg.StdId = id; //范围为 0 到 0x7FF 
 
	memcpy(Can1TxMsg.Data, data, 8); 
 
	u8 mbox = CAN_Transmit(can_x, &Can1TxMsg); 
} 
