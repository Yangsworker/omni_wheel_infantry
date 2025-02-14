#include "com_task.h"
#include "led_task.h"
#include "multi_imu.h"
#include "judgement.h"
#include "schedule.h"
#include "power.h"
#include "my_math.h"
#include "imu_task.h"


CAN_TypeDef *ThCan = CAN1;//�趨��ͨCAN


/****����->��̨�����ϰ�****/
JgmtAbout JgmtPack;
LocalAbout LocalPack;
/****��̨->���̵����ϰ�****/
CtrlAbout CtrlPack;
DisplayAbout DisplayPack;

/*******************�Զ��崦����*******************/

//��Ϣ�ӹ�ͨCAN����
void Com_Task(_myPack myPack)
{
	if(myPack == Jgmt)
	{
		JgmtPack.jgmtOfflineFlag = judgement.jgmtOffline;

		JgmtPack.shooterHeatRemainA = LIMIT(judgement.gameRobotStatus.shooter_barrel_heat_limit 
									- judgement.powerHeatData.shooterId1_17mmCoolingHeat,0,500); //��ǰAǹʣ����
		
		JgmtPack.bulletSpeed = LIMIT((judgement.shootData.bulletSpeed*100),0,4000); //��ǰ����
		JgmtPack.bulletCount = judgement.shootNum[ID1_17mm_SHOOT_NUM]+judgement.shootNum[ID2_17mm_SHOOT_NUM]; //�����������ṩ���䣬����ֵ���岻��

		JgmtPack.powerlimitKp = power.powerLimitKp* 100;
		JgmtPack.yaw_dps = boardImu->gyro.dps.data[1] * 3.14159f / 180.0f * 500.0f;   //30000
//		/*���Ϸ�������ϢԤ����*/
		canTx(JgmtPack.data,ThCan,0x400);
	}
	else if(myPack == Local)
	{
		LocalPack.myColor = judgement.gameRobotStatus.robotId > 100;

		LocalPack.shooterCoolingLimitA = judgement.gameRobotStatus.shooter_barrel_heat_limit;   //ǹ����������
		LocalPack.shooterCoolingRateA = judgement.gameRobotStatus.shooter_barrel_cooling_value; //ǹ��ÿ����ȴֵ

//		/*���Ϸ�������ϢԤ����*/
		canTx(LocalPack.data,ThCan,0x401);
	}
}


//��ͨCAN��Ϣ����
void CanInfo_Handle(CanRxMsg ThCanMsg)
{
	switch(ThCanMsg.StdId)
	{
		case 0X500:
			topCtrlOfflineCnt = 0;
			memcpy(CtrlPack.data,ThCanMsg.Data,8);
			if(CtrlPack.sysRST)
			{
				__disable_irq(); //�ر������ж�
				NVIC_SystemReset(); //��λ
				while (1)
				{
				} //���ȴ���λ
			}
			break;
		case 0X501:    //����״̬����Ƶ��
			topCtrlOfflineCnt = 0;
			memcpy(DisplayPack.data,ThCanMsg.Data,8);
			break;
		default:
			break;
	}
}