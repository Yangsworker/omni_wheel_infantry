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
	
	//��鴫�����ݵ�ַ�ʹ�С
    if (size < 1 || ptChar == NULL)
    {
        return 0;
    }
   
    I2C_Err |= iicCheckBusy();					//��������Ƿ�ռ��
    I2C_Err |= iicStart();						//������ʼ�ź�
    I2C_Err |= iicWAddr(SlaveAddress);			//���ʹӻ���ַ+W
    I2C_Err |= iicSendByte(REG_Address >> 8);		//���ͼĴ�����ַ
    I2C_Err |= iicSendByte(REG_Address & 0xff);		//���ͼĴ�����ַ
	//ѭ��д��
    while (size--)
    {
        I2C_Err |= iicSendByte(*(ptChar++));	//����һ���ֽڲ����Ӧ��λ
    }
    iicStop();									//����ֹͣλ

    return !I2C_Err;
}

u8 Bl24Cxx::regReadData(uint8_t SlaveAddress, uint16_t REG_Address, uint8_t *ptChar, uint8_t size)
{
    uint8_t I2C_Err = 0;

	//��鴫�����ݵ�ַ�ʹ�С
    if (size < 1 || ptChar == NULL)
    {
        return 0;
    }
    //�������æ��������æ���ܴ���Ӳ��IIC��������״̬�����³�ʼ����λ
    if (iicCheckBusy())
    {
        iicInit();
        return 0;
    }

    I2C_Err |= iicStart();						//������ʼ�ź�
    I2C_Err |= iicWAddr(SlaveAddress);			//���ʹӻ���ַ+W
    I2C_Err |= iicSendByte(REG_Address >> 8);		//���ͼĴ�����ַ
    I2C_Err |= iicSendByte(REG_Address & 0xff);		//���ͼĴ�����ַ
    I2C_Err |= iicStart();						//������ʼ�ź�
    I2C_Err |= iicRAddr(SlaveAddress);			//���ʹӻ���ַ+R
    //ѭ����ȡ
    while (--size)
    {
        I2C_Err |= iicReceiveDataByte_Ack(ptChar++);	//��ȡһ���ֽڲ�����Ӧ��λ
    }
    I2C_Err |= iicReceiveDataByte_NoAck(ptChar++);	//���һ�����ݲ�����Ӧ��λ
    iicStop();									//����STOP�ź�
    
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