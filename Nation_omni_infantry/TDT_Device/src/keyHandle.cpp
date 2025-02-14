
//按键连续切换判定
//可用于进入特殊模式等
bool contiSwitch(uint8_t Key)
{
	static uint8_t switchCnt = 0;//来回切换计数器
	static uint16_t switchCd = 1000;//来回切换计时器[1kf]
	static bool last_Key = 0;
	
	if(Key!=last_Key)//有切换时:计数器累加,计时器重置
	{
		switchCnt++;
		switchCd = 1000;
	}
	switchCd-=!!switchCnt;//计数器不空时:计时器递减
	if(!switchCd)//倒计时归零时:计数器清空，计时器重置
	{
		switchCnt = 0;
		switchCd = 1000;
	}
	last_Key = Key;
	
	return switchCnt>10;//计数器积累足够，返回真
}


//通用双击检测
typedef struct _doubleKickJudge{
public:
	_doubleKickJudge(float doublekickJudgeThreshold = 0.25f){}
	
	//无键值返回0，单击返回1，双击返回2
	uint8_t doubleKickVal(uint8_t keyVal)
	{
		if(!keyVal)//无键值时返回0
			thisKeyVal = 0;
		if(keyVal>lastKeyVal)//键值下降沿检测
		{
			lastKickTimeStamp = thisKickTimeStamp; //记录上次时间戳
			thisKickTimeStamp = getSysTimeUs() / 1e6f; //获得新的时间戳
			//若两次时间戳相差在阈值内,返回2[双击]，否则返回1[单击]
			thisKeyVal = 1 + (timeIntervalFrom_f(lastKickTimeStamp) < doublekickJudgeThreshold);
		}
		lastKeyVal = keyVal;
		return thisKeyVal;
	};
private:
	float doublekickJudgeThreshold;//双击阈值(s),默认1/4秒
	float thisKickTimeStamp;//最新一次点击时间戳(s)
	float lastKickTimeStamp;//上一次点击时间戳(s)
	uint8_t lastKeyVal;
	uint8_t thisKeyVal;
}doubleKickJudge;