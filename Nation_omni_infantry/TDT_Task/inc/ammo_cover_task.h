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
//У׼���
	u8 ifstartoffset; //��ʼУ׼�ӿ�
	u8   count; 
	float outLimitTemp;    
  int   offsetPos;	
	bool  offset_start = 0;
  u8    ifchassismove = 0; //�����Ƿ��ƶ�
  bool Ammo_offset(float reSetSpeed, float maxErr, float outLimit);  //���ո�У׼���� 
  void Ammo_offset();

	  AmmoCover();
	Motor * Bullet;

  u8 open_close = 0;  //���ոǿ���
	u8 coverOpen = 0;		 //���ո��Ƿ��
  void Coveropen_Judge(void); //���ո��Ƿ���ж�
	inline u8 getCoverOpen() { return coverOpen; };
//���ոǿ���
	float position_set = 0; //���ոǿ���λ��
	void auto_close(void);   //�����ƶ�/�Զ��ر�
	
  void Ammo_open();
	
};
extern AmmoCover ammoCover;

void Ammo_Task(void);

#endif