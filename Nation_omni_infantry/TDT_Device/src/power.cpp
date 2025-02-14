#include "power.h"
#include "judgement.h"
#include "my_math.h"


Power power;
//���ʿ���
//�õ������޷�ϵ��"powerLimitKp"
void Power::getPowerKp(void)
{
	useTooMuchCapFlag 	 = powerDownLimit >= capacitance_percentage;
	useCap *= capHealth;//���ݵ�ѹ�����Զ���ֱͨ/*���������������-_-|||*/
	
	//*******����δ���ߣ����ݽ�����ʹ�õ��ݻ򻺳�����>40ʱ->��������ģʽ********//
    if (whereIsPower!=offline && capHealth && (useCap || chassisPowerBuffer >= 40))
    {
		getPowerLimit();//�õ��������Ʒ�Χ
		powerLimitKp = (capacitance_percentage - powerDownLimit) / powerLimitRange;//ͨ����������������ϵ��
		powerLimitKp*= 1.0;//������ʹ�ó���������ϵ��
		powerLimitKp = LIMIT(powerLimitKp, 0.05, 1.0f); //ϵ���޷����ͱ�ϵ������֤���̲������ܲ���
    }
	//******************�����쳣������ϵͳ����->��Ӳ����ͬ����******************//
		else if(!can1Feedback.jgmtOffline)
	{
		powerOverFlowCal();	//����ϵ������
		powerLimitKp = (chassisPowerBuffer - 10.0f) / 50.0f;//ͨ������������������ϵ��[����������10J~60J](60J/250J)
		powerLimitKp*= overFlowKp;//ʹ�ó���ϵ������������ǿ��
		powerLimitKp = LIMIT(powerLimitKp, 0, 0.4f);
	}
	//*****************�����쳣������ϵͳ����->���ñ�������ϵ��*****************//
	else 						 
		powerLimitKp = 0.1; 
	
	underVoltageProtection();//Ƿѹ����
}

//����ϵ������
//�����쳣������ϵͳ��������Ӳ������޹���ʱ��
//���overFlowKp
void Power::powerOverFlowCal(void)
{
	static u8 ctrlCnt;

	ctrlCnt = ctrlCnt%100;//��Ƶ����100:1

	if(!ctrlCnt++ && can1Feedback.remainPowerBuffer <= 0)//��������ʹ�õ��������£���������ǿ��
	{
		overFlowKp -= 0.03f;
		overFlowKp = LIMIT(overFlowKp,0.7f,1.0f);
	}
}

//Ƿѹ����ϵ������
//��ѹ���͵��µ������
//����Ƿѹ����ϵ��underVoltageProtectKp������powerLimitKp
void Power::underVoltageProtection(void)
{
	if(whereIsPower == offline||!currentNow)//�������߻�ǰ����Ϊ0ʱ������
		return;
	underVoltageProtectKp = LIMIT(maxCurrent/currentNow,0,1.0f);
	powerLimitKp *= underVoltageProtectKp;
	powerLimitKp *= underVoltageProtectKp;
}


//�������Ʒ�Χ�趨
//����д����ʱ�ɷ�Χ�����޾���
//��ΧĬ��Ϊ10,��0ʱ������Χ��ֵ
void Power::setPowerLimit(float downLimit,float upLimit,uint8_t range)
{
	powerDownLimit = downLimit;
	if(upLimit)
		powerLimitRange = upLimit-downLimit;
	else if(range)
		powerLimitRange = range;
}

//�õ��������Ʒ�Χ
//����ģʽӵ��������ȼ���������̶�����30%~50%
//���ù��ʽ������ڵ��ݵ���55%ʱ��ʹ�ü���ʱ������������̶�����30%~50%��ǿ��ʹ�û��ͷ�3%�ĵ���
//ʹ�ö������ٺ�һ�����ͷų����������������̶�����50%~60%
//ʹ�ü��ٺ��ͷ�10%�ĵ��ݣ����������50%
//�ڹ��ʼ����б�����
void Power::getPowerLimit(void)
{
	static u8 lastSpeedUpLevel;

	if(speedUpLevel == 3)//��������[����]
	{
		setPowerLimit(30,50);
		goto GOT_THE_LIMIT;
	}
	else if(speedUpLevel)//һ����������
	{
		//������ʹ�ñ��ù���
		if(usingBackup)
		{
			setPowerLimit(30,50);
			goto GOT_THE_LIMIT;
		}
		//��������ʱ
		else if(speedUpLevel == 2)
		{
			setPowerLimit(50,60);
			goto GOT_THE_LIMIT;
		}
		//���μ��٣��ͷ�50%���ϵ�10%������ //��Ϊ�ͷ�20%�ĵ���
		else if(!lastSpeedUpLevel)
		{
			setPowerLimit(MAX(50,capacitance_percentage-10));
			goto GOT_THE_LIMIT;
		}
	}
	else//�����ڼ�����
	{
		if(lastSpeedUpLevel == 1)//�Ӽ������˳�ʱ���ͷ�3%������[50%-90%]
			setPowerLimit(LIMIT(capacitance_percentage-3,50,90),0,0);
		
		if(usingBackup)
		{
			//���ݵ���55%ʱʹ�ñ��ù��ʣ�������[30%-50%]
			if(capacitance_percentage<55)
			{
				setPowerLimit(30,50);
				goto GOT_THE_LIMIT;
			}	
			else
				usingBackup = 0;//����>55%ʱ������ʹ�ñ��ù���
		}
		/*�������ƻָ�*/
		//����>93%ʱ������[90%-100%]
		if(capacitance_percentage >= 93) 
		{
			setPowerLimit(90,100);
			goto GOT_THE_LIMIT;
		}
		else
		{
			//�����Զ��ָ�����ǰ����ֵ����������10%
			setPowerLimit(LIMIT(capacitance_percentage - 10,powerDownLimit, 90));
			goto GOT_THE_LIMIT;
		}
	}
GOT_THE_LIMIT:	
	lastSpeedUpLevel = speedUpLevel;
}

//����ģ�����߼��
//����ģ�鷢��:500Hz,2ms
void Power::myPowerOfflineJudge(void)
{
	whereIsPower = onCAN1;
	//�����ж���ֵ:250ms
	if(powerOfflineCnt<250)
		powerOfflineCnt++;
	else
		whereIsPower = offline;
}

//����CAN��Ϣ����(1kHz)
//������������
//����Ĭ�Ϸ�CAN1
void Power::myPowerCanTx(void)
{
	static u8 txCnt;
	
	txCnt = !txCnt;
	
	//������Ϣ500Hzһ��,������ϵͳ����,��֡���͹���״̬
	if(!can1Feedback.jgmtOffline && txCnt)  //����ϵͳ���߲�����,������,�ɹ���ģ�鴦��
	{ 
		chassisPower  =  can1Feedback.Jgmt_RealPower;
		chassisPowerBuffer = LIMIT(can1Feedback.remainPowerBuffer, 0, 300);   //���̻�������,�޷�����ֹ�䲻��ȥ��?
		max_chassis_power = LIMIT(can1Feedback.chassisPowerLimit, 50, 100);  //����ʣ��޷�����ֹ����ϵͳ�ҷ�������ͨ������
		  
		canTx(txData_110,(whereIsPower == onCAN2)?CAN2:CAN1,0X110);//����ϵͳ���
	}
	else
		canTx(txData_120,(whereIsPower == onCAN2)?CAN2:CAN1,0X120);//����״̬
}

//����CAN��Ϣ����
//����CAN�����ж�
void Power::myPowerCanRx(CanRxMsg canRxMsg)
{
	memcpy(rxData_130,canRxMsg.Data,8);
	
	//����ת��
	capacitance_percentage = capacitance_percentage_t/300.0f;
	maxCurrent = maxCurrent_t/2000.0f;
	currentNow = currentNow_t/2000.0f;
	
	powerOfflineCnt = 0;	//������߼�����
}
