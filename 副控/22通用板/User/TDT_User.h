#include "stdint.h"
#ifndef __TDT_INFANTRY__
#define __TDT_INFANTRY__

#define Run_Mode 1		
#define Abandon !Run_Mode		/**不调用的部分不编译**/

#define FB_MAXSPEED		8000.0f/660.0f	  /**< 底盘前后的最大速度*/
#define LR_MAXSPEED		8000.0f/660.0f	  /**< 底盘左右的最大速度*/
#define ROT_MAXASPEED	4500.0f/660.f	  	/**< 底盘旋转的最大角速度，度每秒*/

#define FB_MINSPEED		7800.0f/6600.0f	  /**< 底盘前后的最小速度*/
#define LR_MINSPEED		7800.0f/6600.0f	  /**< 底盘左右的最小速度*/
#define ROT_MINASPEED	360.0f/3300.0f*10	  	/**< 底盘旋转的最小角速度，度每秒*/
/***************I2C GPIO定义******************/
#define RCC_I2C1	     RCC_AHB1Periph_GPIOC
#define I2C1_PORT      GPIOC
#define I2C1_Pin_SCL   GPIO_Pin_2
#define I2C1_Pin_SDA   GPIO_Pin_1

#define RCC_I2C2_SCL	     RCC_AHB1Periph_GPIOA
#define RCC_I2C2_SDA	     RCC_AHB1Periph_GPIOA
#define I2C2_PORT_SCL      GPIOA
#define I2C2_Pin_SCL  		 GPIO_Pin_1
#define I2C2_PORT_SDA      GPIOA
#define I2C2_Pin_SDA  		 GPIO_Pin_0


#define WHEELNUM	  6		  /**< 底盘轮子数目*/
#define MAXSET3510  13500  /**< 3510电机的最大给定值*/
#define MAXSET3508  8000
#define MAXSET6623  5000  /**< 6623电机的最大给定值*/
#define MAXSETDM50  850  /**< DM50电机的最大给定值*/
#define Belt_Speed  2500  /**< 同步带2006电机给定值*/

#define KEY_B			0x8000
#define KEY_V			0x4000
#define KEY_C			0x2000
#define KEY_X			0x1000
#define KEY_Z			0x0800
#define KEY_G			0x0400
#define KEY_F			0x0200
#define KEY_R			0x0100
#define KEY_E			0x0080
#define KEY_Q			0x0040
#define KEY_CTRL	0x0020
#define KEY_SHIFT	0x0010
#define KEY_D			0x0008
#define KEY_A			0x0004
#define KEY_S			0x0002
#define KEY_W			0x0001



/**
* @struct  _RC
* @brief   定义遥控器结构体
*/


typedef struct _RC{
					int16_t CH[11];
	        int16_t CH_LAST[11];
					int16_t FB_Set;	/***底盘前后***/
					int16_t LR_Set;	/***底盘左右***/
					int16_t RT_Set;	/***底盘旋转(不带跟随)***/
					int16_t SW_L;
					int16_t SW_R;
					struct
					{
						int16_t value;
						uint8_t     B;			
						uint8_t     V;			
						uint8_t     C;			
						uint8_t     X;		
						uint8_t     Z;			
						uint8_t     G;		
						uint8_t     F;			
						uint8_t     R;			
						uint8_t     E;			
						uint8_t     Q;		
						uint8_t     CTRL;	
						uint8_t     SHIFT;	
						uint8_t     D;			
						uint8_t     A;			
						uint8_t     S;			
						uint8_t     W;			
					} key;
					struct
					{
						int16_t value;
						uint8_t     B;			
						uint8_t     V;			
						uint8_t     C;			
						uint8_t     X;		
						uint8_t     Z;			
						uint8_t     G;		
						uint8_t     F;			
						uint8_t     R;			
						uint8_t     E;			
						uint8_t     Q;		
						uint8_t     CTRL;	
						uint8_t     SHIFT;	
						uint8_t     D;			
						uint8_t     A;			
						uint8_t     S;			
						uint8_t     W;			
					} last_key;
					struct
					{
						uint8_t     left_jump;			
						uint8_t     Right_jump;
					} mouse;
					struct
					{
						uint8_t     left_jump;			
						uint8_t     Right_jump;
					} mouse_last;
					struct
					{
						int value;
						int     B;			
						int     V;			
						int     C;			
						int     X;		
						int     Z;			
						int     G;		
						int     F;			
						int     R;			
						int     E;			
						int     Q;		
						int     CTRL;	
						int     SHIFT;	
						int     D;			
						int     A;			
						int     S;			
						int     W;	
						int     left_jump;
						int     Right_jump;	
						
					} flag;
	     }Remote_Control;


/**
* @struct  _vec1f
* @brief 一维float向量结构体
*/
typedef struct _vec1f
{
	float data[1];
}vec1f;
/**
* @struct  _vec1int16
* @brief 一维int16向量结构体
*/
typedef struct _vec1int16
{
	 short data[1];
}vec1int16;




/**
* @struct  _vec2f
* @brief 二维float向量结构体
*/
typedef struct _vec2f
{
	float data[2];
}vec2f;
/**
* @struct  _vec2int16
* @brief 二维int16向量结构体
*/
typedef struct _vec2int16
{
	 short data[2];
}vec2int16;





/**
* @struct  _vec3f
* @brief 三维float向量结构体
*/
typedef struct _vec3f
{
	float data[3];
}vec3f;

/**
* @struct  _vec3int16
* @brief 三维int16向量结构体
*/
typedef struct _vec3int16
{
	 short data[3];
}vec3int16;



/**
* @struct  _vec4f
* @brief 四维float向量结构体
*/
typedef struct _vec4f
{
	float data[4];
}vec4f;
/**
* @struct  _vec4int16
* @brief 四维int16向量结构体
*/
typedef struct _vec4int16
{
	 short data[4];
}vec4int16;





/**
* @struct  _vec6f
* @brief 六维float向量结构体
*/
typedef struct _vec6f
{
	float data[6];
}vec6f;
/**
* @struct  _vec6int16
* @brief 六维int16向量结构体
*/
typedef struct _vec6int16
{
	 short data[6];
}vec6int16;




#endif
