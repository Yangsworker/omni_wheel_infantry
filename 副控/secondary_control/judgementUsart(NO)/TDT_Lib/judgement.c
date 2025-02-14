/*******************************************************
*
*��Ҫ��������ϵͳ������������Ҫ����STEP_DATA_CRC16��һ��
*
*
********************************************************/
#define __DRIVER_GLOBALS
#include "judgement.h"
float StartTime = 0;			//ʱ�����ݣ������ж�ָʾ�Ƽ����ݵĿɿ����뼰ʱ��
u8 JgmtOfflineCheck = 0;		//����ϵͳ���߼���
u8 JgmtOffline = 0;				//����ϵͳ����
int16_t judgement_full_count = 0;
int hurt_flag;					//����ϵͳ�ܵ��˺��Ÿ���װ�װ�, �������ж��Ƿ����1s���Ƿ��ܵ�����

union robot_interactive_data_t send_interactive_data;

jgmt_mesg_t jgmt_mesg;

/*����ϵͳ��Ƶ��
int Status_data_cnt, Robot_survival_data_cnt, Robot_State_Data_cnt, Real_time_power_and_heat_data_cnt, Robot_position_data_cnt;
*/
int16_t judge_shoot_num = 0;	//����ϵͳ�Ʒ�����


void Judge_Uart3_Config(void)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef  GPIO_InitStructure;
    DMA_InitTypeDef   DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//tx
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&GPIO_InitStructure);


    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//rx
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//floatingGPIO_Mode_IPU
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&GPIO_InitStructure);

    USART_DeInit(USART3);
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_Mode = USART_Mode_Tx|USART_Mode_Rx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART3,&USART_InitStructure);
    USART_ITConfig(USART3,USART_IT_IDLE,ENABLE);
    USART_Cmd(USART3,ENABLE);

    DMA_DeInit(DMA1_Channel3);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART3->DR);		//�������ַ
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)JudgeDataBuffer; 		//�ڴ����ַ
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;		//���ݴ��䷽�����赽�ڴ�
    DMA_InitStructure.DMA_BufferSize = JudgeBufferLength;			//dma�����С
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//�����ַ����
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;		//���õ�ַ�Ĵ�������
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//���ݿ��Ϊ��λ
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			//���ݿ��Ϊ��λ
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;					//����ģʽΪ����
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;			//dmaͨ��ӵ�и����ȼ�
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;	//���ڴ浽�ڴ洫��
    DMA_Init(DMA1_Channel3,&DMA_InitStructure);
    USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE);
    DMA_Cmd(DMA1_Channel3,ENABLE);

    DMA_ITConfig(DMA1_Channel3,DMA_IT_TC,ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  DMA��������жϣ����ڼ���������ָ��
  * @input  void
  * @output void
  * @note   This handler function is used to calculate handling circle
  */
void DMA1_Channel3_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC3) != RESET)
    {
        JgmtOfflineCheck = 0;
        judgement_full_count++;
    }
    DMA_ClearITPendingBit(DMA1_IT_TC3);
}


/**
  * @brief  ����ת��4��U8�ϳ�1��float
  * @input  u8*4
  * @output float
  * @note   This function is used to change character-string to floating-point.
  */
float u8toflaot(uint8_t u4,uint8_t u3,uint8_t u2,uint8_t u1)
{
    FormatTrans FT;
    FT.U[3] = u4;
    FT.U[2] = u3;
    FT.U[1] = u2;
    FT.U[0] = u1;
    return FT.F;
}



/**
  * @brief  ����ת��1��float�ֽ��4��U8
  * @input  u8*4
  * @output float
  * @note   This function is used to change floating-point to character-string.
  */
_vec4u floattou8(float f1)
{
    FormatTrans FT;
    _vec4u u;
    FT.F = f1;
    u.data[0] = FT.U[0];
    u.data[1] = FT.U[1];
    u.data[2] = FT.U[2];
    u.data[3] = FT.U[3];
    return u;
}

/**
  * @brief  ���ݸ�ֵ
  * @input  idʶ�� ���ݣ� ƫ��ָ��
  * @output none
  * @note   This function is used to change floating-point to character-string.
  */
void data_transfer(unsigned short cmdid, unsigned char data, unsigned short index)
{
    switch (cmdid)
    {
    case Status_data://0x0001
        jgmt_mesg.jgmt_game_status.dataBuff[index] = data;
        return;
    case Result_data://0x0002
        jgmt_mesg.jgmt_game_result.dataBuff = data;
        return;
    case Robot_HP_data://0x0003
        jgmt_mesg.jgmt_game_robot_HP.dataBuff[index] = data;
        return;
    case Site_event_data://0x0101
        jgmt_mesg.jgmt_event_data.dataBuff[index] = data;
        return;
    case Site_action_identification_data://0x0102
        jgmt_mesg.jgmt_supply_projectile_action.dataBuff[index] = data;
        return;
    case Reservation_bullet_data_for_site_supply_station://0x0103
        jgmt_mesg.jgmt_supply_projectile_booking.dataBuff[index] = data;
        return;
    case Robot_warning_data://0x0104
        jgmt_mesg.jgmt_referee_warning.dataBuff[index] = data;
        return;
    case Robot_State_Data://0x0201
        jgmt_mesg.jgmt_game_robot_status.dataBuff[index] = data;
        return;
    case Real_time_power_and_heat_data://0x202
        jgmt_mesg.jgmt_power_heat_data.dataBuff[index] = data;
        return;
    case Robot_position_data://0x203
        jgmt_mesg.jgmt_game_robot_pos.dataBuff[index] = data;
        return;
    case Robot_buff_data://0x0204
        jgmt_mesg.jgmt_buff.dataBuff = data;
        return;
    case Energy_State_Data_of_Air_Robot://0x0205
        jgmt_mesg.jgmt_aerial_robot_energy.dataBuff[index] = data;
        return;
    case Damage_status_data://0x0206
        jgmt_mesg.jgmt_robot_hurt.dataBuff = data;
        return;
    case Real_time_shooting_data://0x0207
        jgmt_mesg.jgmt_shoot_data.dataBuff[index] = data;
        return;
    case Last_bullet_num_data://0x0208
        jgmt_mesg.jgmt_bullet_remaining.dataBuff[index] = data;
        return;
    case Interactive_data_between_robots://0x0301
        jgmt_mesg.jgmt_robot_interactive_data.dataBuff[index] = data;
    default:
        return;
    }
}

/**
  * @brief  ��ȡ���ݰ�����
  * @input  CMD_ID
  * @output length
  * @note   This function is used to get length according to CMD_ID, which can help CRC_Check_Sum.
  */
/*��ȡ���ݳ��Ⱥ�����0x301���ݳ��Ȳ��������0xFE����Ϊ����ID������0xFF(����)��*/
unsigned char get_length(unsigned short cmdid)
{
    switch (cmdid)
    {
    case Status_data://0x0001
        return 3;
    case Result_data://0x0002
        return 1;
    case Robot_HP_data://0x0003
        return 28;
    case Site_event_data://0x0101
        return 4;
    case Site_action_identification_data://0x0102
        return 4;
    case Reservation_bullet_data_for_site_supply_station://0x0103
        return 2;
    case Robot_warning_data://0x0104
        return 2;
    case Robot_State_Data://0x0201
        return 15;
    case Real_time_power_and_heat_data://0x202
        return 14;
    case Robot_position_data://0x203
        return 16;
    case Robot_buff_data://0x0204
        return 1;
    case Energy_State_Data_of_Air_Robot://0x0205
        return 3;
    case Damage_status_data://0x0206
        return 1;
    case Real_time_shooting_data://0x0207
        return 6;
    case Last_bullet_num_data://0x0208
        return 2;
    case Interactive_data_between_robots://0x0301
        return 0xFE;
    default:
        return 0xFF;
    }
}
/**
  * @brief  ����ϵͳ���ݽ���
  * @input  void
  * @output void
  * @note	This function is to analyse data(originally and sketchy) to put a full data frame to the array named protocol_packet.
  */
int CRC_Wrong_num;
int Cycle_Wrong_num;
void judge_Ring_queue(void)
{
    static uint8_t byte = 0;
    static int64_t read_len, rx_len;
    int read_arr;
    static uint16_t      len;
    static unpack_step_e unpack_step;
    static int32_t       index_in_data = 0, index_in_header = 0;
    static u8 start_state = 0;

    rx_len = JudgeBufferLength - DMA_GetCurrDataCounter(DMA1_Channel3) + judgement_full_count * JudgeBufferLength;
    while(rx_len > read_len + 1)
    {
        read_arr = read_len % JudgeBufferLength;
        byte = JudgeDataBuffer[read_arr];
        read_len++;
        switch(unpack_step)
        {
        case STEP_HEADER:
            if (start_state == 0)
            {
                if (byte == 0xA5)
                {
                    jgmt_mesg.jgmt_frame_header.dataBuff[index_in_header++] = byte;
                    start_state = 1;
                }
                else
                {
                    index_in_header = 0;
                    start_state = 0;
                }
            }
            else
            {
                if (index_in_header < 4-1)
                {
                    jgmt_mesg.jgmt_frame_header.dataBuff[index_in_header++] = byte;
                }
                else
                {
                    unpack_step = STEP_HEADER_CRC8;
                    jgmt_mesg.jgmt_frame_header.dataBuff[index_in_header++] = byte;
                }
            }
            break;
        case STEP_HEADER_CRC8:
        {
            jgmt_mesg.jgmt_frame_header.dataBuff[index_in_header++] = byte;
            if (Verify_CRC8_Check_Sum(jgmt_mesg.jgmt_frame_header.dataBuff, 5))
            {
                unpack_step = STEP_CMDID_GET;
            }
            else
            {
                unpack_step = STEP_HEADER;
                index_in_header = 0;
                start_state = 0;
            }
        }
        break;
        case STEP_CMDID_GET:
        {
            if (index_in_header < 7 - 1)
                jgmt_mesg.jgmt_frame_header.dataBuff[index_in_header++] = byte;
            else
            {
                jgmt_mesg.jgmt_frame_header.dataBuff[index_in_header++] = byte;
                unpack_step = STEP_DATA_TRANSFER;
                len = get_length(jgmt_mesg.jgmt_frame_header.data.cmdid);
                if (len == 0xFE)
                {
                    len = jgmt_mesg.jgmt_frame_header.data.data_length + 7;
                    index_in_data = 7;
                }
                else index_in_data = 0;
				if(len == 0xFF)
				{
					unpack_step = STEP_HEADER;
					index_in_header = 0;
					start_state = 0;
				}
            }
        }
        break;
        case STEP_DATA_TRANSFER:
        {
            if (index_in_data < len - 1)
            {
                data_transfer(jgmt_mesg.jgmt_frame_header.data.cmdid, byte, index_in_data);
            }
            else
            {
                data_transfer(jgmt_mesg.jgmt_frame_header.data.cmdid, byte, index_in_data);
                unpack_step = STEP_DATA_CRC16;
                index_in_data = 0;
            }
            index_in_data++;
        }
        break;
        case STEP_DATA_CRC16:
        {
            if (index_in_data <= 1)
            {
                index_in_data++;
                jgmt_mesg.jgmt_frame_header.dataBuff[index_in_header++] = byte;
            }
            else
            {
                jgmt_mesg.jgmt_frame_header.dataBuff[index_in_header++] = byte;
                unpack_step = STEP_HEADER;
                index_in_header = 0;
                start_state = 0;
            }
            /***********��Ҫ���������ݿ��Է�������߷���schedule.c************/
            if(jgmt_mesg.jgmt_frame_header.data.cmdid == Real_time_shooting_data)
            {
                judge_shoot_num++;
            }
        }
        break;
        default:
        {
            unpack_step = STEP_HEADER;
            index_in_header = 0;
            start_state = 0;
        }
        break;
        }
        rx_len = JudgeBufferLength - DMA_GetCurrDataCounter(DMA1_Channel3) + judgement_full_count * JudgeBufferLength;
    }
    if(rx_len % JudgeBufferLength > (JudgeBufferLength/3) && rx_len % JudgeBufferLength < (2 * JudgeBufferLength / 3))//��ֹjudgement_full_count���
    {
        read_len -= JudgeBufferLength * judgement_full_count;
        judgement_full_count = 0;
    }
}
/**
  * @brief  ��ȡ����ID����
  * @input  mate_id�� ������judgment.h(Line 17)
  * @output ������ ����0�� ���򷵻ض���ID
  * @note	This function is to analyse data(originally and sketchy) to put a full data frame to the array named protocol_packet.
  */
u8 get_mate_id(u8 mate_id)
{
    u8 robot_id = 0;
    if(jgmt_mesg.jgmt_game_robot_status.data.robot_id	> 0xa)//����
    {
        robot_id = 1;
    }
    if(jgmt_mesg.jgmt_game_robot_HP.HP_Buff[mate_id - 1 + robot_id * 8] > 0)
    {
        return mate_id - 1 + robot_id * 8;
    }
    return 0;//�޴��
}

void Uart3_SendChar(unsigned char b)
{
    while (USART_GetFlagStatus(USART3,USART_FLAG_TC) == RESET);
    USART_SendData(USART3,b);
}


/**
  * @brief  �����ϵͳ��������
* @param  content_ID:��������ϵͳ������ID��0xd180���͸��ͻ��ˣ� 0x200-0x2ff�������������ˣ� ע��ͨѶƵ��
  * @retval void
  * @note	���ڸú����������ݽϴ󣬹ʿɿ����ڸó������Ԥ�裨�������ø��ط�1������������ϵͳ���͡�1������Ӧ��Ԥ�裩�Լ������ظ�����canռ����
  */
union client_graphic_draw_t client_graphic_draw;
void judgement_send(u16 content_ID)
{

    int i;

    if(content_ID == 0xd180)
    {
        jgmt_mesg.jgmt_client_custom_data.data.frame_header.sof = 0XA5;
        jgmt_mesg.jgmt_client_custom_data.data.frame_header.data_length = 0x13;
        jgmt_mesg.jgmt_client_custom_data.data.frame_header.seq = 0;

        Append_CRC8_Check_Sum(jgmt_mesg.jgmt_client_custom_data.dataBuff,5);

        jgmt_mesg.jgmt_client_custom_data.data.frame_header.cmdid = 0x0301;
        jgmt_mesg.jgmt_client_custom_data.data.ID.data_cmd_id = 0xd180;
        jgmt_mesg.jgmt_client_custom_data.data.ID.sender_ID = jgmt_mesg.jgmt_game_robot_status.data.robot_id;
        if(jgmt_mesg.jgmt_game_robot_status.data.robot_id < 0xa)//�췽
        {
            jgmt_mesg.jgmt_client_custom_data.data.ID.receiver_ID = (0x0100 + jgmt_mesg.jgmt_game_robot_status.data.robot_id);
        }
        else if(jgmt_mesg.jgmt_game_robot_status.data.robot_id > 0xa)//����
        {
            jgmt_mesg.jgmt_client_custom_data.data.ID.receiver_ID = (0x110 + jgmt_mesg.jgmt_game_robot_status.data.robot_id - 0xa);//��ʹ
        }

        Append_CRC16_Check_Sum(jgmt_mesg.jgmt_client_custom_data.dataBuff,28);

        i = 0;
        while(i<28)
        {
            Uart3_SendChar(jgmt_mesg.jgmt_client_custom_data.dataBuff[i]);
            i++;
        }
        return;
    }
    else if(content_ID == 0x100)
	{
		client_graphic_draw.data.frame_header.sof = 0xA5;
		client_graphic_draw.data.frame_header.data_length = 61;
		client_graphic_draw.data.frame_header.seq = 0;
		Append_CRC8_Check_Sum(client_graphic_draw.dataBuff, 5);
		
		client_graphic_draw.data.frame_header.cmdid = 0x301;
		client_graphic_draw.data.ID.data_cmd_id = 0x100;
		client_graphic_draw.data.ID.sender_ID = jgmt_mesg.jgmt_game_robot_status.data.robot_id;
		if(jgmt_mesg.jgmt_game_robot_status.data.robot_id < 0xa)//�췽
        {
            client_graphic_draw.data.ID.receiver_ID = (0x0100 + jgmt_mesg.jgmt_game_robot_status.data.robot_id);
        }
        else if(jgmt_mesg.jgmt_game_robot_status.data.robot_id > 0xa)//����
        {
            client_graphic_draw.data.ID.receiver_ID = (0x110 + jgmt_mesg.jgmt_game_robot_status.data.robot_id - 0xa);//��ʹ
        }
		Append_CRC16_Check_Sum(client_graphic_draw.dataBuff, 70);
        i = 0;
        while(i<70)
        {
            Uart3_SendChar(client_graphic_draw.dataBuff[i]);
            i++;
        }
		
	}
	else
    {

        send_interactive_data.data.frame_header.sof = 0XA5;
        send_interactive_data.data.frame_header.data_length = 0x77;
        send_interactive_data.data.frame_header.seq = 0;

        Append_CRC8_Check_Sum(send_interactive_data.dataBuff,5);

        send_interactive_data.data.frame_header.cmdid = 0x0301;
        send_interactive_data.data.ID.data_cmd_id = content_ID;
        send_interactive_data.data.ID.sender_ID = jgmt_mesg.jgmt_game_robot_status.data.robot_id;
        send_interactive_data.data.ID.receiver_ID = 1;//eg:get_mate_id(HERO);

        {   /**************��ֹ����idΪ�з�id���·��ʹ���************/
            if(send_interactive_data.data.ID.receiver_ID > 0xa && jgmt_mesg.jgmt_game_robot_status.data.robot_id < 0xa)
            {
                send_interactive_data.data.ID.receiver_ID -= 10;
            }
            else if(send_interactive_data.data.ID.receiver_ID < 0xa && jgmt_mesg.jgmt_game_robot_status.data.robot_id > 0xa)
            {
                send_interactive_data.data.ID.receiver_ID += 10;
            }
        }

        Append_CRC16_Check_Sum(send_interactive_data.dataBuff,128);
        i = 0;
        while(i<128)
        {
            Uart3_SendChar(send_interactive_data.dataBuff[i]);
            i++;
        }
    }
}

void Judge_send_to_client(void)
{
    u8 num = 0;
    can1Feedback.CPU_Offline_Check++;
    if(can1Feedback.CPU_Offline_Check > 5)	//5 * 100ms
    {
        can1Feedback.CPU_Offline_Check = 250;//��ֹ���
        if(can1Feedback.CPU_Offline == 0)
        {
            for(num = 0; num < 6; num++)
            {
                can1Feedback.Light[num] = 0;
                can1Feedback.Light_FastToggle[num] = 0;
            }
        }
        can1Feedback.CPU_Offline = 1;
        for(num = 0; num < 6; num++)
            can1Feedback.Light_Toggle[num] = 1;
    }
    else
    {
        if(can1Feedback.CPU_Offline == 1)
        {
            for(num = 0; num < 6; num++)
                can1Feedback.Light_Toggle[num] = 0;
        }
        can1Feedback.CPU_Offline = 0;
    }

    jgmt_mesg.jgmt_client_custom_data.data.ID.data_cmd_id = 0xd180;

    jgmt_mesg.jgmt_client_custom_data.data.ID.sender_ID = jgmt_mesg.jgmt_game_robot_status.data.robot_id;						//��ʹ
    if(jgmt_mesg.jgmt_game_robot_status.data.robot_id < 0xa)
        jgmt_mesg.jgmt_client_custom_data.data.ID.receiver_ID = 0x0100 + jgmt_mesg.jgmt_game_robot_status.data.robot_id;		//��ʹ
    else if(jgmt_mesg.jgmt_game_robot_status.data.robot_id > 0xa)
        jgmt_mesg.jgmt_client_custom_data.data.ID.receiver_ID = 0x0110 + (jgmt_mesg.jgmt_game_robot_status.data.robot_id - 0xa);

    if(can1Feedback.EmergencyClosePower == 0) jgmt_mesg.jgmt_client_custom_data.data.data1 = can1Feedback.SuperPowerRemain_P;
    else jgmt_mesg.jgmt_client_custom_data.data.data1 = -can1Feedback.SuperPowerRemain_P;
    if(can1Feedback.SpdFeedBack_B != 0) jgmt_mesg.jgmt_client_custom_data.data.data2 = (float)can1Feedback.SpdFeedBack_A + (float)(can1Feedback.SpdFeedBack_B * 0.01);
    else jgmt_mesg.jgmt_client_custom_data.data.data2 = (float)can1Feedback.SpdFeedBack_A;

    if(can1Feedback.ForeOpenLoop == 1) jgmt_mesg.jgmt_client_custom_data.data.data2 = -ABS(jgmt_mesg.jgmt_client_custom_data.data.data2);
    if(can1Feedback.VisionShoot == 1) jgmt_mesg.jgmt_client_custom_data.data.data3 = -ABS(StartTime);
    else jgmt_mesg.jgmt_client_custom_data.data.data3 = ABS(StartTime);

    //�����ң�1Ϊ����ĵ�������0x40Ϊȫ��
    //Light[0]Ϊ����
    jgmt_mesg.jgmt_client_custom_data.data.mask = can1Feedback.Light[5]<<5|can1Feedback.Light[4]<<4|can1Feedback.Light[3]<<3|can1Feedback.Light[2]<<2|can1Feedback.Light[1]<<1|can1Feedback.Light[0];

    judgement_send(jgmt_mesg.jgmt_client_custom_data.data.ID.data_cmd_id);
}



#define MID_X 960
#define MID_Y 540
#define Range_X(x) LIMIT(x,0,1919)
#define Range_Y(y) LIMIT(y,0,1079)



#define CROSSHAIR_LAYER 0//׼��ͼ��
#define BACK_CAMERA_LAYER 1

//���½�Ϊ0,0
//׼�Ļ���(����schedule.c��,type = 1 ʱ��Ҫ��������(һ��һ������).type = 2 ʱ��Ҫ���ѵ���type = 1�������µ���type = 2Լ10��)
//type = 1:T�ּ��⣬ 2:�ѻ����(δд)
void aim_point_draw(u8 type, int16_t offset_x, int16_t offset_y, u8 WIDE, u16 LENTH, u8 progress)
{
	static u8 last_type = 0;
	if(last_type != type)//�л�
	{
		last_type = 1;
		progress = 0;
	}
	switch(progress)
	{
		case 0://   -
		{
			client_graphic_draw.data.operate_tpye = 1;
			client_graphic_draw.data.graphic_tpye = 1;
			client_graphic_draw.data.graphic_name.number= progress;
			client_graphic_draw.data.layer = CROSSHAIR_LAYER;
			client_graphic_draw.data.color = 1;
			client_graphic_draw.data.width = 3;
			client_graphic_draw.data.start_x = (offset_x + MID_X + WIDE / 2);
			client_graphic_draw.data.end_x = (offset_x + MID_X + WIDE / 2 + LENTH);
			client_graphic_draw.data.start_y = (offset_y + MID_Y);
			client_graphic_draw.data.end_y  = client_graphic_draw.data.start_y;
			progress++;
		}break;
		case 1://    -  -
		{
			client_graphic_draw.data.operate_tpye = 1;
			client_graphic_draw.data.graphic_tpye = 1;
			client_graphic_draw.data.graphic_name.number= progress;
			client_graphic_draw.data.layer = CROSSHAIR_LAYER;
			client_graphic_draw.data.color = 1;
			client_graphic_draw.data.width = 3;
			client_graphic_draw.data.start_x = (offset_x + MID_X - WIDE / 2);
			client_graphic_draw.data.end_x = (offset_x + MID_X - WIDE / 2 - LENTH);
			client_graphic_draw.data.start_y = (offset_y + MID_Y);
			client_graphic_draw.data.end_y  = client_graphic_draw.data.start_y;
			progress++;
		}break;
		case 2://   - |-
		{
			client_graphic_draw.data.operate_tpye = 1;
			client_graphic_draw.data.graphic_tpye = 1;
			client_graphic_draw.data.layer = CROSSHAIR_LAYER;
			client_graphic_draw.data.graphic_name.number= progress;
			client_graphic_draw.data.color = 1;
			client_graphic_draw.data.width = 3;
			client_graphic_draw.data.start_x = (offset_x + MID_X - WIDE / 2);
			client_graphic_draw.data.end_x = client_graphic_draw.data.start_x;
			client_graphic_draw.data.start_y = (offset_y + MID_Y);
			client_graphic_draw.data.end_y = (offset_y + MID_Y - LENTH);
			progress++;
		}break;
		case 3://	-| |-
		{
			client_graphic_draw.data.operate_tpye = 1;
			client_graphic_draw.data.graphic_tpye = 1;
			client_graphic_draw.data.layer = CROSSHAIR_LAYER;
			client_graphic_draw.data.graphic_name.number= progress;
			client_graphic_draw.data.color = 1;
			client_graphic_draw.data.width = 3;
			client_graphic_draw.data.start_x = (offset_x + MID_X + WIDE / 2);
			client_graphic_draw.data.end_x = client_graphic_draw.data.start_x;
			client_graphic_draw.data.start_y = (offset_y + MID_Y);
			client_graphic_draw.data.end_y = (offset_y + MID_Y - LENTH);
			progress++;
		}break;
		default:
		{
			
		}break;
	}
		
	judgement_send(0x100);
}



u8 engineer_pointing(int16_t offset_x, int16_t offset_y, u8 WIDE, u16 radius)//���ԣ�Բ�����½�Ϊ��0��0��,��(x)����(y)Ϊ����
{
			client_graphic_draw.data.operate_tpye = 1;
			client_graphic_draw.data.graphic_tpye = 3;
			client_graphic_draw.data.graphic_name.number = radius;
			client_graphic_draw.data.layer = CROSSHAIR_LAYER;
			client_graphic_draw.data.color = 1;
			client_graphic_draw.data.width = WIDE;
			client_graphic_draw.data.start_x = (offset_x + MID_X);
			client_graphic_draw.data.start_y = (offset_y + MID_Y);
			client_graphic_draw.data.radius = radius;
			client_graphic_draw.data.start_angle = -180;
			client_graphic_draw.data.end_angle = 180;
	judgement_send(0x100);
	return 0;
}


void unpointable_test(uint16_t y)//�ɻ淶Χ����1��ֱ��
{
	static uint16_t turn;
	if(turn * 25 > 1080)
		turn = 0;
	client_graphic_draw.data.operate_tpye = 1;
	client_graphic_draw.data.graphic_tpye = 1;
	client_graphic_draw.data.graphic_name.number = turn+1;
	client_graphic_draw.data.layer = CROSSHAIR_LAYER;
	client_graphic_draw.data.color = 1;
	client_graphic_draw.data.width = 50;
	client_graphic_draw.data.start_x = (0);
	client_graphic_draw.data.end_x = (1920);
	client_graphic_draw.data.start_y = (turn*25);
	client_graphic_draw.data.end_y  = client_graphic_draw.data.start_y;
	turn++;
	judgement_send(0x100);
}


void back_camera_tips(int8_t tips_dir)//1:��:-1��:0�м� ����:���
{

	if(tips_dir ==1)//��
	{
		client_graphic_draw.data.operate_tpye = 1;
		client_graphic_draw.data.graphic_tpye = 5;
		client_graphic_draw.data.graphic_name.number = 1;
		client_graphic_draw.data.layer = BACK_CAMERA_LAYER;
		client_graphic_draw.data.color = 3;
		client_graphic_draw.data.width = 20;
		client_graphic_draw.data.start_x = (MID_X);
		client_graphic_draw.data.start_y = (MID_Y);
		client_graphic_draw.data.end_x = (100);
		client_graphic_draw.data.end_y = (100);
		client_graphic_draw.data.start_angle = -135;
		client_graphic_draw.data.end_angle = -45;
	}
	else if(tips_dir == -1)//��
	{
		client_graphic_draw.data.operate_tpye = 1;
		client_graphic_draw.data.graphic_tpye = 5;
		client_graphic_draw.data.graphic_name.number = 2;
		client_graphic_draw.data.layer = BACK_CAMERA_LAYER;
		client_graphic_draw.data.color = 3;
		client_graphic_draw.data.width = 20;
		client_graphic_draw.data.start_x = (MID_X);
		client_graphic_draw.data.start_y = (MID_Y);
		client_graphic_draw.data.end_x = (100);
		client_graphic_draw.data.end_y = (100);
		client_graphic_draw.data.start_angle = 45;
		client_graphic_draw.data.end_angle = 135;
	}
	else if(tips_dir == 0)//��
	{
		client_graphic_draw.data.operate_tpye = 1;
		client_graphic_draw.data.graphic_tpye = 5;
		client_graphic_draw.data.graphic_name.number = 3;
		client_graphic_draw.data.layer = BACK_CAMERA_LAYER;
		client_graphic_draw.data.color = 3;
		client_graphic_draw.data.width = 20;
		client_graphic_draw.data.start_x = (MID_X);
		client_graphic_draw.data.start_y = (MID_Y);
		client_graphic_draw.data.end_x = (100);
		client_graphic_draw.data.end_y = (100);
		client_graphic_draw.data.start_angle = 135;
		client_graphic_draw.data.end_angle = -135;
	}
	else //���
	{
		client_graphic_draw.data.operate_tpye = 5;
		client_graphic_draw.data.layer = BACK_CAMERA_LAYER;
	}
	judgement_send(0x100);
}
