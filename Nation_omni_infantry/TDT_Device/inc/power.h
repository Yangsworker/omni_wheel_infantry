#ifndef _POWER_
#define _POWER_

#include "board.h"
#include "can.h"

class Power
{
public:
	float powerLimitKp;					//功率限制系数
	float underVoltageProtectKp = 1;	//欠压保护系数
	float overFlowKp = 1;				//浮动功率系数

	union{	//0X110:裁判系统功率相关数据
		u8 txData_110[8];
		struct {
			float chassisPower;				//底盘功率
			uint16_t chassisPowerBuffer;	//底盘缓冲能量
			uint16_t max_chassis_power;		//最大功率		
		};
	};
	union{	//0X120:功率状态数据
		u8 txData_120[8];
		struct {
			uint8_t useCap:1;			//使用电容[路径切换]
			uint8_t Check_Mode:1;		//检录模式
			uint8_t ULTS_Mode:1;		//用了他死模式	
		};	
	};
	union{	//0X130:功率模块反馈数据
		u8 rxData_130[8];
		struct {
			uint16_t capacitance_percentage_t;	//电容能量百分比[仅通讯用，模拟定点数]
			uint16_t maxCurrent_t;				//最大电流[仅通讯用，模拟定点数]
			uint16_t currentNow_t;				//当前电流[仅通讯用，模拟定点数]
			uint16_t capHealth:1;				//当前电容电压过低，处于直通状态
			uint16_t errorCode:3;				//功率错码
			uint16_t reserved:12;
		};	
	};
	float capacitance_percentage;	//电容百分比
	float currentNow;				//当前电流
	float maxCurrent;				//最大电流

	uint8_t powerDownLimit;			//功率控制下限
	uint8_t powerLimitRange;		//功率控制范围
	
	uint8_t speedUpLevel;	//加速等级 0:常态|1:普通加速|2:二级加速|3:飞坡加速
	
	bool usingBackup;				//"尝试"使用备用功率[主动开启，自动关闭]
	bool useTooMuchCapFlag;			//电容使用超限标志位
	bool flySloping;				//正在飞坡

	enum {
		offline,onCAN1,onCAN2
	}whereIsPower;//功率模块挂载情况检测
	void myPowerCanRx(CanRxMsg CanRxMsg);
	void myPowerCanTx(void);			//功率CAN信息发送
	
	void myPowerOfflineJudge(void);		//功率模块离线检测
	void getPowerKp(void);	//根据加速等级得到功率限制系数
	
private:
	u8 powerOfflineCnt;
	void getPowerLimit(void);			//根据加速等级选择功率限制值
	void powerOverFlowCal(void);		//浮动功率计算
	void underVoltageProtection(void);	//欠压保护系数计算
	void setPowerLimit(float downLimit,float upLimit = 0,uint8_t range = 10);
};

extern Power power;


//双击检测
typedef struct _doubleKickJudge{
public:
	//_doubleKickJudge(float doublekickJudgeThreshold = 0.5f){}
	
	//无键值返回0，单击返回1，双击返回2
	uint8_t doubleKickVal(uint8_t keyVal)
	{
		doublekickJudgeThreshold = 0.3f; //双击检测时间间隔
		if(!keyVal)//无键值时返回0
			thisKeyVal = 0;
		if(keyVal>lastKeyVal)//键值下降沿检测
		{
			lastKickTimeStamp = thisKickTimeStamp; //记录上次时间戳
			thisKickTimeStamp = getSysTimeUs() / 1e6f; //获得新的时间戳
			//若两次时间戳相差在阈值内,返回2[双击]，否则返回1[单击]
			thisKeyVal = 1 + (timeIntervalFrom_f(lastKickTimeStamp) < doublekickJudgeThreshold);
		}
		lastKeyVal = keyVal;
		return thisKeyVal;
	};
private:
	float doublekickJudgeThreshold;//双击阈值(s),默认0.3秒
	float thisKickTimeStamp;//最新一次点击时间戳(s)
	float lastKickTimeStamp;//上一次点击时间戳(s)
	uint8_t lastKeyVal;
	uint8_t thisKeyVal;
}doubleKickJudge;


#endif
