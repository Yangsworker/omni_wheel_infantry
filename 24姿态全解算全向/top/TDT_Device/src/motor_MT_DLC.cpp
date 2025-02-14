#include "motor_MT_DLC.h"

MotorList MT_motorlist[2][32];
MT_Can MT_can1;
MT_Can MT_can2;
u8 MT_IMcnt;

//���������ʼ������
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

//���ڵ������ֵ�Ļ�ȡ	�Ž�schedule��	500Hz
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

//�������(����)
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

//setZeroValue		����

/**
  * @brief MT����ϵ��ʼ��
  * @param[in] can_x CAN1/CAN2
  * @param[in] _CanRxMsg Canԭʼ���ݵ�ַ
  */
void MT_Can::Motor_Offset(u8 can_x, CanRxMsg *_CanRxMsg)
{
	//��ȡ���������ַ
	Info = &(MT_motorlist[can_x][_CanRxMsg->StdId - 0x241].motorPoint->canInfo);
	
	//��ʼ�Ƕ�
	Info->encoder = (int16_t)((_CanRxMsg->Data[7] << 8) | (_CanRxMsg->Data[6])); //��е�Ƕ�
	Info->lastEncoder = Info->encoder;											 //����ϵ�Ȧ����������
	Info->totalRound = 0;														 //����Ƕȹ���ʱ��
	Info->totalAngle = 0;
	Info->totalEncoder_SI = 0;
	Info->totalEncoder = 0;
}

//can������Ϣ����	�Ž�can�����ж���
void MT_Can::Motor_Information_Calculate(u8 can_x, CanRxMsg *_CanRxMsg)
{
	//�Ǵ󽮱�׼�����ֱ���˳�
	if (IsDJIMotorCheck(*_CanRxMsg) == 1)
	{
		return;
	}
	
	//�����can�ڵĵ��û��ʹ�ܣ���error���˳�
	if (MT_motorlist[can_x][_CanRxMsg->StdId - 0x241].enableFlag == 0)
	{
		return;
	}
	if (&(MT_motorlist[can_x][_CanRxMsg->StdId - 0x241].motorPoint->canInfo) != 0)
	{
		//��ȡ���������ַ
		Info = &(MT_motorlist[can_x][_CanRxMsg->StdId - 0x241].motorPoint->canInfo);
	}

	//��ʼ��У׼
	if (++Info->msgCnt < 3)
	{
		Motor_Offset(can_x, _CanRxMsg);
		return;
	}

	Info->msgCnt = 100;

	/*��ȡ���ת�ټ���е�Ƕ�*/
	if(_CanRxMsg->Data[0] == 0x90)
	{
		Info->encoder = (uint16_t)((_CanRxMsg->Data[5] << 8) | (_CanRxMsg->Data[4]));	//λ��
		Info->encoderCalibration = Info->encoder;
		/*��������ת��Ȧ��*/
		if (Info->encoderCalibration - Info->lastEncoderCalibration > 8192)
		{
			Info->totalRound--;
		}
		else if (Info->encoderCalibration - Info->lastEncoderCalibration < -8192)
		{
			Info->totalRound++;
		}
		/*��������ת�ܻ�е�Ƕ�ֵ*/
		Info->totalEncoder = (Info->totalRound * 16384 + Info->encoderCalibration);
		/*��������ת�ܽǶ�*/
		Info->totalAngle = Info->totalEncoder * 0.02197265625f;	 //(ǿ��ת��������)
		Info->totalAngle_f = Info->totalEncoder * 0.02197265625f; //(������)
		/*��¼�˴λ�е�Ƕ�*/
		Info->lastEncoder = Info->encoder;
		Info->lastTotalEncoder = Info->totalEncoder;
		Info->lastEncoderCalibration = Info->encoderCalibration;
	}
	else
	{
		Info->speed = (int16_t)((_CanRxMsg->Data[5] << 8) | (_CanRxMsg->Data[4]));		 //�ٶ�
		Info->trueCurrent = (int16_t)((_CanRxMsg->Data[3] << 8) | (_CanRxMsg->Data[2])); //ʵ�ʵ���
		Info->temperature = (int16_t)(_CanRxMsg->Data[1]);								 //�¶�
	}
	

	/*������̨�����ת�ٶ�=��е�Ƕ�ֵ֮����RPM����*/
	Info->dps = Info->speed * 6.0f; //(������)

	Info->totalEncoder_SI += Info->speed;
	//�ٶȻ��ֺ�λ�÷����������˲�������

	/*������߼�ⲿ��*/
	Info->lostCnt = 0;	//��ռ�����
	Info->lostFlag = 0; //�������
}


/**
  * @brief �󽮵�����
  * @param[in] _CanRxMsg ͨ���ж�ID��ȷ���ǲ��Ǵ󽮵��
  * @return 1:�ǣ�0������
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

