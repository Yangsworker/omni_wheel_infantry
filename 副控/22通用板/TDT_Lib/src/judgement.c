/*******************************************************
*
*需要依赖裁判系统计数的内容需要放在STEP_DATA_CRC16这一步
*
*
********************************************************/
#define __DRIVER_GLOBALS
#include "judgement.h"
#include "string.h"

jgmt_mesg_t jgmt_mesg = {0};
uint8_t jgmt_Update[4];
static const u16 RED_BLUE_ID_DIFF = 100;	 //红蓝ID差
static const u16 ROBOT_CLIENT_ID_DIFF = 256; //机器人与客户端ID差

#define CRC_NO_CALC_STATUS 0	 //不计算CRC16和CRC8
#define CRC8_CRC16_CALC_STATUS 1 //同时计算CRC16和CRC8
#define CRC16_CALC_STATUS 2		 //仅计算CRC16

#define USE_DMA_TRANSFER_DATA 0 //使用DMA进行内存拷贝

float StartTime = 0;	 //时序数据，用于判断指示灯及数据的可靠性与及时性
u8 JgmtOfflineCheck = 0; //裁判系统离线计数
u8 JgmtOffline = 0;		 //裁判系统离线
int hurt_flag = 0;		 //裁判系统受到伤害才更新装甲板,此次与上一次hurtCount不相同即为受攻击

enum JudgementStep judgementStep;

uint8_t judgeDataBuffer[JUDGE_BUFFER_LENGTH];
uint16_t judgementFullCount;

uint8_t fullDataBuffer[128];


/*裁判系统测频用
int Status_data_cnt, Robot_survival_data_cnt, Robot_State_Data_cnt, Real_time_power_and_heat_data_cnt, Robot_position_data_cnt;
*/
int16_t judge_shoot_num = 0; //裁判系统计发弹数

void Judge_Uart3_Config(void) //串口3
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //tx
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;			  //rx
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //floatingGPIO_Mode_IPU
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	USART_DeInit(USART3);
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART3, &USART_InitStructure);
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
	USART_Cmd(USART3, ENABLE);

	DMA_DeInit(DMA1_Channel3);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (USART3->DR);	//外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)judgeDataBuffer;		//内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;						//数据传输方向：外设到内存
	DMA_InitStructure.DMA_BufferSize = JUDGE_BUFFER_LENGTH;					//dma缓存大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设地址不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//内置地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //数据宽度为八位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			//数据宽度为八位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							//工作模式为环形
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;						//dma通道拥有高优先级
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							//非内存到内存传输
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
	DMA_Cmd(DMA1_Channel3, ENABLE);

	DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


}

/**
 * @brief 接收DMA定义
 * @note  由于裁判系统发送一帧的中间会停止发送，故不能用空闲中断+DMA进行读取
 * 		  而使用DMA循环接收+各个字节轮询遍历的方法
 */
void DMA1_Channel3_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_IT_TC3) != RESET)
	{
		JgmtOfflineCheck = 0;
		JgmtOffline = 0;
		judgementFullCount++; //说明到末尾了，需要进行过圈处理
	}
	DMA_ClearITPendingBit(DMA1_IT_TC3);
}


static u8 CRC8_Check = 0xff;		  //参与CRC8校验的计算变量
static uint16_t CRC16_Check = 0xffff; //参与CRC16校验的计算变量
static u8 crcCalcState = 0;			  //CRC8判断到底进行什么校验

static int64_t read_len;	  //已经处理的长度
static int64_t rx_len;		  //已经读取的长度
static uint16_t wholePackLen; //当前解算的帧的总长度
static int32_t index = 0;	  //当前解算的帧在fullDataBuffer的索引
static u8 lastSeq;			  //上一次包序号


u16 shootNum[3];
/**
* @brief  裁判系统数据解析
* @input  void
* @output void
* @note	This function is to analyse data(originally and sketchy) to put a full data frame to the array named protocol_packet.
*/

void judge_Ring_queue(void)
{
	rx_len = JUDGE_BUFFER_LENGTH - DMA_GetCurrDataCounter(DMA1_Channel3) + judgementFullCount * JUDGE_BUFFER_LENGTH; //获取裁判当前已经接收的长度
	if (rx_len > read_len + JUDGE_BUFFER_LENGTH + 1)
		jgmt_mesg.wrongStatusCnt.CYCLE_Wrong_cnt++;//如果读取的长度比接收的长度短一圈（少JUDGE_BUFFER_LENGTH），认为已经过圈，过圈计数器加一

	while (rx_len > read_len + 1) //当没读满时
	{
		int read_arr = read_len % JUDGE_BUFFER_LENGTH; //当前应该读取的字节在judgeDataBuffer的位置
		u8 byte = judgeDataBuffer[read_arr];		   //将该字节取出

		u8 dataLenFromCmdid;							//根据命令id获取长度

		/* 独立计算CRC，平摊算力 */
		if (crcCalcState == CRC8_CRC16_CALC_STATUS)
		{
			CRC8_Check = CRC8_TAB[CRC8_Check ^ (byte)];
			CRC16_Check = ((uint16_t)(CRC16_Check) >> 8) ^ wCRC_Table[((uint16_t)(CRC16_Check) ^ (uint16_t)(byte)) & 0x00ff];
		}

		if (crcCalcState == CRC16_CALC_STATUS)
			CRC16_Check = ((uint16_t)(CRC16_Check) >> 8) ^ wCRC_Table[((uint16_t)(CRC16_Check) ^ (uint16_t)(byte)) & 0x00ff];

		read_len++; //认为该字节已经读取，位置往后移

		switch (judgementStep)
		{
		case STEP_HEADER: //帧头
			if (index == 0)
			{
				if (byte != 0xA5)//错位
				{
					index = 0;
					crcCalcState = CRC8_CRC16_CALC_STATUS;
					CRC8_Check = 0xff;
					CRC16_Check = 0xffff;
					break;
				}
				fullDataBuffer[index++] = byte;
			}
			else if (index < 4)
			{
				fullDataBuffer[index++] = byte;
				if (index == 4) //包序号
				{
					lastSeq++;
					if (((struct FrameHeader *)fullDataBuffer)->seq != lastSeq) //上一帧的包序号不等于这一帧的包序号+1
					{
						jgmt_mesg.wrongStatusCnt.SEQ_Wrong_cnt++; //认为包序号错误，计数器+1
						lastSeq = byte;
					}
					//包序号错误仍进行处理
					crcCalcState = CRC16_CALC_STATUS; //下一帧起不再计算CRC8
					judgementStep = STEP_HEADER_CRC8;
				}
			}
			break;
		case STEP_HEADER_CRC8:
			fullDataBuffer[index++] = byte;
			if (((struct FrameHeader *)fullDataBuffer)->crc8 != CRC8_Check) //CRC8校验正确
			{
				jgmt_mesg.wrongStatusCnt.CRC8_Wrong_cnt++;	   //CRC8校验错误计数器+1
				judgementStep = STEP_HEADER;		   //从头开始
				index = 0;							   //清空fullDataBuffer索引
				crcCalcState = CRC8_CRC16_CALC_STATUS; //CRC8与CRC16都计算
				CRC8_Check = 0xff;					   //初始化CRC8校验
				CRC16_Check = 0xffff;				   //初始化CRC16校验
				break;
			}
			judgementStep = STEP_CMDID_GET; //获取命令码
			CRC8_Check = 0xff;				//初始化CRC8校验
			break;
		case STEP_CMDID_GET:
			fullDataBuffer[index++] = byte;
			if (index != 7) //未获取完命令码
				break;

			judgementStep = STEP_DATA_TRANSFER;
			dataLenFromCmdid= getLength((struct FrameHeader *)fullDataBuffer);

			wholePackLen = 7 + dataLenFromCmdid + 2;
			if (dataLenFromCmdid == 0xFF || dataLenFromCmdid != ((struct FrameHeader *)fullDataBuffer)->dataLength) //命令错误
			{
				judgementStep = STEP_HEADER;		   //从头开始
				index = 0;							   //清空fullDataBuffer索引
				crcCalcState = CRC8_CRC16_CALC_STATUS; //CRC8与CRC16都计算
				CRC8_Check = 0xff;					   //初始化CRC8校验
				CRC16_Check = 0xffff;				   //初始化CRC16校验
			}
			break;
		case STEP_DATA_TRANSFER:
			fullDataBuffer[index++] = byte;
			if (index == (wholePackLen - 2)) //数据传输完，待校验
			{
				crcCalcState = CRC_NO_CALC_STATUS; //下帧起不计算CRC16，避免将帧尾的校验位也计算
				judgementStep = STEP_DATA_CRC16;
			}
			break;
		case STEP_DATA_CRC16:
			fullDataBuffer[index++] = byte;
			if (index == (wholePackLen))
			{
				uint8_t CRC16_char[2];
				CRC16_char[0] = (u8)(CRC16_Check & 0x00ff);
				CRC16_char[1] = (u8)((CRC16_Check >> 8) & 0x00ff);
				if (CRC16_char[0] == fullDataBuffer[index - 2] && CRC16_char[1] == fullDataBuffer[index - 1]) //CRC16校验通过
					getJudgeData();
				else
					jgmt_mesg.wrongStatusCnt.CRC16_Wrong_cnt++; //CRC16校验失败则计数器+1

				//无论是否通过都重置
				judgementStep = STEP_HEADER;		   //从头开始
				index = 0;							   //清空fullDataBuffer索引
				crcCalcState = CRC8_CRC16_CALC_STATUS; //CRC8与CRC16都计算
				CRC8_Check = 0xff;					   //初始化CRC8校验
				CRC16_Check = 0xffff;				   //初始化CRC16校验
			}
			break;
		default:
			judgementStep = STEP_HEADER;		   //从头开始
			index = 0;							   //清空fullDataBuffer索引
			crcCalcState = CRC8_CRC16_CALC_STATUS; //CRC8与CRC16都计算
			CRC8_Check = 0xff;					   //初始化CRC8校验
			CRC16_Check = 0xffff;				   //初始化CRC16校验
			break;
		}
		rx_len = JUDGE_BUFFER_LENGTH - DMA_GetCurrDataCounter(DMA1_Channel3) +
			judgementFullCount * JUDGE_BUFFER_LENGTH; //重新获取已经接收的长度
	}
	if (rx_len % JUDGE_BUFFER_LENGTH > (JUDGE_BUFFER_LENGTH / 3) &&
		rx_len % JUDGE_BUFFER_LENGTH < (2 * JUDGE_BUFFER_LENGTH / 3)) //防止judgementFullCount溢出，过早清除与过晚清除都可能会导致包圈
	{
		read_len -= JUDGE_BUFFER_LENGTH * judgementFullCount;
		judgementFullCount = 0;
	}
}

/**
 * @brief 获取当前帧对应的长度
 *
 * @param frameHeader 帧头结构体
 * @return uint8_t 长度
 */
uint8_t getLength(struct FrameHeader *frameHeader)
{
	switch (frameHeader->cmdid)
	{
	case STATUS_DATA:
		return sizeof(struct GameStatus);
	case RESULT_DATA:
		return sizeof(struct GameResult);
	case ROBOT_HP_DATA:
		return sizeof(union GameRobotHP);
	case DART_STATUS:
		return sizeof(struct DartStatus);
	case ICRA_BUFF_DEBUFF_ZONE_STATUS:
		return sizeof(struct ICRA_BuffDebuffZoneStatus_t);
	case EVENT_DATA:
		return sizeof(struct EventData);
	case SUPPLY_PROJECTILE_ACTION:
		return sizeof(struct SupplyProjectileAction);
	case ROBOT_WARNING_DATA:
		return sizeof(struct RefereeWarning);
	case DART_REMAINING_TIME:
		return sizeof(struct DartRemainingTime);
	case GAME_ROBOT_STATUS:
		return sizeof(struct GameRobotStatus);
	case POWER_HEAT_DATA:
		return sizeof(struct PowerHeatData);
	case GAME_ROBOT_POS:
		return sizeof(struct GameRobotPos);
	case BUFF:
		return sizeof(struct Buff);
	case AERIAL_ROBOT_ENERGY:
		return sizeof(struct AerialRobotEnergy);
	case ROBOT_HURT:
		return sizeof(struct RobotHurt);
	case SHOOT_DATA:
		return sizeof(struct ShootData);
	case BULLET_REMAINING:
		return sizeof(struct BulletRemaining);
	case RFID_STATUS:
		return sizeof(struct RfidStatus);
	case DART_CLIENT_CMD:
		return sizeof(struct DartClientCmd);

	case STUDENT_INTERACTIVE_HEADER_DATA:
		return frameHeader->dataLength;
	}
	return 0xff;
}

/**
 * @brief 结构体复制
 *
 */
void getJudgeData()
{
	unsigned char *judgeData_ADD;
	u8 update[4] = {0};
#if USE_DMA_TRANSFER_DATA
	static NVIC_InitTypeDef NVIC_InitStructure;
	static DMA_InitTypeDef DMA_InitStructure;
	static u8 firstLoad = 1; //首次加载时加载配置缺省值
	if (firstLoad == 1)
	{
		firstLoad = 0;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)fullDataBuffer + sizeof(struct FrameHeader); //外设基地址
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;												  //数据传输方向：外设到内存
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;									  //外设地址递增
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;											  //内置地址递增
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;							  //数据宽度为八位
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;									  //数据宽度为八位
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;													  //不执行循环模式
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;												  //dma通道拥有高优先级
		DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;														  //内存到内存传输

		NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

		DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, DISABLE); //失能传输完成中断中断
	}
#endif
	u16 cmdid = ((struct FrameHeader *)fullDataBuffer)->cmdid;
	switch (cmdid)
	{
	case STATUS_DATA:
		judgeData_ADD = (u8 *)&jgmt_mesg.gameStatus;
		break; //10Hz

	case RESULT_DATA:
		judgeData_ADD = (u8 *)&jgmt_mesg.gameResult;
		break;

	case ROBOT_HP_DATA:
		update[1] = 1;
		judgeData_ADD = (u8 *)&jgmt_mesg.gameRobotHP;
		break;

	case DART_STATUS:
		judgeData_ADD = (u8 *)&jgmt_mesg.dartStatus;
		break;

	case ICRA_BUFF_DEBUFF_ZONE_STATUS:
		update[2] = 1;
		update[3] = 1;
		judgeData_ADD = (u8 *)&jgmt_mesg.ICRA_BuffDebuffZoneStatus;
		break;

	case EVENT_DATA:
		judgeData_ADD = (u8 *)&jgmt_mesg.eventData;
		break; //50hZ

	case SUPPLY_PROJECTILE_ACTION:
		judgeData_ADD = (u8 *)&jgmt_mesg.supplyProjectileAction;
		break; //10hz

	case ROBOT_WARNING_DATA:
		judgeData_ADD = (u8 *)&jgmt_mesg.refereeWarning;
		break;

	case DART_REMAINING_TIME:
		judgeData_ADD = (u8 *)&jgmt_mesg.dartRemainingTime;
		break;

	case GAME_ROBOT_STATUS:
		update[0] = 1;
		judgeData_ADD = (u8 *)&jgmt_mesg.gameRobotStatus;
		break;

	case POWER_HEAT_DATA:
		judgeData_ADD = (u8 *)&jgmt_mesg.powerHeatData;
		break; //50hz

	case GAME_ROBOT_POS:
		judgeData_ADD = (u8 *)&jgmt_mesg.gameRobotPos;
		break;

	case BUFF:
		judgeData_ADD = (u8 *)&jgmt_mesg.buff;
		break;

	case AERIAL_ROBOT_ENERGY:
		judgeData_ADD = (u8 *)&jgmt_mesg.aerialRobotEnergy;
		break;

	case ROBOT_HURT:
		judgeData_ADD = (u8 *)&jgmt_mesg.robotHurt;
		break;

	case SHOOT_DATA:
		judgeData_ADD = (u8 *)&jgmt_mesg.shootData;
		break;

	case BULLET_REMAINING:
		judgeData_ADD = (u8 *)&jgmt_mesg.bulletRemaining;
		break;

	case RFID_STATUS:
		judgeData_ADD = (u8 *)&jgmt_mesg.rfidStatus;
		break;

	case DART_CLIENT_CMD:
		judgeData_ADD = (u8 *)&jgmt_mesg.dartClientCmd;
		break;

	case STUDENT_INTERACTIVE_HEADER_DATA:
		judgeData_ADD = (u8 *)&jgmt_mesg.studentRecviveData;
		break;

	default:
		return;
	}

	// memcpy(judgeData_ADD,fullDataBuffer+sizeof(struct FrameHeader),getLength((FrameHeader*)fullDataBuffer));

#if USE_DMA_TRANSFER_DATA
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)judgeData_ADD;						//内存基地址
	DMA_InitStructure.DMA_BufferSize = getLength((struct FrameHeader *)fullDataBuffer); //dma缓存大小
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1, ENABLE);
	NVIC_Init(&NVIC_InitStructure);
#else
	//采用内存拷贝，保证can进入can中断前拷贝完
	memmove(judgeData_ADD, fullDataBuffer + sizeof(struct FrameHeader), getLength((struct FrameHeader *)fullDataBuffer));
#endif
	for (int i = 0; i < 4; i++)
	{
		jgmt_Update[i] = update[i] | jgmt_Update[i];
	}

	switch (cmdid)
	{
	case SUPPLY_PROJECTILE_ACTION:
		if(jgmt_mesg.supplyProjectileAction.supplyRobotId == jgmt_mesg.gameRobotStatus.robotId)
		{
			extern int16_t customUIBulletTest;
			customUIBulletTest += jgmt_mesg.supplyProjectileAction.supplyProjectileNum;
		}
		break;
	case ROBOT_HP_DATA:
		update[1] = 1;
		break;
	case ICRA_BUFF_DEBUFF_ZONE_STATUS:
		update[2] = 1;
		update[3] = 1;
		break;
	case GAME_ROBOT_STATUS:
		update[0] = 1;
		break;
	case ROBOT_HURT:
		hurt_flag++;
		break;
	case SHOOT_DATA:
		shootNum[jgmt_mesg.shootData.shooterId-1]++;
		extern int16_t customUIBulletTest;
		customUIBulletTest = MAX(customUIBulletTest-1, 0);
	default:
		break;
	}
}

/**
 * @brief 针对0x301的联合体数据进行发送
 *
 * @param count
 */
void customSend(u8 count)
{
	jgmt_mesg.sendUnionData.frameHeader.sof = 0xa5;									//帧头
	jgmt_mesg.sendUnionData.frameHeader.dataLength = count - sizeof(struct FrameHeader) - 2; //数据长度
	Append_CRC8_Check_Sum((unsigned char *)&jgmt_mesg.sendUnionData, 5);				//CRC8校验
	jgmt_mesg.sendUnionData.senderId = jgmt_mesg.gameRobotStatus.robotId;						//发送方ID

	if (jgmt_mesg.sendUnionData.receiverId != ClientId())						   //不是客户端
		jgmt_mesg.sendUnionData.receiverId = IdToMate(IdToIdDef(jgmt_mesg.sendUnionData.receiverId)); //转化为友方

	Append_CRC16_Check_Sum((unsigned char *)&jgmt_mesg.sendUnionData, count);

	uart3SendBytes(&jgmt_mesg.sendUnionData, count);
}

/**
 * @brief 发送机器人交互数据
 *
 * @param dataLength 交互内容的长度
 */
void robotsCommunication(uint16_t dataCmdId, enum RobotIdDef robotIdDef, u8 dataLength)
{
	jgmt_mesg.sendUnionData.dataCmdId = dataCmdId;
	jgmt_mesg.sendUnionData.receiverId = robotIdDef;
	int count = sizeof(struct FrameHeader) + 6 + dataLength + 2;
	jgmt_mesg.sendUnionData.frameHeader.cmdid = 0x0301;
	//    unsigned char send_Buffer[17];
	customSend(count);
}

/**
 * @brief 发送客户端UI命令
 *
 * @param sendGraphics 一次发送的图形数目
 */
void graphicDraw(u8 sendGraphics)
{
	if (sendGraphics <= 1)
	{
		jgmt_mesg.sendUnionData.dataCmdId = 0x101;
		sendGraphics = 1;
	}
	else if (sendGraphics <= 2)
	{
		jgmt_mesg.sendUnionData.dataCmdId = 0x102;
		sendGraphics = 2;
	}
	else if (sendGraphics <= 5)
	{
		jgmt_mesg.sendUnionData.dataCmdId = 0x103;
		sendGraphics = 5;
	}
	else if (sendGraphics <= 7)
	{
		jgmt_mesg.sendUnionData.dataCmdId = 0x104;
		sendGraphics = 7;
	}
	jgmt_mesg.sendUnionData.frameHeader.cmdid = 0x0301;

	int count = sizeof(struct FrameHeader) + 6 + sendGraphics * sizeof(union GraphicDataUnion) + 2;
	jgmt_mesg.sendUnionData.receiverId = ClientId();
	jgmt_mesg.sendUnionData.senderId = jgmt_mesg.gameRobotStatus.robotId;

	customSend(count);
}

/**
 * @brief 字符绘制
 *
 */
void characterDraw()
{
	jgmt_mesg.sendUnionData.frameHeader.cmdid = 0x0301;
	jgmt_mesg.sendUnionData.dataCmdId = 0x110;
	int count = sizeof(struct FrameHeader) + 6 + sizeof(union GraphicDataUnion) + 30 + 2;
	jgmt_mesg.sendUnionData.receiverId = ClientId();
	jgmt_mesg.sendUnionData.senderId = jgmt_mesg.gameRobotStatus.robotId;

	customSend(count);
}

/**
 * @brief 删除图形
 *
 */
void graphicDel(void)
{
	jgmt_mesg.sendUnionData.frameHeader.cmdid = 0x0301;
	jgmt_mesg.sendUnionData.dataCmdId = 0x100;
	int count = sizeof(struct FrameHeader) + 6 + sizeof(struct ClientCustomGraphicDelete) + 2;
	jgmt_mesg.sendUnionData.receiverId = ClientId();
	jgmt_mesg.sendUnionData.senderId = jgmt_mesg.gameRobotStatus.robotId;

	customSend(count);
}

/**
 * @brief 发送地图命令
 *
 */
void mapCommandSend(void)
{
	jgmt_mesg.mapCommandData.frameHeader.cmdid = 0x0303;
	int count = sizeof(struct FrameHeader) + sizeof(struct MapCommand) + 2;

	jgmt_mesg.mapCommandData.frameHeader.sof = 0xa5;
	jgmt_mesg.mapCommandData.frameHeader.dataLength = count - sizeof(struct FrameHeader) - 2;
	Append_CRC8_Check_Sum((unsigned char *)&jgmt_mesg.mapCommandData, 5);
	Append_CRC16_Check_Sum((unsigned char *)&jgmt_mesg.mapCommandData, count);

	uart3SendBytes(&jgmt_mesg.mapCommandData, count);
}

void uart3SendBytes(void *ptr, u8 len)
{
	static DMA_InitTypeDef DMA_InitStructure;
	static NVIC_InitTypeDef NVIC_InitStructure;
	static u8 firstLoad = 1; //首次加载时加载配置缺省值
	if (firstLoad == 1)
	{
		firstLoad = 0;
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (USART3->DR); //外设基地址
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;					  //数据传输方向：内存到外设
		//dma缓存大小
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设地址不变
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//内置地址寄存器递增
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //数据宽度为八位
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			//数据宽度为八位
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							//工作模式为环形
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;						//dma通道拥有高优先级
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							//非内存到内存传输

		NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

		USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
		DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, DISABLE);
	}

	DMA_Cmd(DMA1_Channel2, DISABLE);
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ptr; //内存基地址
	DMA_InitStructure.DMA_BufferSize = len;				  //dma缓存大小
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel2, ENABLE);
	NVIC_Init(&NVIC_InitStructure);
//if (firstLoad == 1)
//	{
//		firstLoad = 0;
//		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (USART1->DR); //外设基地址
//		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;					  //数据传输方向：内存到外设
//		//dma缓存大小
//		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设地址不变
//		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//内置地址寄存器递增
//		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //数据宽度为八位
//		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			//数据宽度为八位
//		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							//工作模式为环形
//		DMA_InitStructure.DMA_Priority = DMA_Priority_High;						//dma通道拥有高优先级
//		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							//非内存到内存传输

//		NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
//		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
//		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

//		USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
//		DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, DISABLE);
//	}

//	DMA_Cmd(DMA1_Channel4, DISABLE);
//	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ptr; //内存基地址
//	DMA_InitStructure.DMA_BufferSize = len;				  //dma缓存大小
//	DMA_Init(DMA1_Channel4, &DMA_InitStructure);
//	DMA_Cmd(DMA1_Channel4, ENABLE);
//	NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief 将ID枚举转换为友方机器人id
 *
 * @param robotIdDef ID枚举
 * @return uint16_t 友方机器人id
 */
uint16_t IdToMate(enum RobotIdDef robotIdDef)
{
	if (jgmt_mesg.gameRobotStatus.robotId > RED_BLUE_ID_DIFF)
	{
		return (uint16_t)robotIdDef + RED_BLUE_ID_DIFF;
	}
	return (uint16_t)robotIdDef;
}

/**
 * @brief 根据自身id返还客户端id
 *
 * @return uint16_t 客户端id
 */
uint16_t ClientId()
{
	return jgmt_mesg.gameRobotStatus.robotId + ROBOT_CLIENT_ID_DIFF;
}

/**
 * @brief 将ID枚举转换为敌方机器人id
 *
 * @param robotIdDef ID枚举
 * @return uint16_t 敌方机器人id
 */
uint16_t IdToEnemy(enum RobotIdDef robotIdDef)
{
	if (jgmt_mesg.gameRobotStatus.robotId < RED_BLUE_ID_DIFF)
	{
		return (uint16_t)robotIdDef + RED_BLUE_ID_DIFF;
	}
	return robotIdDef;
}

enum RobotIdDef IdToIdDef(uint16_t robotId)
{
	return (enum RobotIdDef)(robotId % RED_BLUE_ID_DIFF);
}
