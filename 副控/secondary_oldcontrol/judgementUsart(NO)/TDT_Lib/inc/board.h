#ifndef __BOARD_H__
#define __BOARD_H__
#include "TDT_User.h"
#include "stm32f10x.h"
#include "stm32f10x_spi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "vision.h"
#include "can.h"
#include "led.h"
#include "schedule.h"
#include "crc.h"
#include "myiic.h"
#include "mpu6050.h"
#include "mymath.h"
#include "imu.h"
#include  "mpudata.h"
#include "judgement.h"
#include "filter.h"
#include "gy53.h"

/***************LED GPIO����******************/
#define  RCC_LED				RCC_APB2Periph_GPIOB
#define  LED_PORT				GPIOB
#define  LED_Pin				GPIO_Pin_5


#define  LED_OFF		GPIO_SetBits(LED_PORT,LED_Pin)
#define  LED_ON			GPIO_ResetBits(LED_PORT,LED_Pin)
#define  LED_TOGGLE		LED_PORT->ODR ^= LED_Pin


#define abs( x ) ( (x)>0? (x):(-(x)) )
#define limit( x, min, max ) ( (x) < (min)  ? (min) : ( (x) > (max) ? (max) : (x) ) )
#define deathzoom( x, zoom ) ( abs(x)>zoom? (x):(0) )
#define contain( x, min, max ) ( (x<min)?(x+2*max):( (x>max)?(x-2*max):(x) ) )

#define NOW 0
#define OLD 1	
#define NEW 2

#define GET_TIME_NUM 10
extern volatile uint32_t sysTickUptime;
float Get_Cycle_T(u8);
void Cycle_Time_Init(void);

//λ������,ʵ��51���Ƶ�GPIO���ƹ���
//����ʵ��˼��,�ο�<<CM3Ȩ��ָ��>>������(87ҳ~92ҳ).
//IO�ڲ����궨��
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2))
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr))
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))
//IO�ڵ�ַӳ��
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 

//IO�ڲ���,ֻ�Ե�һ��IO��!
//ȷ��n��ֵС��16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //��� 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //���� 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //��� 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //���� 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //��� 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //���� 

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //��� 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //���� 

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //��� 
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //����

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //��� 
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //����

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //��� 
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //����

void SysTick_Init(void);
void delay_ms(unsigned int t);
void delay_us(unsigned int t);
void TDT_Board_ALL_Init(void);
uint32_t GetSysTime_us(void);







/////**
////* @struct  _vec2f
////* @brief ��άfloat�����ṹ��
////*/
////typedef struct _vec2f
////{
////	float data[2];
////}vec2f;

/////**
////* @struct  _vec3f
////* @brief ��άfloat�����ṹ��
////*/
////typedef struct _vec3f
////{
////	float data[3];
////}vec3f;

/////**
////* @struct  _vec3int16
////* @brief ��άint16�����ṹ��
////*/
////typedef struct _vec3int16
////{
////	 short data[3];
////}vec3int16;

/////**
////* @struct  _vec4f
////* @brief ��άfloat�����ṹ��
////*/
////typedef struct _vec4f
////{
////	float data[4];
////}vec4f;









#endif
