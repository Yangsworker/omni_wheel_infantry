#ifndef _MY_RC_
#define _MY_RC_

#include "board.h"
#include "KeyProcess.h"



#pragma pack(1)

class MyRC
{
public:
	MyRC();
	void myRC_Update(void);

	enum VisionColor_t{
		BLUE = 0,
		GG = 1,
		RED = 2
	};//视觉颜色0蓝2红1寄

	/*云台相关***************/
	float mySpdSetZ;		//【下行】云台自旋速度设定
	float mySpdSetP;		//云台俯仰速度设定
/**/uint8_t visionMode;		//【下行】视觉模式 0:未开启|1:跟随|2:陀螺|3:反符|4:小符|5:大符
	bool lookBack;			//快速回头
	bool baseShootSW;		//建筑与工程击打开启
	bool visionYawOnly;		//视觉仅锁Y轴模式
	bool longFocusSwitch;	//长焦相机模式
	bool laserOn;			//启用激光
	
	/*底盘相关***************/
	bool if_Ctrl;			    //【下行】是否按Ctrl
	bool defollowFlag;		//【下行】底盘跟随取消
	int16_t mySpdSetY;		//【下行】车体前后速度设定
	int16_t mySpdSetX;		//【下行】车体左右速度设定
	uint8_t AvoidMode;		//【下行】0-normal 1-陀螺模式 2-检录 3-检录
	bool if_Shift;			  //【下行】是否按Shift
	bool SpeedUp_level;   //【下行】加速等级
  bool flySloping;		  //【下行】飞坡模式
  bool useCapacitor = 1;	//【下行】使用电容
  bool Rotation_HL;		  //【下行】高低陀螺
  bool ULTS;				    //【下行】ULTS模式
  bool CheckMode;			  //【下行】检录模式

	/*摩擦轮相关*************/
	bool friRun;			//开摩擦轮

	/*拨盘相关***************/
	bool fireTrig;			//开火扳机
	u8 Firefrequency;			//射频 0:单发|1:10弹频|2:20弹频
	bool AutoFire;			// 自动开火
	bool AutoFire_buff; //打符自动开火

	/*弹仓盖相关*************/
	bool coverCal;			//弹仓盖校准
	bool coverState;		//弹仓盖状态 0:未打开 | 1:已打开
	
	/*系统相关***************/
/**/bool topCtrlRst;		//上主控重启
/**/bool botCtrlRst;		//【下行】下主控重启
/**/bool UIInit;			//【下行】UI初始化
	uint8_t whatWheelCtrl:3;//【下行】正在使用滚轮调节什么 0:底盘高度 1:roll轴 2:大符P 3:大符Y
	bool VTX;
	uint8_t VTXplan_ID;
		
	//可保存参数[存入flash的]
	union{
		uint8_t paramInFlash[8];//扇区内缓存
		struct{
			uint8_t visionColor;	//视觉颜色
			int8_t	BuffPOffset:5;	//大符P轴偏置调节		
			int8_t	BuffYOffset:5;	//大符Y轴偏置调节
			int8_t	VisPOffset:5;	//大符P轴偏置调节		
			int8_t	VisYOffset:5;	//大符Y轴偏置调节
		};
	};	

	void FlashParamInit(void);
	void FlashParamSave(void);
private:
	//双击检测类
	doubleKickJudge* visionKick;//陀螺模式【右键】
	doubleKickJudge* buffKick;	//开符模式【Q】
	doubleKickJudge* avoidKick;	//躲避模式【E】
	doubleKickJudge* powerKick;	//功率模式【G】
	doubleKickJudge* lFocusKick;//长焦模式【X】
	//触发类时间戳
	float lookBackTimeStamp;	//快速回头时间戳
	float ULTSTimeStamp;		//ULTS时间戳
	//其他私有变量
	// int32_t wheelCodeOffset;	//滚轮编码偏置
	
	bool contiSwitch(uint8_t Key);
};

#pragma pack()

extern MyRC myRC;




#endif

