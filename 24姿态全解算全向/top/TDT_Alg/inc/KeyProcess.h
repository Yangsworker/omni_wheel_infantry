/**
 * @file KeyProcess.h
 * @author 梁文生
 * @brief DBUS按键处理
 * @version 0.1
 * @date 2021-03-22
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __KEY_PROCESS_H
#define __KEY_PROCESS_H

#include "board.h"
#include "dbus.h"
#include <stddef.h>
#include "cycle.h"

/**
 * @addtogroup TDT_KEY_PROCESS
 * @{
 */

//通用双击检测
typedef struct _doubleKickJudge{
public:
	_doubleKickJudge(float kickTime_TH = 0.25f): doublekickJudgeThreshold(kickTime_TH){};
	
	//无键值返回0，单击返回1，双击返回2
	uint8_t doubleKickVal(uint8_t keyVal)
	{
		if(!keyVal)//无键值时返回0
			thisKeyVal = 0;
		if(keyVal>lastKeyVal)//键值下降沿检测
		{
			downtimes++;
			lastKickTimeStamp = thisKickTimeStamp; //记录上次时间戳
			thisKickTimeStamp = getSysTimeUs() / 1e6f; //获得新的时间戳
			//若两次时间戳相差在阈值内,返回2[双击]，否则返回1[单击]
			lastfromtime = timeIntervalFrom_f(lastKickTimeStamp);
			thisKeyVal = 1 + (timeIntervalFrom_f(lastKickTimeStamp) < doublekickJudgeThreshold);
		}
		lastKeyVal = keyVal;
		return thisKeyVal;
	};
private:
	float lastfromtime;
	uint16_t downtimes;
	float doublekickJudgeThreshold;//双击阈值(s),默认1/4秒
	float thisKickTimeStamp;//最新一次点击时间戳(s)
	float lastKickTimeStamp;//上一次点击时间戳(s)
	uint8_t lastKeyVal;
	uint8_t thisKeyVal;
}doubleKickJudge;
/**
 * @copydoc TDT_KEY_PROCESS
 */
class KeyProcess
{
public:
	/**
	 * @brief Key Process构造器
	 * @param keyValue 					键值，不允许重复
	 * @param keyProcessCtrlToDisable 	仅不包含ctrl的键值有效
	 * @param keyProcessShiftToDisable 	仅不包含ctrl的键值有效
	 * @param press 					按下回调函数（按下了多长时间(毫秒））
	 * @param release 					释放回调函数（按下了多长时间(毫秒））
	 * @param hold 						按住回调函数（按下了多长时间(毫秒））
	 */
	KeyProcess(uint16_t keyValue, void (*press)(uint32_t *interval) = 0, void (*release)(uint32_t *interval) = 0,
			   void (*hold)(uint32_t *interval) = 0, u8 keyProcessCtrlToDisable = 0, u8 keyProcessShiftToDisable = 0);

	/**
	 * @brief 根据键值获取对象
	 * @param keyValue 键值
	 * @return KeyProcess* 
	 */
	KeyProcess *getKeyProcess(uint16_t keyValue);

	/**
	 * @brief 键值循环处理函数
	 * 
	 * @param keyValue 键值
	 */
	static void keyHandle(uint16_t keyValue);

	/**
	 * @brief 获取当前按键对象是否允许被触发
	 * @return u8 0 不允许; 1 允许
	 */
	inline u8 getEnable() { return _enable; };
	/**
	 * @brief 设置当前按键对象是否允许被触发
	 * @param enable u8 0 不允许; 1 允许
	 */
	void setEnable(u8 enable);
	/**
	 * @brief 允许当前按键对象被触发
	 */
	inline void enable() { this->_enable = 1; };
	/**
	 * @brief 不允许当前按键对象被触发
	 */
	void disable();

	/**
	 * @brief 设置按下时的回调函数
	 * @param press 按下时的回调函数
	 */
	inline void setPressCallback(void (*press)(uint32_t *interval)) { this->press = press; }
	/**
	 * @brief 设置松开时的回调函数
	 * @param release 松开时的回调函数
	 */
	inline void setReleaseCallback(void (*release)(uint32_t *interval)) { this->release = release; }
	/**
	 * @brief 设置按住时的回调函数
	 * @param hold 按住时的回调函数
	 */
	inline void setHoldCallback(void (*hold)(uint32_t *interval)) { this->hold = hold; }

private:
	static Cycle keyCycle; ///<按键定时器

	/**
	 * @brief 键值遍历
	 * @param[in] keyValue 键值
	 * @param[in] indexFrom 从第几个索引
	 * @param[in] indexTo 到第几个索引
	 * @param[in] interval 距离上一次触发的时间 毫秒
	 */
	static void keyTravel(uint16_t keyValue, uint16_t indexFrom, uint16_t indexTo, uint32_t interval);

	KeyProcess();

	u8 _enable : 1;					 ///<是否失能
	u8 hasPress : 1;				 ///<是否已经按下
	u8 keyProcessCtrlToDisable : 1;	 ///<按下ctrl时自动失能该按键
	u8 keyProcessShiftToDisable : 1; ///<按下ctrl时自动失能该按键
	uint32_t interval;				 ///<按下持续时间
	/**
	 * @brief 包括判断函数指针的函数调用
	 */
	inline void preHold()
	{
		if (hold)
			hold(&interval);
	};
	/**
	 * @brief 包括判断函数指针的函数调用
	 */
	inline void preRelease()
	{
		if (release)
			release(&interval);
		interval = 0;
	};
	/**
	 * @brief 包括判断函数指针的函数调用
	 */
	inline void prePress()
	{
		if (press)
			press(&interval);
		interval = 0;
	};

	void (*hold)(uint32_t *interval); ///<按住时的回调函数

	void (*release)(uint32_t *interval); ///<松开时的回调函数

	void (*press)(uint32_t *interval); ///<按下时的回调函数
};

/** @} */

#endif
