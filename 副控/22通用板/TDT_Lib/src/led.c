#include "led.h"
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;				//–¬Õ®”√∞ÂPC13
    RCC_APB2PeriphClockCmd(RCC_LED, ENABLE);
    GPIO_InitStructure.GPIO_Pin = LED_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);
    LED_OFF;
	
}
