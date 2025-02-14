#ifndef _GY53_H_
#define _GY53_H_
#include "board.h"

void GY53_init(void);
void Distance_Read(void);
void TIM3_Int_Init(u16 arr,u32 psc);
extern float GYdistance[3];
enum GYsenser
{
	LF = 0,LB = 1,FF = 2
};
#endif
