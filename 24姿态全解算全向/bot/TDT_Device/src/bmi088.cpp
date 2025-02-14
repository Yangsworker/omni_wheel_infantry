/*
Description: BMI088�����ǳ�ʼ���Լ����ݶ�ȡ
Author: liuskywalkerjskd
*/
#include "bmi088.h"
#include "bmi088_reg.h"
#include "filter.h"
#include "imu_task.h"

//ʹ��088��������Ҫ��ȡboardImu->AHRS_data.Angle

Lpf2p filter_lpf_50 ;
Lpf2p filter_lpf_100 ;
Lpf2p filter_lpf_150 ;
Lpf2p filter_lpf_200 ;
Lpf2p filter_lpf_250 ;
Lpf2p filter_lpf_300 ;
Lpf2p filter_lpf_350 ;
Lpf2p filter_lpf_400 ;
Lpf2p filter_lpf_450 ;

float watchgyro[10] ;

float watchtemp = 0 ;

/**
  * @brief BMI088 Accelerator configuration data and Error Status
  */
//������û�к����жϽǣ�������������ж�
static uint8_t Accel_Register_ConfigurationData_ErrorStatus[BMI088_WRITE_ACCEL_REG_NUM][3] =
{
    /* Turn on accelerometer */
    {BMI088_ACC_PWR_CTRL, BMI088_ACC_ENABLE_ACC_ON, BMI088_ACC_PWR_CTRL_ERROR},   

    /* Pause mode */
    {BMI088_ACC_PWR_CONF, BMI088_ACC_PWR_ACTIVE_MODE, BMI088_ACC_PWR_CONF_ERROR}, 

    /* Acceleration Configuration */
    {BMI088_ACC_CONF,  (BMI088_ACC_NORMAL| BMI088_ACC_800_HZ | BMI088_ACC_CONF_MUST_Set), BMI088_ACC_CONF_ERROR}, 

    /* Accelerometer setting range */ 
    {BMI088_ACC_RANGE, BMI088_ACC_RANGE_6G, BMI088_ACC_RANGE_ERROR},  

    /* INT1 Configuration input and output pin */ 
//    {BMI088_INT1_IO_CTRL, (BMI088_ACC_INT1_IO_ENABLE | BMI088_ACC_INT1_GPIO_PP | BMI088_ACC_INT1_GPIO_LOW), BMI088_INT1_IO_CTRL_ERROR}, 

    /* interrupt map pin */
//    {BMI088_INT_MAP_DATA, BMI088_ACC_INT1_DRDY_INTERRUPT, BMI088_INT_MAP_DATA_ERROR}  
};

/**
  * @brief BMI088 Gyro configuration data and Error Status
  */
//������û�к����жϽǣ�������������ж�
static uint8_t Gyro_Register_ConfigurationData_ErrorStatus[BMI088_WRITE_GYRO_REG_NUM][3] =
{
    /* Angular rate and resolution */
    {BMI088_GYRO_RANGE, BMI088_GYRO_2000, BMI088_GYRO_RANGE_ERROR}, 

    /* Data Transfer Rate and Bandwidth Settings */
    {BMI088_GYRO_BANDWIDTH, (BMI088_GYRO_2000_230_HZ | BMI088_GYRO_BANDWIDTH_MUST_Set), BMI088_GYRO_BANDWIDTH_ERROR}, 

    /* Power Mode Selection Register */
    {BMI088_GYRO_LPM1, BMI088_GYRO_NORMAL_MODE, BMI088_GYRO_LPM1_ERROR},   

    /* Data Interrupt Trigger Register */
    {BMI088_GYRO_CTRL, BMI088_DRDY_ON, BMI088_GYRO_CTRL_ERROR},   

    /* Interrupt Pin Trigger Register */
//    {BMI088_GYRO_INT3_INT4_IO_CONF, (BMI088_GYRO_INT3_GPIO_PP | BMI088_GYRO_INT3_GPIO_LOW), BMI088_GYRO_INT3_INT4_IO_CONF_ERROR},  

    /* interrupt map register */
//    {BMI088_GYRO_INT3_INT4_IO_MAP, BMI088_GYRO_DRDY_IO_INT3, BMI088_GYRO_INT3_INT4_IO_MAP_ERROR}   
};
//���캯��
Bmi088::Bmi088(SPI_TypeDef* spix , int baud) : Spi(spix , baud) 
{
	float BMI088_ACCEL_SEN = BMI088_ACCEL_6G_SEN; //���������޸�
	float BMI088_GYRO_SEN = BMI088_GYRO_2000_SEN; //���������޸�		
	imuview = this ;
	accValueFector = BMI088_ACCEL_SEN ;
	gyroDpsFector = 1/16.4f; ;
}
/*
	����V6���ص�BMI088�����ǲ�壬��ȡ����PA4��Ƭѡ�ż�ȡ����·ʵ�ּ��ٶȼƺ�������������������Ƭѡ
	�����SPIƬѡ��ʱ��PA4 ����_���� �� ����_���� �ֱ�����Ƭѡ����������
*/
u8 Bmi088::acc_readByte(u8 regAddr) //���ٶȼƵ��ֽڶ�ȡ
{
	csOn();
	u8 val;
	readWriteByte(regAddr | 0x80); //Register. MSB 1 is read instruction.
	readWriteByte(0x00); //���ٶȼ�������Ҫ���ѯһ��
	val = readWriteByte(0x00); //Send DUMMY to read data
	csOff();
	return val;
}

void Bmi088::acc_readBytes(u8 regAddr, u8 len, u8* data) //���ٶȼƶ��ֽڶ�ȡ
{
	csOn();
	readWriteByte(regAddr | 0x80); //Register. MSB 1 is read instruction.
	readWriteByte(regAddr | 0x80); //���ٶȼ�������Ҫ���һ��
	for(u8 i=0;i<len;i++)
	{
		data[i] = readWriteByte(0x00); //Send DUMMY to read data
	}
	csOff();
}

void Bmi088::acc_writeByte(u8 regAddr, u8 data) //���ٶȼƵ��ֽ�д��
{
	csOn();
	readWriteByte(regAddr & 0x7F); //Register. MSB 0 is write instruction.
	readWriteByte(data); //Send Data to write
	csOff();
}

void Bmi088::acc_writeBytes(u8 regAddr, u8 len, u8* data) //���ٶȼƶ��ֽ�д��
{
	csOn();
	readWriteByte(regAddr & 0x7F); //Register. MSB 0 is write instruction.
	for(u8 i=0;i<len;i++)
	{
		readWriteByte(data[i++]); //Send Data to write
	}
	csOff();
}
		
u8 Bmi088::gyro_toggle_readByte(u8 regAddr) //�����ǵ��ֽڶ�ȡ_Ƭѡȡ����·
{
	csOff();
	u8 val;
	readWriteByte(regAddr | 0x80); //Register. MSB 1 is read instruction.
	val = readWriteByte(0x00); //Send DUMMY to read data
	csOn();
	return val;
}

void Bmi088::gyro_toggle_readBytes(u8 regAddr, u8 len, u8* data) //�����Ƕ��ֽڶ�ȡ_Ƭѡȡ����·
{
	csOff();
	readWriteByte(regAddr | 0x80); //Register. MSB 1 is read instruction.
	for(u8 i=0;i<len;i++)
	{
		data[i] = readWriteByte(0x00); //Send DUMMY to read data
	}
	csOn();
}

void Bmi088::gyro_toggle_writeByte(u8 regAddr, u8 data) //�����ǵ��ֽ�д��_Ƭѡȡ����·
{
	csOff();
	readWriteByte(regAddr & 0x7F); //Register. MSB 0 is write instruction.
	readWriteByte(data); //Send Data to write
	csOn();
}

void Bmi088::gyro_toggle_writeBytes(u8 regAddr, u8 len, u8* data) //�����Ƕ��ֽ�д��_Ƭѡȡ����·
{
	csOff();
	readWriteByte(regAddr & 0x7F); //Register. MSB 0 is write instruction.
	for(u8 i=0;i<len;i++)
	{
		readWriteByte(data[i++]); //Send Data to write
	}
	csOn();
}
//���ٶȼƳ�ʼ��
uint16_t Bmi088::acc_init(void)
{
	uint8_t res = 0 ;
	
	delayMs(BMI088_LONG_DELAY_TIME) ;
	
	res = acc_readByte(BMI088_ACC_CHIP_ID) ;
	delayUs(BMI088_COM_WAIT_SENSOR_TIME) ;
	//��Ҫ�ȶ�һ���л���SPIģʽ
	res = acc_readByte(BMI088_ACC_CHIP_ID) ;
	delayUs(BMI088_COM_WAIT_SENSOR_TIME) ;
	
	acc_writeByte(BMI088_ACC_SOFTRESET,BMI088_ACC_SOFTRESET_VALUE) ;
	delayMs(BMI088_LONG_DELAY_TIME) ;
	
	res = acc_readByte(BMI088_ACC_CHIP_ID) ;
	delayUs(BMI088_COM_WAIT_SENSOR_TIME) ;
	//��Ҫ�ȶ�һ���л���SPIģʽ
	res = acc_readByte(BMI088_ACC_CHIP_ID) ;
	delayUs(BMI088_COM_WAIT_SENSOR_TIME) ;
	
	if (res != BMI088_ACC_CHIP_ID_VALUE)
    {
		Error_Status = BMI088_NO_SENSOR ;
		return Error_Status ;
    }
	
	//ȷ�϶������ٶȼƵĴ��ڣ������������
	
	/* config the accelerator sensor */
    for (uint8_t write_reg_num = 0; write_reg_num < BMI088_WRITE_ACCEL_REG_NUM; write_reg_num++)
    {
        acc_writeByte(Accel_Register_ConfigurationData_ErrorStatus[write_reg_num][0], Accel_Register_ConfigurationData_ErrorStatus[write_reg_num][1]) ; 
        /* waiting 150us */
        delayUs(BMI088_COM_WAIT_SENSOR_TIME) ;

        /* read the configuration */
        res = acc_readByte(Accel_Register_ConfigurationData_ErrorStatus[write_reg_num][0]);
        /* waiting 150us */
        delayUs(BMI088_COM_WAIT_SENSOR_TIME) ;
        
        /* check the configuration and return the specified error */
        if (res != Accel_Register_ConfigurationData_ErrorStatus[write_reg_num][1])
        {
            Error_Status = (BMI088_Status_e)Accel_Register_ConfigurationData_ErrorStatus[write_reg_num][2] ;
			return Error_Status ;
        }
    }

    /* no error */
    Error_Status = BMI088_NO_ERROR ; 
	return Error_Status ;
}
//�����ǳ�ʼ��
uint16_t Bmi088::gyro_init(void)
{
	uint8_t res = 0;
	res = gyro_toggle_readByte(BMI088_GYRO_CHIP_ID) ;
	delayUs(BMI088_COM_WAIT_SENSOR_TIME) ;
	res = gyro_toggle_readByte(BMI088_GYRO_CHIP_ID) ;
	delayUs(BMI088_COM_WAIT_SENSOR_TIME) ;
	
	gyro_toggle_writeByte(BMI088_GYRO_SOFTRESET,BMI088_GYRO_SOFTRESET_VALUE) ;
	delayMs(BMI088_LONG_DELAY_TIME) ;
	
	res = gyro_toggle_readByte(BMI088_GYRO_CHIP_ID) ;
	delayUs(BMI088_COM_WAIT_SENSOR_TIME) ;
	res = gyro_toggle_readByte(BMI088_GYRO_CHIP_ID) ;
	delayUs(BMI088_COM_WAIT_SENSOR_TIME) ;
	
	if (res != BMI088_GYRO_CHIP_ID_VALUE)
    {
		Error_Status = BMI088_NO_SENSOR ;
		return Error_Status ;
    }
	
	//ȷ�϶��������ǵĴ��ڣ������������
	
	for (uint8_t write_reg_num = 0; write_reg_num < BMI088_WRITE_GYRO_REG_NUM; write_reg_num++)
    {
        gyro_toggle_writeByte(Gyro_Register_ConfigurationData_ErrorStatus[write_reg_num][0], Gyro_Register_ConfigurationData_ErrorStatus[write_reg_num][1]) ;
        /* waiting 150us */
        delayUs(BMI088_COM_WAIT_SENSOR_TIME) ;

        /* read the configuration */
        res = gyro_toggle_readByte(Gyro_Register_ConfigurationData_ErrorStatus[write_reg_num][0]) ;
        /* waiting 150us */
        delayUs(BMI088_COM_WAIT_SENSOR_TIME) ;

        /* check the configuration and return the specified error */
        if (res != Gyro_Register_ConfigurationData_ErrorStatus[write_reg_num][1])
        {
            Error_Status = (BMI088_Status_e)Gyro_Register_ConfigurationData_ErrorStatus[write_reg_num][2] ;
			return Error_Status ;
        }
    }

    /* no error */
    Error_Status =  BMI088_NO_ERROR ;
	return Error_Status ;
}

void Bmi088::init(void)
{
	uint16_t status = BMI088_NO_ERROR ; //��¼������
	
	Spi::init() ;
	delayMs(50) ;
	csInit({GPIOA,GPIO_Pin_4}) ;
	delayMs(50) ;
	
	do//ֱ��������errorΪֹ
	{
		status |= acc_init() ;
		delayMs(2);
		status |= gyro_init() ;
		delayMs(2);
	}while(status) ;
	
	ImuCalc::init();
	
	//��ʼ���˲���
	filter_lpf_50.SetCutoffFreq(500,50) ;
	filter_lpf_100.SetCutoffFreq(500,100) ;
	filter_lpf_150.SetCutoffFreq(500,150) ;
	filter_lpf_200.SetCutoffFreq(500,200) ;
	filter_lpf_250.SetCutoffFreq(500,250) ;
	filter_lpf_300.SetCutoffFreq(500,300) ;
	filter_lpf_350.SetCutoffFreq(500,350) ;
	filter_lpf_400.SetCutoffFreq(500,400) ;
	filter_lpf_450.SetCutoffFreq(500,450) ;
	
	return ;
}
//��ȡȫ������
void Bmi088::get6AxisRawData(void)
{
	uint8_t buf[8] = {0} ;
	
	acc_readBytes(BMI088_ACCEL_XOUT_L , 6 , buf) ;
	accRaw[0] = (int16_t)((buf[1] << 8) | buf[0]);
	accRaw[1] = (int16_t)((buf[3] << 8) | buf[2]);
	accRaw[2] = (int16_t)((buf[5] << 8) | buf[4]);
	
	acc_readBytes(BMI088_TEMP_M , 2 , buf) ;
	tempRaw = (int16_t)((buf[0] << 3) | (buf[1] >> 5));
	if(tempRaw > 1023)
	{
		tempRaw -= 2048 ;
	}
	tempRaw = BMI088_TEMP_FACTOR * tempRaw + BMI088_TEMP_OFFSET ;
	watchtemp = tempRaw ;
	
	gyro_toggle_readBytes(BMI088_GYRO_XOUT_L , 6 , buf) ;
	gyroRaw[0] = (int16_t)((buf[1] << 8) | buf[0]);
	gyroRaw[1] = (int16_t)((buf[3] << 8) | buf[2]);
	gyroRaw[2] = (int16_t)((buf[5] << 8) | buf[4]);
}
//��ȡ�����Ƕ���
void Bmi088::get3AxisGyroRawData(void)
{
	uint8_t buf[8] = {0} ;
	
	gyro_toggle_readBytes(BMI088_GYRO_XOUT_L , 6 , buf) ;
	gyroRaw[0] = (int16_t)((buf[1] << 8) | buf[0]);
	gyroRaw[1] = (int16_t)((buf[3] << 8) | buf[2]);
	gyroRaw[2] = (int16_t)((buf[5] << 8) | buf[4]);
}
//��ȡ���ٶȼƶ���
void Bmi088::get3AxisAccRawData(void)
{
	uint8_t buf[8] = {0} ;
	
	acc_readBytes(BMI088_ACCEL_XOUT_L , 6 , buf) ;
	accRaw[0] = (int16_t)((buf[1] << 8) | buf[0]);
	accRaw[1] = (int16_t)((buf[3] << 8) | buf[2]);
	accRaw[2] = (int16_t)((buf[5] << 8) | buf[4]);
}
//��ȡ�¶ȣ����϶ȣ�
void Bmi088::getTempRawData(void)
{
	uint8_t buf[8] = {0} ;

	acc_readBytes(BMI088_TEMP_M , 2 , buf) ;
	tempRaw = (int16_t)((buf[0] << 3) | (buf[1] >> 5));
	if(tempRaw > 1023)
	{
		tempRaw -= 2048 ;
	}
	tempRaw = BMI088_TEMP_FACTOR * tempRaw + BMI088_TEMP_OFFSET ;//��ֵ����
	watchtemp = tempRaw ;
}
//״̬����
void Bmi088::gyroAccUpdate()
{
	get6AxisRawData();
	
	if(!(accRaw[0] == 0 && accRaw[1] == 0 && accRaw[2] ==0))
	{
		acc.origin.data[0] = accRaw[0];
		acc.origin.data[1] = accRaw[1];
		acc.origin.data[2] = accRaw[2];
	}
	if(!(gyroRaw[0] == 0 && gyroRaw[0] == 0 && gyroRaw[0] == 0))
	{
		gyro.origin.data[0] = gyroRaw[0];
		gyro.origin.data[1] = gyroRaw[1];
		gyro.origin.data[2] = gyroRaw[2];
	}
	
	
	//��yaw��ԭʼ����Ӧ�ò�ͬ��ֹƵ�ʵĵ�ͨ�˲����۲�Ч��
	//�˴���Ӧʹ�ô�ͨ�˲����ͨ�˲�����Ϊת��������Ƶ�������ǵ�Ƶ�źţ������������˲����ή��������
	
	watchgyro[0] = gyro.origin.data[2] ;
	watchgyro[1] = filter_lpf_50.Apply(gyro.origin.data[2]) ;
	watchgyro[2] = filter_lpf_100.Apply(gyro.origin.data[2]) ;
	watchgyro[3] = filter_lpf_150.Apply(gyro.origin.data[2]) ;
	watchgyro[4] = filter_lpf_200.Apply(gyro.origin.data[2]) ;
	watchgyro[5] = filter_lpf_250.Apply(gyro.origin.data[2]) ;
	watchgyro[6] = filter_lpf_300.Apply(gyro.origin.data[2]) ;
	watchgyro[7] = filter_lpf_350.Apply(gyro.origin.data[2]) ;
	watchgyro[8] = filter_lpf_400.Apply(gyro.origin.data[2]) ;
	watchgyro[9] = filter_lpf_450.Apply(gyro.origin.data[2]) ;
	
	gyro.origin.data[2] = watchgyro[1] ; //ѡȡ�˲����ֺõ�һ���˲��������yaw��ԭʼ����
}
