#include "bl24c512a.h"
Bl24Cxx bl24C512(0x50, GPIOA, GPIO_Pin_5, GPIO_Pin_7);

Bl24Cxx::Bl24Cxx(uint8_t slaveAddress, GPIO_TypeDef *iicPort, uint16_t iicSclPin, uint16_t iicSdaPin, GPIO_TypeDef *iicSdaPort, I2C_TypeDef *IIC)
#if _USE_HARDIIC
		: Hardiic(iicPort, iicSclPin, iicSdaPin,IIC,400000,iicSdaPort)
#else
        : Softiic(iicPort, iicSclPin, iicSdaPin,IIC,1000000,iicSdaPort)
#endif
{
	this->slaveAddress = slaveAddress;
}

u8 Bl24Cxx::regWriteData(uint8_t SlaveAddress, uint16_t REG_Address, uint8_t *ptChar, uint8_t size)
{
    uint8_t I2C_Err = 0;
	
	//检查传入数据地址和大小
    if (size < 1 || ptChar == NULL)
    {
        return 0;
    }
   
    I2C_Err |= iicCheckBusy();					//检查总线是否被占用
    I2C_Err |= iicStart();						//发送起始信号
    I2C_Err |= iicWAddr(SlaveAddress);			//发送从机地址+W
    I2C_Err |= iicSendByte(REG_Address >> 8);		//发送寄存器地址
    I2C_Err |= iicSendByte(REG_Address & 0xff);		//发送寄存器地址
	//循环写入
    while (size--)
    {
        I2C_Err |= iicSendByte(*(ptChar++));	//发送一个字节并检查应答位
    }
    iicStop();									//发送停止位

    return !I2C_Err;
}

u8 Bl24Cxx::regReadData(uint8_t SlaveAddress, uint16_t REG_Address, uint8_t *ptChar, uint8_t size)
{
    uint8_t I2C_Err = 0;

	//检查传入数据地址和大小
    if (size < 1 || ptChar == NULL)
    {
        return 0;
    }
    //检测总线忙，若总线忙可能处于硬件IIC总线死锁状态，重新初始化复位
    if (iicCheckBusy())
    {
        iicInit();
        return 0;
    }

    I2C_Err |= iicStart();						//发送起始信号
    I2C_Err |= iicWAddr(SlaveAddress);			//发送从机地址+W
    I2C_Err |= iicSendByte(REG_Address >> 8);		//发送寄存器地址
    I2C_Err |= iicSendByte(REG_Address & 0xff);		//发送寄存器地址
    I2C_Err |= iicStart();						//发送起始信号
    I2C_Err |= iicRAddr(SlaveAddress);			//发送从机地址+R
    //循环读取
    while (--size)
    {
        I2C_Err |= iicReceiveDataByte_Ack(ptChar++);	//读取一个字节并发送应答位
    }
    I2C_Err |= iicReceiveDataByte_NoAck(ptChar++);	//最后一次数据不发送应答位
    iicStop();									//发送STOP信号
    
    return !I2C_Err;
}

void Bl24Cxx::writeByte(uint16_t dataAddr, uint8_t data)
{
	regWriteData(slaveAddress, dataAddr, &data, 1);
}

void Bl24Cxx::writePage(uint16_t dataAddr, u8 *dataBuff, u8 size)
{
	regWriteData(slaveAddress, dataAddr, dataBuff, size);
}

u8 Bl24Cxx::readByte(uint16_t dataAddr)
{
	u8 ret = 0;
	regReadData(slaveAddress, dataAddr, &ret, 1);
	return ret;
}

void Bl24Cxx::readPage(uint16_t dataAddr, u8 *dataBuff, u8 size)
{
	regReadData(slaveAddress, dataAddr, dataBuff, size);
}