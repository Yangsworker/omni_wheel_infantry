#ifndef _FIRE_
#define _FIRE_

#include "board.h"
#include "motor.h"
#include "can.h"
#include "chassis_task.h"
#include "ptz_task.h"
#include "vision.h"

enum FireID_t     //2个摩擦轮M3508 拨盘 弹舱
{
	fire1, fire2, DIAL
};
enum //（未用到）
	{
		SILENCE,	//禁止发射(静默)
		CLEAN_AMMO, //清弹 
		FAST_SHOOT, //高频
		MID_SHOOT,	//正常
		SLOW_SHOOT, //低频
		BUFF_SHOOT	//打符
	} fireMode;							//开火状态枚举



class Fire
{
  public:
		Fire(u16 fire1ID, u16 fire2ID, u16 DialID);
	
	Motor *Firemotor[2]; //2*3508摩擦轮
	Motor *Dial;    //拨盘
	u8 Robot_Fire_Mode = 0;//机器人发射机构选择模式 0-初始设置 1-爆发 2-冷却 3-弹速
	bool isjudgeover = 0;
	//摩擦轮程序
  double friction_spd1 , friction_spd2; //摩擦轮设定速度
	u8 allowshoot = 0;        //手动发射限制
	u8 allowshoot_vision = 0; //视觉发射限制 
	void friction_ctrl();//摩擦轮控制
	//拨盘程序	
	float Dialpos_set;      //拨盘位置设定值
	float Dialpos_record;   //拨盘现在位置
	float Dialpos_initial;  //拨盘初始位置
	float Dialencouder_record; //拨盘现在机械角度值记录
	bool  Dialpos_change;      

	uint8_t Fire_Mode = 0;       //开火模式（0-单发/1-10连发/2-20连发） 通过F键控制改变，默认单发
	bool unLimitedFired = 0;     //解除热量限制
	
	uint8_t last_key;    //上弹（拨盘）模式切换
	uint8_t last_key_vision; //上弹（视觉）
	uint8_t last_fire;   //记录上次发射
	int DialFireTime;    //拨盘转动速度
	int LastDialFireTime;
	int FireTimes;       //发射时间间隔（决定拨盘转动频率）
	int CheckTimes;      //堵转检测时间
	float current_now;   //实时电流值
	void fireload(void);     //拨盘装弹(堵转检测)
	
//以下为弹速、热量限制 （大部分好像也没用到）
	struct
	{
		uint16_t sprocketShootNum; //拨盘(本地)判定的发射数量
		uint16_t frictionShootNum; //摩擦轮判定的发射数量
		uint16_t jgmtShootNum;	   //裁判系统判定的发射数量
	}thisShootNum,lastShootNum;
	Cycle sprocketHeatTimer;
	void sprocketHeatCalc();	//拨弹盘热量计算
	Cycle frictionHeatTimer;
	void frictionHeatCalc();	//摩擦轮热量计算
	void jgmtHeatCalc();		//裁判系统热量计算
	void heatCalc();			//热量计算
	float sprocketHeat;		//拨弹轮判定的本地热量
	float frictionHeat;		//摩擦轮判定的本地热量
	float jgmtHeat;			//裁判系统热量
	float finalHeat;		//三者结合的最终热量
	float finalHeatAdditionSet[3];
	float finalHeatAddition;
	u8 heatChoose;			//热量选择
	int16_t jgmtHeatMax;		//裁判系统热量最大值
	uint16_t jgmtCoolDown;		//裁判系统冷却
	u8 getSpeedIndex();
	void frictionSpdSwitch();	 //发射速度切换
	void frictionSpdLimit();	 //发射速度限制
	float jgmtBulletSpeed;		 //当前子弹速度
	uint16_t jgmtBulletSpeedMax; //裁判系统射速上限
	uint16_t last_SpeedMax;      //记录裁判系统值判断其是否变化
	float lastJgmtBulletSpeed;
	
	float notFireTimer;
	Cycle notFireTimerCycle;
	//以上
	void getFireLimit();        //获得开火限制/获得发射模式
	void fireSpeedSwitch();     //开火速度选择
  void ModeJudge_Fire(void);  //模式选择（主程序）
};

void Fire_Task(void);

extern Fire fire;
#endif