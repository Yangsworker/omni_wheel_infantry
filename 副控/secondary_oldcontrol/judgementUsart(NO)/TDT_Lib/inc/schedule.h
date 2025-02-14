#ifndef _SCHEDULE_H
#define _SCHEDULE_H

#include "board.h"
typedef struct _schedule 
{
    uint16_t   cnt_1ms;
    uint16_t   cnt_2ms;
    uint16_t   cnt_5ms;
    uint16_t   cnt_10ms;
    uint16_t   cnt_20ms;
    uint16_t   cnt_50ms;
    uint16_t   cnt_100ms;
    uint64_t   sysTime_ms;
    uint64_t   beatStart_ms;
} _schedule;

void Loop_1000Hz(void); //1msִ��һ��
void Loop_500Hz(void);	//2msִ��һ��
void Loop_200Hz(void);	//5msִ��һ��
void Loop_100Hz(void);	//10msִ��һ��
void Loop_50Hz(void);	  //20msִ��һ��
void Loop_20Hz(void);	  //50msִ��һ��
void Loop_10Hz(void);	  //100msִ��һ��

void Loop(void);

extern struct _schedule schedule;
extern int8_t aim_point_cnt;
extern int8_t find_sub_obj;
extern float MidKp;
extern float LowKp;
#endif

