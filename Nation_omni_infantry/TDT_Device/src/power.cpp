#include "power.h"
#include "judgement.h"
#include "my_math.h"


Power power;
//功率控制
//得到功率限幅系数"powerLimitKp"
void Power::getPowerKp(void)
{
	useTooMuchCapFlag 	 = powerDownLimit >= capacitance_percentage;
	useCap *= capHealth;//电容电压过低自动切直通/*放在这里认真的吗-_-|||*/
	
	//*******功率未离线，电容健康，使用电容或缓冲能量>40时->常规限制模式********//
    if (whereIsPower!=offline && capHealth && (useCap || chassisPowerBuffer >= 40))
    {
		getPowerLimit();//得到功率限制范围
		powerLimitKp = (capacitance_percentage - powerDownLimit) / powerLimitRange;//通过电容量给出限制系数
		powerLimitKp*= 1.0;//防电容使用超调的限制系数
		powerLimitKp = LIMIT(powerLimitKp, 0.05, 1.0f); //系数限幅，低保系数，保证底盘不至于跑不了
    }
	//******************功率异常，裁判系统正常->软硬件共同限制******************//
		else if(!can1Feedback.jgmtOffline)
	{
		powerOverFlowCal();	//超限系数计算
		powerLimitKp = (chassisPowerBuffer - 10.0f) / 50.0f;//通过缓冲能量给出限制系数[期望限制在10J~60J](60J/250J)
		powerLimitKp*= overFlowKp;//使用超限系数，增大限制强度
		powerLimitKp = LIMIT(powerLimitKp, 0, 0.4f);
	}
	//*****************功率异常，裁判系统离线->采用本地限制系数*****************//
	else 						 
		powerLimitKp = 0.1; 
	
	underVoltageProtection();//欠压保护
}

//超限系数计算
//功率异常，裁判系统正常，软硬件混合限功率时用
//输出overFlowKp
void Power::powerOverFlowCal(void)
{
	static u8 ctrlCnt;

	ctrlCnt = ctrlCnt%100;//差频控制100:1

	if(!ctrlCnt++ && can1Feedback.remainPowerBuffer <= 0)//缓冲能量使用到限制以下，增大限制强度
	{
		overFlowKp -= 0.03f;
		overFlowKp = LIMIT(overFlowKp,0.7f,1.0f);
	}
}

//欠压保护系数计算
//电压过低导致电调重启
//计算欠压保护系数underVoltageProtectKp并修正powerLimitKp
void Power::underVoltageProtection(void)
{
	if(whereIsPower == offline||!currentNow)//功率离线或当前电流为0时不修正
		return;
	underVoltageProtectKp = LIMIT(maxCurrent/currentNow,0,1.0f);
	powerLimitKp *= underVoltageProtectKp;
	powerLimitKp *= underVoltageProtectKp;
}


//电容限制范围设定
//不填写上限时由范围和下限决定
//范围默认为10,填0时不给范围赋值
void Power::setPowerLimit(float downLimit,float upLimit,uint8_t range)
{
	powerDownLimit = downLimit;
	if(upLimit)
		powerLimitRange = upLimit-downLimit;
	else if(range)
		powerLimitRange = range;
}

//得到功率限制范围
//飞坡模式拥有最高优先级，开启后固定降至30%~50%
//备用功率仅允许在电容低于55%时或使用加速时开启，开启后固定降至30%~50%，强制使用会释放3%的电容
//使用二级加速后及一次性释放常规电容量，开启后固定降至50%~60%
//使用加速后释放10%的电容，但不会低于50%
//在功率计算中被调用
void Power::getPowerLimit(void)
{
	static u8 lastSpeedUpLevel;

	if(speedUpLevel == 3)//三级加速[飞坡]
	{
		setPowerLimit(30,50);
		goto GOT_THE_LIMIT;
	}
	else if(speedUpLevel)//一、二级加速
	{
		//加速中使用备用功率
		if(usingBackup)
		{
			setPowerLimit(30,50);
			goto GOT_THE_LIMIT;
		}
		//二级加速时
		else if(speedUpLevel == 2)
		{
			setPowerLimit(50,60);
			goto GOT_THE_LIMIT;
		}
		//单次加速，释放50%以上的10%的能量 //改为释放20%的电容
		else if(!lastSpeedUpLevel)
		{
			setPowerLimit(MAX(50,capacitance_percentage-10));
			goto GOT_THE_LIMIT;
		}
	}
	else//不处于加速中
	{
		if(lastSpeedUpLevel == 1)//从加速中退出时，释放3%的能量[50%-90%]
			setPowerLimit(LIMIT(capacitance_percentage-3,50,90),0,0);
		
		if(usingBackup)
		{
			//电容低于55%时使用备用功率，控制在[30%-50%]
			if(capacitance_percentage<55)
			{
				setPowerLimit(30,50);
				goto GOT_THE_LIMIT;
			}	
			else
				usingBackup = 0;//电容>55%时，不许使用备用功率
		}
		/*电容限制恢复*/
		//电容>93%时控制在[90%-100%]
		if(capacitance_percentage >= 93) 
		{
			setPowerLimit(90,100);
			goto GOT_THE_LIMIT;
		}
		else
		{
			//下限自动恢复至当前电容值，上限上推10%
			setPowerLimit(LIMIT(capacitance_percentage - 10,powerDownLimit, 90));
			goto GOT_THE_LIMIT;
		}
	}
GOT_THE_LIMIT:	
	lastSpeedUpLevel = speedUpLevel;
}

//功率模块离线检测
//功率模块发送:500Hz,2ms
void Power::myPowerOfflineJudge(void)
{
	whereIsPower = onCAN1;
	//离线判定阈值:250ms
	if(powerOfflineCnt<250)
		powerOfflineCnt++;
	else
		whereIsPower = offline;
}

//功率CAN信息发送(1kHz)
//两个包轮流发
//离线默认发CAN1
void Power::myPowerCanTx(void)
{
	static u8 txCnt;
	
	txCnt = !txCnt;
	
	//单条信息500Hz一发,若裁判系统离线,满帧发送功率状态
	if(!can1Feedback.jgmtOffline && txCnt)  //裁判系统离线不处理,不发送,由功率模块处理
	{ 
		chassisPower  =  can1Feedback.Jgmt_RealPower;
		chassisPowerBuffer = LIMIT(can1Feedback.remainPowerBuffer, 0, 300);   //底盘缓冲能量,限幅，防止充不进去电?
		max_chassis_power = LIMIT(can1Feedback.chassisPowerLimit, 50, 100);  //最大功率，限幅，防止裁判系统乱发数导致通道超载
		  
		canTx(txData_110,(whereIsPower == onCAN2)?CAN2:CAN1,0X110);//裁判系统相关
	}
	else
		canTx(txData_120,(whereIsPower == onCAN2)?CAN2:CAN1,0X120);//功率状态
}

//功率CAN信息接收
//放在CAN接收中断
void Power::myPowerCanRx(CanRxMsg canRxMsg)
{
	memcpy(rxData_130,canRxMsg.Data,8);
	
	//数据转换
	capacitance_percentage = capacitance_percentage_t/300.0f;
	maxCurrent = maxCurrent_t/2000.0f;
	currentNow = currentNow_t/2000.0f;
	
	powerOfflineCnt = 0;	//清空离线计数器
}
