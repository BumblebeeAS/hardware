#include "screen.h"
#include "sensor.h"
 uint8_t i2ctraBuf[8];
 uint8_t i2crecBuf[8];
 uint16_t imudata[8];

 // sensor variables
 uint8_t humidity = 0;
 uint8_t IntPressure = 0;
 uint8_t temperature = 0;
 uint8_t InitialP = 0;
 uint16_t rawExtPressure[8];

 uint32_t HIH_read_loop=0;

 //raw ExtPressure data
 uint32_t D1;
 uint32_t D2;
 int32_t TEMP;
 int32_t P;




/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */

void init_LSM6(void){
	 //lsm6
 	  i2ctraBuf[0]=0x10;//CTRL1_XL
 	  i2ctraBuf[1]=0x80;
 	  HAL_I2C_Master_Transmit(&hi2c1,lsm6_addrt,i2ctraBuf,2,10);
 	  i2ctraBuf[0]=0x11;//CTRL2_G
 	  i2ctraBuf[1]=0x80;
 	  HAL_I2C_Master_Transmit(&hi2c1,lsm6_addrt,i2ctraBuf,2,10);
 	  i2ctraBuf[0]=0x12;//CTRL3_C
 	  i2ctraBuf[1]=0x04;
 	  HAL_I2C_Master_Transmit(&hi2c1,lsm6_addrt,i2ctraBuf,2,10);
 	  //lis3
 	  i2ctraBuf[0]=0x20;
 	  i2ctraBuf[1]=0x70;
 	  HAL_I2C_Master_Transmit(&hi2c1,lis3_addrt,i2ctraBuf,2,10);
 	  i2ctraBuf[0]=0x21;
 	  i2ctraBuf[1]=0x00;
 	  HAL_I2C_Master_Transmit(&hi2c1,lis3_addrt,i2ctraBuf,2,10);
 	  i2ctraBuf[0]=0x22;
 	  i2ctraBuf[1]=0x00;
 	  HAL_I2C_Master_Transmit(&hi2c1,lis3_addrt,i2ctraBuf,2,10);
 	  i2ctraBuf[0]=0x23;
 	  i2ctraBuf[1]=0x0C;
 	  HAL_I2C_Master_Transmit(&hi2c1,lis3_addrt,i2ctraBuf,2,10);

 }


void LSM6_read(void){
 	i2ctraBuf[0]=0x28;//CTRL1_XL
     HAL_I2C_Master_Transmit(&hi2c1,lsm6_addrt,i2ctraBuf,1,10);
     HAL_I2C_Master_Receive(&hi2c1,lsm6_addrr,&i2crecBuf[0],6,10);

     uint8_t xla = i2crecBuf[0];
     uint8_t xha = i2crecBuf[1];
     uint8_t yla = i2crecBuf[2];
     uint8_t yha = i2crecBuf[3];
     uint8_t zla = i2crecBuf[4];
     uint8_t zha = i2crecBuf[5];

     // combine high and low bytes
     int16_t a_x = (int16_t)(xha << 8 | xla);
     int16_t a_y = (int16_t)(yha << 8 | yla);
     int16_t a_z = (int16_t)(zha << 8 | zla);
     //convert to m2/s
     a_x=a_x*0.061*9.81/1000;
     a_y=a_y*0.061*9.81/1000;
     a_z=a_z*0.061*9.81/1000;

     internalStats[IMU_A_X]=a_x;
     internalStats[IMU_A_Y]=a_y;
     internalStats[IMU_A_Z]=a_z;

    i2ctraBuf[0]=0x22;
    	     HAL_I2C_Master_Transmit(&hi2c1,lsm6_addrt,i2ctraBuf,1,10);
    	     HAL_I2C_Master_Receive(&hi2c1,lsm6_addrr,&i2crecBuf[0],6,10);

    	     uint8_t xlg = i2crecBuf[0];
    	     uint8_t xhg = i2crecBuf[1];
    	     uint8_t ylg = i2crecBuf[2];
    	     uint8_t yhg = i2crecBuf[3];
    	     uint8_t zlg = i2crecBuf[4];
    	     uint8_t zhg = i2crecBuf[5];

    	     // combine high and low bytes
    	     int16_t g_x = (int16_t)(xhg << 8 | xlg);
    	     int16_t g_y = (int16_t)(yhg << 8 | ylg);
    	     int16_t g_z = (int16_t)(zhg << 8 | zlg);
    	     //convert to degree/s
    	     g_x=g_x*245/32267;
    	     g_y=g_y*245/32267;
    	     g_z=g_z*245/32267;

    	     internalStats[IMU_G_X]=g_x;
    	     internalStats[IMU_G_Y]=g_y;
    	     internalStats[IMU_G_Z]=g_z;
 }

void init_LIS3(void){
	i2ctraBuf[0]=0x20;
	i2ctraBuf[1]=0x70;
	HAL_I2C_Master_Transmit(&hi2c1,lis3_addrt,i2ctraBuf,2,10);
	i2ctraBuf[0]=0x21;
	i2ctraBuf[1]=0x00;
	HAL_I2C_Master_Transmit(&hi2c1,lis3_addrt,i2ctraBuf,2,10);
	i2ctraBuf[0]=0x22;
	i2ctraBuf[1]=0x00;
	HAL_I2C_Master_Transmit(&hi2c1,lis3_addrt,i2ctraBuf,2,10);
	i2ctraBuf[0]=0x23;
	i2ctraBuf[1]=0x0C;
	HAL_I2C_Master_Transmit(&hi2c1,lis3_addrt,i2ctraBuf,2,10);
}

void LIS3_read(void){
	i2ctraBuf[0]=(0x28|0x80);
	HAL_I2C_Master_Transmit(&hi2c1,lis3_addrt,i2ctraBuf,1,10);
	HAL_I2C_Master_Receive(&hi2c1,lis3_addrr,&i2crecBuf[0],6,10);
    uint8_t xlm = i2crecBuf[0];
    uint8_t xhm = i2crecBuf[1];
    uint8_t ylm = i2crecBuf[2];
    uint8_t yhm = i2crecBuf[3];
    uint8_t zlm = i2crecBuf[4];
    uint8_t zhm = i2crecBuf[5];

    // combine high and low bytes
    int16_t m_x = (int16_t)(xhm << 8 | xlm);
    int16_t m_y = (int16_t)(yhm << 8 | ylm);
    int16_t m_z = (int16_t)(zhm << 8 | zlm);
    //unit is mgauss
    m_x=m_x*8000/65536;
    m_y=m_y*8000/65536;
    m_z=m_z*8000/65536;
    internalStats[IMU_M_X]=m_x;
    internalStats[IMU_M_Y]=m_y;
    internalStats[IMU_M_Z]=m_z;

}

void HIH_init(void){
	HAL_I2C_Master_Transmit(&hi2c1,hih_addrt,0,0,10);
}

void HIH_read(void){
	if(HIH_read_loop-HAL_GetTick()>=50){
    uint8_t read_byte,status_data;
	uint16_t temperature_data,humidity_data;
	HAL_I2C_Master_Receive(&hi2c1,hih_addrr,&i2crecBuf[0],4,10);
	read_byte=i2crecBuf[0];
	status_data=read_byte>>6;
	humidity_data=(read_byte&0x3f)<<8;
	read_byte=i2crecBuf[1];
	humidity_data+=read_byte;

	read_byte=i2crecBuf[2];
	temperature_data=read_byte<<6;
	read_byte=i2crecBuf[3];
	temperature_data+=read_byte>>2;

	humidity=humidity_data/(16384.0-1)*100;
	temperature=temperature/(16384.0-1)*165-40;
	internalStats[HUMIDITY]=humidity;
	internalStats[ST_TEMP]=temperature;
	HAL_I2C_Master_Transmit(&hi2c1,hih_addrt,0,0,10);
	HIH_read_loop=HAL_GetTick();
	}


}

void IntPressure_read(void){
	HAL_I2C_Master_Transmit(&hi2c1,pressure_addrt,0,0,10);
	HAL_Delay(50);
	HAL_I2C_Master_Receive(&hi2c1,pressure_addrr,&i2crecBuf[0],2,10);
	uint8_t preh=i2crecBuf[0];
	uint8_t prel=i2crecBuf[1];
	uint16_t prec=(preh<<8)|prel;
	uint16_t pressure=(prec-1638)*60/13107;
	internalStats[INT_PRESS]=prec;

}

void ExtPress_init(void){
	i2ctraBuf[0]=MS5387_RESET;
	HAL_I2C_Master_Transmit(&hi2c1,MS5387_addrt,i2ctraBuf,1,10);
	HAL_Delay(10); //wait for reset to complete
	for (uint8_t i=0;i<7;i++){
		i2ctraBuf[0]=MS5387_PROM_READ+i*2;
		HAL_I2C_Master_Transmit(&hi2c1,MS5387_addrt,i2ctraBuf,1,10);
		HAL_I2C_Master_Receive(&hi2c1,MS5387_addrr,&i2crecBuf[0],2,10);
		rawExtPressure[0]=(i2crecBuf[0]<<8)|i2crecBuf[1];

}
}


void ExtPress_read(void){
	i2ctraBuf[0]=MS5387_CONVERT_D1_8192;
	HAL_I2C_Master_Transmit(&hi2c1,MS5387_addrt,i2ctraBuf,1,10);
	HAL_Delay(20);
	i2ctraBuf[0]=MS5387_ADC_READ;
	HAL_I2C_Master_Transmit(&hi2c1,MS5387_addrt,i2ctraBuf,1,10);
	HAL_I2C_Master_Receive(&hi2c1,MS5387_addrr,&i2crecBuf[0],3,10);
	D1=(((i2crecBuf[0]<<16)|(i2crecBuf[1]<<8))|(i2crecBuf[2]));

	i2ctraBuf[1]=MS5387_CONVERT_D2_8192;
	HAL_I2C_Master_Transmit(&hi2c1,MS5387_addrt,i2ctraBuf,1,10);
	HAL_Delay(20);
	i2ctraBuf[0]=MS5387_ADC_READ;
	HAL_I2C_Master_Transmit(&hi2c1,MS5387_addrt,i2ctraBuf,1,10);
	HAL_I2C_Master_Receive(&hi2c1,MS5387_addrr,&i2crecBuf[0],3,10);
	D2=(((i2crecBuf[0]<<16)|(i2crecBuf[1]<<8))|(i2crecBuf[2]));
	internalStats[EXT_PRESS]=D2;
	Ext_Pressure_Cal();
}

void Ext_Pressure_Cal(void){

	int32_t dT = 0;
	int64_t SENS = 0;
	int64_t OFF = 0;
	int32_t SENSi = 0;
	int32_t OFFi = 0;
	int32_t Ti = 0;
	int64_t OFF2 = 0;
	int64_t SENS2 = 0;

	// Terms called
	uint32_t dt= (rawExtPressure[5])*(0x256l);
	dT = D2-dt;

	int32_t sen=(rawExtPressure[1])*(0x32768l);
	int32_t sens=(rawExtPressure[3])*dT/(0x256l);
	SENS = sen+sens;

	int32_t of=(rawExtPressure[2])*(0x65536l);
	int32_t off=(rawExtPressure[4])*dT/(0x128l);
	OFF = of+off;

	P = (D1*SENS/(0x2097152l)-OFF)/(0x8192l);

	// Temp conversion
	TEMP = (0x2000l)+(dT)*rawExtPressure[6]/(0x8388608LL);

	//Second order compensation
		if((TEMP/100)<20){         //Low temp
			Ti = (3*(dT)*(dT))/(0x8589934592LL);
			OFFi = (3*(TEMP-2000)*(TEMP-2000))/2;
			SENSi = (5*(TEMP-2000)*(TEMP-2000))/8;
			if((TEMP/100)<-15){    //Very low temp
				OFFi = OFFi+7*(TEMP+(0x1500l))*(TEMP+(0x1500l));
				SENSi = SENSi+4*(TEMP+(0x1500l))*(TEMP+(0x1500l));
			}
		}
		else if((TEMP/100)>=20){    //High temp
			Ti = 2*(dT*dT)/(0x137438953472LL);
			OFFi = (1*(TEMP-2000)*(TEMP-2000))/16;
			SENSi = 0;
		}


	OFF2 = OFF-OFFi;           //Calculate pressure and temp second order
	SENS2 = SENS-SENSi;

	TEMP = (TEMP-Ti);

		P = (((D1*SENS2)/(0x2097152l)-OFF2)/(0x8192l));
		P = P*0.001f/10.0f;
		internalStats[EXT_PRESS]=P;
}


void set_led(uint16_t color){
	if((color&0b00000001)==1){
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);
	}
	if((color&0b00000010)==1){
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);
	}

	if((color&0b00000100)==1){
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);
	}
	if((color&0b00001000)==1){
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
	}

	if((color&0b00010000)==1){
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_SET);
	}
	if((color&0b00100000)==1){
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);
	}
}

void reset_led(void){
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6,GPIO_PIN_RESET);
}

void sensor_update(void){
	LSM6_read();
	LIS3_read();
	HIH_read();
	IntPressure_read();
	ExtPress_read();
	Ext_Pressure_Cal();
}

void sensor_initialize(void){
	ExtPress_init();
	init_IMU();
	init_LIS3();
	HIH_init();
}
