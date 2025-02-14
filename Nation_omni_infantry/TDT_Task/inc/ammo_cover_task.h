#ifndef _BULLET_
#define _BULLET_

#include "board.h"
#include "motor.h"
#include "can.h"
#include "chassis_task.h"
#include "ptz_task.h"

class AmmoCover
{  
public:
//校准相关
	u8 ifstartoffset; //开始校准接口
	u8   count; 
	float outLimitTemp;    
  int   offsetPos;	
	bool  offset_start = 0;
  u8    ifchassismove = 0; //底盘是否移动
  bool Ammo_offset(float reSetSpeed, float maxErr, float outLimit);  //弹舱盖校准函数 
  void Ammo_offset();

	  AmmoCover();
	Motor * Bullet;

  u8 open_close = 0;  //弹舱盖开闭
	u8 coverOpen = 0;		 //弹舱盖是否打开
  void Coveropen_Judge(void); //弹舱盖是否打开判断
	inline u8 getCoverOpen() { return coverOpen; };
//弹舱盖开闭
	float position_set = 0; //弹舱盖开闭位置
	void auto_close(void);   //底盘移动/自动关闭
	
  void Ammo_open();
	
};
extern AmmoCover ammoCover;

void Ammo_Task(void);

#endif