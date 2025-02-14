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
  power.useCap = 1;             //使用电容/默认开启
	power.myPowerOfflineJudge();  //检测是否离线
	power.myPowerCanTx();         //2ms发一次 0x110 0x120 交替发送
	power.getPowerKp();           //获得功率限幅系数
}




