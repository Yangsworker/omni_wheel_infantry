#include "board.h"
/*裁判系统测频用
extern int Status_data_cnt, Robot_survival_data_cnt, Robot_State_Data_cnt, Real_time_power_and_heat_data_cnt, Robot_position_data_cnt;
int AVERAGE_Status_data_cnt = 0, AVERAGE_Robot_survival_data_cnt = 0, AVERAGE_Robot_State_Data_cnt = 0, AVERAGE_Real_time_power_and_heat_data_cnt = 0, AVERAGE_Robot_position_data_cnt = 0;
float Status_data_cnt_freq, Robot_survival_data_cnt_freq, Robot_State_Data_cnt_freq, Real_time_power_and_heat_data_cnt_freq, Robot_position_data_cnt_freq;
float AVERAGE_Status_data_cnt_freq = 0, AVERAGE_Robot_survival_data_cnt_freq = 0, AVERAGE_Robot_State_Data_cnt_freq = 0, AVERAGE_Real_time_power_and_heat_data_cnt_freq = 0, AVERAGE_Robot_position_data_cnt_freq = 0;
float SUM_Status_data_cnt_freq = 0, SUM_Robot_survival_data_cnt_freq = 0, SUM_Robot_State_Data_cnt_freq = 0, SUM_Real_time_power_and_heat_data_cnt_freq = 0, SUM_Robot_position_data_cnt_freq = 0;
*/
//裁判系统测错误率用
extern int Cycle_Wrong_num;
float Cycle_Wrong_time;
extern int CRC_Wrong_num;
float CRC_Wrong_time;


int main(void)
{

	TDT_Board_ALL_Init();	
	jgmt_mesg.robotHurt.armorId = 5;
	jgmt_mesg.robotHurt.hurtType = 3;
	while(1)
	{		
//CRC错误频率及包圈错误计时，裁判系统调试用
//		if(Cycle_Wrong_num > 100)
//		{
//			Cycle_Wrong_num = 0;
//			Cycle_Wrong_time = Get_Cycle_T(4);
//		}
//		if(CRC_Wrong_num > 100)
//		{
//			CRC_Wrong_num = 0;
//			CRC_Wrong_time = Get_Cycle_T(5);
//		}


/*裁判系统测频用(服务器架设完成可加入)
		if(Status_data_cnt != 0)
		{
			SUM_Status_data_cnt_freq += Get_Cycle_T(4);
			AVERAGE_Status_data_cnt += Status_data_cnt;
			AVERAGE_Status_data_cnt_freq = AVERAGE_Status_data_cnt / SUM_Status_data_cnt_freq;
			Status_data_cnt = 0;
		}
		if(Robot_survival_data_cnt != 0)
		{
			Robot_survival_data_cnt_freq = Robot_survival_data_cnt / Get_Cycle_T(5);
			SUM_Robot_survival_data_cnt_freq += Robot_survival_data_cnt_freq;
			AVERAGE_Robot_survival_data_cnt_freq = SUM_Robot_survival_data_cnt_freq / AVERAGE_Robot_survival_data_cnt_freq;
			AVERAGE_Robot_survival_data_cnt_freq++;
			Robot_survival_data_cnt = 0;
		}
*/
/*裁判系统测频用

//		if(Robot_State_Data_cnt != 0)
//		{
//			SUM_Robot_State_Data_cnt_freq += Get_Cycle_T(6);
//			AVERAGE_Robot_State_Data_cnt += Robot_State_Data_cnt;
//			AVERAGE_Robot_State_Data_cnt_freq = AVERAGE_Robot_State_Data_cnt / SUM_Robot_State_Data_cnt_freq;
//			Robot_State_Data_cnt = 0;
//		}
//		if(Real_time_power_and_heat_data_cnt != 0)
//		{
//			SUM_Real_time_power_and_heat_data_cnt_freq += Get_Cycle_T(7);
//			AVERAGE_Real_time_power_and_heat_data_cnt += Real_time_power_and_heat_data_cnt;
//			AVERAGE_Real_time_power_and_heat_data_cnt_freq = AVERAGE_Real_time_power_and_heat_data_cnt / SUM_Real_time_power_and_heat_data_cnt_freq;
//			Real_time_power_and_heat_data_cnt = 0;
//		}
//		if(Robot_position_data_cnt != 0)
//		{
//			SUM_Robot_position_data_cnt_freq += Get_Cycle_T(8);
//			AVERAGE_Robot_position_data_cnt += Robot_position_data_cnt;
//			AVERAGE_Robot_position_data_cnt_freq = AVERAGE_Robot_position_data_cnt / SUM_Robot_position_data_cnt_freq;
//			Robot_position_data_cnt = 0;
//		}
*/

//        USART_SendData(USART1,0xa5);
//        while(!USART_GetFlagStatus(USART1,USART_FLAG_TC));
	}
}

