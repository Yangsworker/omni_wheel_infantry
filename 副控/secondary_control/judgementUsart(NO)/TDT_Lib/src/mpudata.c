#include "mpudata.h"

mpu mpu6050Top;

vec3f Gyro,Acc;
//mpu6050读取数据
void Mpu6050Top_Read(void)
{
    MPU_Get_Gyroscope();
    MPU_Get_Accelerometer();
}

//mpu6050数据准备
void Mpu6050Top_Data_Prepare(void)
{	
	u8 i;
	int32_t FILT_TMP[ITEMS] = {0};
  static int16_t FILT_BUF[ITEMS][MPU6050_FILTER_NUM] = {0};

	/* 得出校准后的数据 */
	mpu6050Top.acc.calibration.data[xx] = mpu6050Top.acc.origin.data[xx]  - mpu6050Top.acc.offset.data[xx] ;
	mpu6050Top.acc.calibration.data[yy] = mpu6050Top.acc.origin.data[yy]  - mpu6050Top.acc.offset.data[yy] ;
	mpu6050Top.acc.calibration.data[zz] = mpu6050Top.acc.origin.data[zz]  - mpu6050Top.acc.offset.data[zz] ;
	mpu6050Top.gyro.calibration.data[xx] = mpu6050Top.gyro.origin.data[xx] - mpu6050Top.gyro.offset.data[xx] ;
	mpu6050Top.gyro.calibration.data[yy] = mpu6050Top.gyro.origin.data[yy] - mpu6050Top.gyro.offset.data[yy] ;
	mpu6050Top.gyro.calibration.data[zz] = mpu6050Top.gyro.origin.data[zz] - mpu6050Top.gyro.offset.data[zz] ;
	
  for(i=MPU6050_FILTER_NUM-1;i>=1;i--)
  {
    FILT_BUF[A_X][i] = FILT_BUF[A_X][i-1];
		FILT_BUF[A_Y][i] = FILT_BUF[A_Y][i-1];
		FILT_BUF[A_Z][i] = FILT_BUF[A_Z][i-1];
		FILT_BUF[G_X][i] = FILT_BUF[G_X][i-1];
		FILT_BUF[G_Y][i] = FILT_BUF[G_Y][i-1];
		FILT_BUF[G_Z][i] = FILT_BUF[G_Z][i-1];
  }

	    FILT_BUF[A_X][0] = mpu6050Top.acc.calibration.data[xx];
		FILT_BUF[A_Y][0] = mpu6050Top.acc.calibration.data[yy];
		FILT_BUF[A_Z][0] = mpu6050Top.acc.calibration.data[zz];
		FILT_BUF[G_X][0] = mpu6050Top.gyro.calibration.data[xx];
		FILT_BUF[G_Y][0] = mpu6050Top.gyro.calibration.data[yy];
		FILT_BUF[G_Z][0] = mpu6050Top.gyro.calibration.data[zz];

	for(i=0;i<MPU6050_FILTER_NUM;i++)
	{
		FILT_TMP[A_X] += FILT_BUF[A_X][i];
		FILT_TMP[A_Y] += FILT_BUF[A_Y][i];
		FILT_TMP[A_Z] += FILT_BUF[A_Z][i];
		FILT_TMP[G_X] += FILT_BUF[G_X][i];
		FILT_TMP[G_Y] += FILT_BUF[G_Y][i];
		FILT_TMP[G_Z] += FILT_BUF[G_Z][i];
	}

	mpu6050Top.acc.filter.data[xx] = (float)( FILT_TMP[A_X] )/(float)MPU6050_FILTER_NUM;
	mpu6050Top.acc.filter.data[yy] = (float)( FILT_TMP[A_Y] )/(float)MPU6050_FILTER_NUM;
	mpu6050Top.acc.filter.data[zz] = (float)( FILT_TMP[A_Z] )/(float)MPU6050_FILTER_NUM;

	mpu6050Top.gyro.filter.data[xx] = (float)( FILT_TMP[G_X] )/(float)MPU6050_FILTER_NUM;
	mpu6050Top.gyro.filter.data[yy] = (float)( FILT_TMP[G_Y] )/(float)MPU6050_FILTER_NUM;
	mpu6050Top.gyro.filter.data[zz] = (float)( FILT_TMP[G_Z] )/(float)MPU6050_FILTER_NUM;
	
	mpu6050Top.gyro.dps.data[xx] = mpu6050Top.gyro.filter.data[xx] * TO_ANGLE;
	mpu6050Top.gyro.dps.data[yy] = mpu6050Top.gyro.filter.data[yy] * TO_ANGLE;
	mpu6050Top.gyro.dps.data[zz] = mpu6050Top.gyro.filter.data[zz] * TO_ANGLE;
	
	mpu6050Top.gyro.radps.data[xx] = mpu6050Top.gyro.dps.data[xx] * ANGLE_TO_RAD;
	mpu6050Top.gyro.radps.data[yy] = mpu6050Top.gyro.dps.data[yy] * ANGLE_TO_RAD;
	mpu6050Top.gyro.radps.data[zz] = mpu6050Top.gyro.dps.data[zz] * ANGLE_TO_RAD;
}

#define Auto_imu_Offset
////////#define Manual_imu_Offset

void Mpu6050Top_CalOffset_Gyro(void)
{
	#ifdef Auto_imu_Offset
	uint16_t cnt_g =1000;
	int32_t tempgx =0, tempgy =0, tempgz =0;

	mpu6050Top.gyro.offset_max.data[xx] = -32768;
	mpu6050Top.gyro.offset_max.data[yy] = -32768;
	mpu6050Top.gyro.offset_max.data[zz] = -32768;
	mpu6050Top.gyro.offset_min.data[xx] = 32767;
	mpu6050Top.gyro.offset_min.data[yy] = 32767;
	mpu6050Top.gyro.offset_min.data[zz] = 32767;

	while(cnt_g--)	 
	{   
		delay_ms(2);			
		Mpu6050Top_Read();
//		ADXRS453SensorData();
		if(mpu6050Top.gyro.origin.data[xx]>mpu6050Top.gyro.offset_max.data[xx]) 
			mpu6050Top.gyro.offset_max.data[xx] = mpu6050Top.gyro.origin.data[xx];
		if(mpu6050Top.gyro.origin.data[yy]>mpu6050Top.gyro.offset_max.data[yy]) 
			mpu6050Top.gyro.offset_max.data[yy] = mpu6050Top.gyro.origin.data[yy];
		if(mpu6050Top.gyro.origin.data[zz]>mpu6050Top.gyro.offset_max.data[zz]) 
			mpu6050Top.gyro.offset_max.data[zz] = mpu6050Top.gyro.origin.data[zz];
		
		if(mpu6050Top.gyro.origin.data[xx]<mpu6050Top.gyro.offset_min.data[xx]) 
			mpu6050Top.gyro.offset_min.data[xx] = mpu6050Top.gyro.origin.data[xx];
		if(mpu6050Top.gyro.origin.data[yy]<mpu6050Top.gyro.offset_min.data[yy]) 
			mpu6050Top.gyro.offset_min.data[yy] = mpu6050Top.gyro.origin.data[yy];
		if(mpu6050Top.gyro.origin.data[zz]<mpu6050Top.gyro.offset_min.data[zz]) 
			mpu6050Top.gyro.offset_min.data[zz] = mpu6050Top.gyro.origin.data[zz];
		
			tempgx += mpu6050Top.gyro.origin.data[xx];
			tempgy += mpu6050Top.gyro.origin.data[yy];
			tempgz += mpu6050Top.gyro.origin.data[zz];
	}
	 
	 //1000次数据有一个异常,重新校准
	 if(mpu6050Top.gyro.offset_max.data[xx]-mpu6050Top.gyro.offset_min.data[xx]>200||
			mpu6050Top.gyro.offset_max.data[yy]-mpu6050Top.gyro.offset_min.data[yy]>200||
			mpu6050Top.gyro.offset_max.data[zz]-mpu6050Top.gyro.offset_min.data[zz]>200)
			Mpu6050Top_CalOffset_Gyro();	 
	 else
	 {
			mpu6050Top.gyro.offset.data[xx] = (float)(tempgx)/1000;
			mpu6050Top.gyro.offset.data[yy] = (float)(tempgy)/1000;
			mpu6050Top.gyro.offset.data[zz] = (float)(tempgz)/1000;
	 }
#endif
	 
#ifdef Manual_imu_Offset	 
	 mpu6050Top.gyro.offset.data[xx] =-160.996994;//	-18.813004f;
	 mpu6050Top.gyro.offset.data[yy] =31.257; // -70.425003;
	 mpu6050Top.gyro.offset.data[zz] = -33.257;// -37.5309982f; 
	 #endif
}
