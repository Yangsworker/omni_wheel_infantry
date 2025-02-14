#ifndef __BL24C512A_H
#define __BL24C512A_H

#include "board.h"
#include "iic.h"

struct Bl24Cxx : public Softiic
{
	Bl24Cxx(uint8_t slaveAddress, GPIO_TypeDef *iicPort, uint16_t iicSclPin, uint16_t iicSdaPin = 0, GPIO_TypeDef *iicSdaPort = 0, I2C_TypeDef *IIC = I2C1);
	uint8_t slaveAddress;
	u8 regWriteData(uint8_t SlaveAddress, uint16_t REG_Address, uint8_t *ptChar, uint8_t size);
	u8 regReadData(uint8_t SlaveAddress, uint16_t REG_Address, uint8_t *ptChar, uint8_t size);

	void writeByte(uint16_t dataAddr, u8 data);
	void writePage(uint16_t dataAddr, u8 *dataBuff, u8 size);
	u8 readByte(uint16_t dataAddr);
	void readPage(uint16_t dataAddr, u8 *dataBuff, u8 size);
};


extern Bl24Cxx bl24C512;

#endif
