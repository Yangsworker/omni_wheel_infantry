#ifndef _CRC_H
#define _CRC_H

#include "board.h"
#define NULL 0                   /* see <stddef.h> */
extern const unsigned char CRC8_TAB[256];
extern const uint16_t wCRC_Table[256];
unsigned char Get_CRC8_Check_Sum(unsigned char *pchMessage,unsigned  int dwLength,unsigned char ucCRC8);
unsigned int Verify_CRC8_Check_Sum(unsigned char *pchMessage, unsigned int dwLength);
void Append_CRC8_Check_Sum(unsigned char *pchMessage, unsigned int dwLength);
uint16_t Get_CRC16_Check_Sum(uint8_t *pchMessage,uint32_t dwLength,uint16_t wCRC);
uint32_t Verify_CRC16_Check_Sum(uint8_t *pchMessage, uint32_t dwLength);
void Append_CRC16_Check_Sum(uint8_t * pchMessage,uint32_t dwLength);



#endif 
