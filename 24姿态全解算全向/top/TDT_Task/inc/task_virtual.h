#ifndef __TASK_VIRTUAL_H
#define __TASK_VIRTUAL_H

enum TaskStatus
{
    taskStateRun = 0,
    taskStateStop = 1,
};

class VirtualTask
{
public:
	TaskStatus status;
	//构造函数
	VirtualTask();

	//初始化
    virtual void init() = 0;
    //主函数
    virtual void run() = 0;
	
	//可重写，遥控数据更新时触发
	virtual void remoteCtrlUpdate(){};
	//脱力
	virtual void deforceCallBack(){};
	//取消脱力
	virtual void deforceCancelCallBack(){};

	//任务列表
	static VirtualTask **taskList;
	//任务数量
	static int taskNum;
};

#endif
