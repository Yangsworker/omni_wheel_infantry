#include "chassis_task.h"
#include "imu_task.h"
#include "imu.h"
#include "power.h"

Chassis chassis;

Chassis::Chassis()
{
	
}

void Chassis::chassis_task()
{
  power.useCap = 1;             //ʹ�õ���/Ĭ�Ͽ���
	power.myPowerOfflineJudge();  //����Ƿ�����
	power.myPowerCanTx();         //2ms��һ�� 0x110 0x120 ���淢��
	power.getPowerKp();           //��ù����޷�ϵ��
}




