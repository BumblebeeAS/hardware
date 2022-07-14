#include "BQ34110.h"
#include <Arduino.h>
#include <Wire.h>
#include "defines.h"


uint8_t BQ_dataW[10];
uint8_t BQ_dataR[10];

uint8_t keys[8];
uint32_t avgRawVolt, avgRawCurr;


uint32_t BQ_ReadRegister(uint8_t add, uint8_t len){
    uint32_t returnVal = 0;
    
    Wire.beginTransmission(BQ34110_ADDRESS);
    Wire.write(add);
    Wire.endTransmission();
    Wire.requestFrom(BQ34110_ADDRESS, (int)(len));

    for(int i=0;i<len;i++){
        returnVal = returnVal  | Wire.read() << (8*i);
    }
       //need to add timeout?

    return returnVal;
}

uint16_t BQ_GetBattStatus(){
    return BQ_ReadRegister( BQ34110_REG_BSTAT, 2);
}

/* This function can also be called BQ_WriteSubcommand(uint8_t cntl_data)
 * Writing to Control() or ManufacturerAccessControl() has the same effect
 * Control() will also be sent back to MAC()
 * EXCEPT for CONTROL_STATUS(), which MUST be read back from Control()
 * Fails with HAL_I2C_Mem_Write
 */

void BQ_WriteControl(uint8_t cntl_data){
  BQ_dataW[0] = BQ34110_REG_CNTL;
	BQ_dataW[1] = cntl_data;
	BQ_dataW[2] = 0x00;
    Wire.beginTransmission(BQ34110_ADDRESS);
    Wire.write(BQ_dataW, 3);
    Wire.endTransmission();
}

int BQ_ReadControl(uint8_t cntl_data){
    BQ_WriteControl(cntl_data);
    delay(30);
    return BQ_ReadRegister(BQ34110_REG_CNTL, 2);
}

void BQ_WriteMAC(uint16_t cntl_data){
  BQ_dataW[0] = (uint8_t)BQ34110_REG_MAC;
	BQ_dataW[1] = (uint8_t)(cntl_data & 0xFF);
	BQ_dataW[2] = (uint8_t)(cntl_data >> 8);
   Wire.beginTransmission(BQ34110_ADDRESS);
   Wire.write(BQ_dataW, 3);
   Wire.endTransmission();
}

// Max bytes to be read is 4
uint32_t BQ_ReadMAC(uint16_t cntl_data, int bytes){
    BQ_WriteMAC(cntl_data);
    delay(50);
    return BQ_ReadRegister(BQ34110_REG_MAC_DATA, bytes);
}

// readRegister 2 bytes of Flash Data from address 0 x addr1 addr2
uint32_t BQ_ReadFlash(uint16_t addr, uint8_t bytes){
    uint16_t flashDataR = 0x0000;
    uint16_t MAC = 0x0000;

    do{
        BQ_WriteMAC(addr);
        delay(50);
        // Check if MAC contains the correct subcommand
        Wire.beginTransmission(BQ34110_ADDRESS);
        Wire.write(0x3E);
        Wire.endTransmission();
//        Wire.beginTransmission(BQ34110_ADDRESS+1);
        Wire.requestFrom(BQ34110_ADDRESS, 1);
        MAC = Wire.read();
//        Wire.endTransmission();
     
        Wire.beginTransmission(BQ34110_ADDRESS);
        Wire.write(0x3F);
        Wire.endTransmission();
       // Wire.beginTransmission(BQ34110_ADDRESS+1);
        Wire.requestFrom(BQ34110_ADDRESS, 1);
        MAC = MAC | Wire.read()<< 8;
        Serial.println(MAC , HEX);
        //Wire.endTransmission();
        delay(500);
    }while (MAC != addr);

    Wire.beginTransmission(BQ34110_ADDRESS);
    Wire.write(BQ34110_REG_MAC_DATA);
    Wire.endTransmission();
    Wire.requestFrom(BQ34110_ADDRESS, (int)(bytes));
    for(int i=0;i<bytes;i++){
        flashDataR = flashDataR << 8 | Wire.read();  // Bytes received in Little Endian
    }
    
    BQ_ReadRegister(BQ34110_REG_MAC_DATA_SUM, 1);
	  BQ_ReadRegister(BQ34110_REG_MAC_DATA_LEN, 1);

    return flashDataR;
}

// bytes : num of bytes to be written = {1,2}
void BQ_WriteFlash(uint16_t addr, uint8_t bytes, uint16_t flashDataW){
	// Send DF address in Little Endian format
    uint16_t MAC = 0x0000;

	uint16_t dataCheck = 0x0000;
	uint16_t dataWritten;
    if (bytes==1){
        dataWritten = (uint8_t)(flashDataW & 0xFF);
    }
	else if (bytes==2) {
        dataWritten = flashDataW;
    }
	uint16_t dataFlashAddr = addr;

	BQ_WriteMAC(addr);
	delay(50);

	// Sent in big endian
    Wire.beginTransmission(BQ34110_ADDRESS);
    Wire.write(BQ34110_REG_MAC_DATA);

	if (bytes == 2) {
        Wire.write((uint8_t)(flashDataW >> 8));
        Wire.write((uint8_t)(flashDataW & 0xFF));
	}
	else if (bytes == 1) {
        Wire.write((uint8_t)(flashDataW & 0xFF));
	}
    Wire.endTransmission();
    delay(300);

    BQ_CheckSum(addr, bytes, flashDataW);
    BQ_dataW[0] = BQ34110_REG_MAC_DATA_LEN;
	BQ_dataW[1] = 0x04 + bytes;

    Wire.beginTransmission(BQ34110_ADDRESS);
    Wire.write(BQ_dataW,2);
    Wire.endTransmission();
    delay(30);
}

void BQ_WriteCCFlash(uint16_t addr, int rawData[]){
	// Send DF address in Little Endian format
	uint16_t MAC = 0x0000;

	uint16_t dataCheck = 0x0000;
	uint16_t dataWritten;

	BQ_WriteMAC(addr);
	delay(50);

	// Sent in big endian   
  BQ_dataW[0] = BQ34110_REG_MAC_DATA;
	BQ_dataW[1] = (uint8_t)(rawData[0]);
	BQ_dataW[2] = (uint8_t)(rawData[1]);
	BQ_dataW[3] = (uint8_t)(rawData[2]);
	BQ_dataW[4] = (uint8_t)(rawData[3]);
    Wire.beginTransmission(BQ34110_ADDRESS);
    Wire.write(BQ_dataW,5);
    Wire.endTransmission();
    delay(300);

    uint16_t summ = 0;
    summ = ~ ( (uint8_t)(addr >> 8) + (uint8_t)(addr & 0xFF) +
    		   (uint8_t)(rawData[0]) + (uint8_t)(rawData[1]) +
			   (uint8_t)(rawData[2]) + (uint8_t)(rawData[3]) );
    BQ_dataW[0] = BQ34110_REG_MAC_DATA_SUM;
    BQ_dataW[1] = summ;
    Wire.beginTransmission(BQ34110_ADDRESS);
    Wire.write(BQ_dataW,2);
    Wire.endTransmission();
    delay(500);

	BQ_dataW[0] = BQ34110_REG_MAC_DATA_LEN;
	BQ_dataW[1] = 0x04 + 0x04;
    Wire.beginTransmission(BQ34110_ADDRESS);
    Wire.write(BQ_dataW,2);
    Wire.endTransmission();
    delay(30);
}

//This calcs the checksum and then writes it to the device
//This then causes the device to check it, and if correct
//It will then store the new data in flash
void BQ_CheckSum(uint16_t addr, uint8_t bytes, uint16_t flashDataW) {
    uint16_t summ = 0;
    summ = ~ ( (uint8_t)(addr >> 8) + (uint8_t)(addr & 0xFF) +
    		   (uint8_t)(flashDataW >> 8) + (uint8_t)(flashDataW & 0xFF) );
    BQ_dataW[0] = BQ34110_REG_MAC_DATA_SUM;
    BQ_dataW[1] = summ;
    Wire.beginTransmission(BQ34110_ADDRESS);
    Wire.write(BQ_dataW,2);
    Wire.endTransmission();
    delay(1000);
}

//Temperature() 0x06 and 0x07 0.1°K R
float BQ_GetTemp() {
    return BQ_ReadRegister(BQ34110_REG_TEMP, 2)*0.1 - 273;
}

void BQ_CalibrateVoltage(uint16_t vApplied) {
	uint16_t wait = 200;
	uint8_t loopCount = 0;
	uint8_t samplesToAvg = 50;
	uint32_t rawDataSum = 0;
	double avgRawVoltage = 0;
	uint32_t data;

	uint16_t counterNow, counterPrev;
	counterNow = 0x79;

	// Obtain Raw Calibration Data
	BQ_EnterCalibration();

	while (loopCount < samplesToAvg) {
		delay(wait);
		rawDataSum += BQ_GetVoltage();
		loopCount++;
	}

	BQ_ExitCalibration();
	avgRawVoltage = rawDataSum/samplesToAvg;
	// ----------------------------

//	Round new_divider as needed to an unsigned 16- bit value.
//	This value cannot exceed 65535.
	uint32_t curr_divider = BQ_ReadFlash(BQ34110_DF_VOLTAGE_DIVIDER, 2);
	uint16_t new_divider = (uint16_t)(vApplied * curr_divider / avgRawVoltage);
  Serial.print("divider:");
  Serial.println(new_divider);
  Serial.print("avgRawVoltage: ");
  Serial.println(avgRawVoltage);

	//	int vOffset = (uint8_t)(vApplied - avgRawVoltage);
	int vOffset;
	if (vApplied - avgRawVoltage > 127) vOffset = 127;
	else if (vApplied - avgRawVoltage < -128) vOffset = -128;
	else vOffset = (vApplied - avgRawVoltage);

	do {
		BQ_WriteFlash(BQ34110_DF_VOLTAGE_DIVIDER, 0x02, new_divider);
		data = BQ_ReadFlash(BQ34110_DF_VOLTAGE_DIVIDER, 2);
	} while (data != new_divider);

	BQ_WriteMAC(0x400F);
	delay(50);

	// Sent in big endian
	BQ_dataW[0] = BQ34110_REG_MAC_DATA;
	BQ_dataW[1] = vOffset;
    Wire.beginTransmission(BQ34110_ADDRESS);
    Wire.write(BQ_dataW,2);
    Wire.endTransmission();
    delay(300);

    BQ_CheckSum(0x400F, 1, vOffset);

	BQ_dataW[0] = BQ34110_REG_MAC_DATA_LEN;
	BQ_dataW[1] = 0x05;
    Wire.beginTransmission(BQ34110_ADDRESS);
    Wire.write(BQ_dataW,2);
    Wire.endTransmission();
    delay(30);
}

void BQ_RestoreCCSettings() {
	int ccGain_df[4], ccDelta_df[4];
	uint32_t data1, data2;
	for (int i = 0; i<4; i++) {
		ccGain_df[i] = 0;
		ccDelta_df[i] = 0;
	}

	floatConversion(4.7095, ccGain_df);
	floatConversion(5.677446*pow(10,5), ccDelta_df);

	BQ_EnterCalibration();

	do {
		BQ_WriteCCFlash(BQ34110_DF_CC_GAIN, ccGain_df);
   Serial.print("ccGain_df: ");
   Serial.println((uint32_t)(ccGain_df[0] << 8 | ccGain_df[1]));
   Serial.println((uint32_t)(ccGain_df[2] << 8 | ccGain_df[3]));
   
		data1 = BQ_ReadFlash(BQ34110_DF_CC_GAIN, 2);
		data2 = BQ_ReadFlash(BQ34110_DF_CC_GAIN + 2, 2);
	} while (data1 != (ccGain_df[0] << 8 | ccGain_df[1]) &&
			 data2 != (ccGain_df[2] << 8 | ccGain_df[3]));

	BQ_WriteCCFlash(BQ34110_DF_CC_DELTA, ccDelta_df);

	BQ_ExitCalibration();

	delay(300);
}

void BQ_CalibrateCurrent(float forcedLoadCurrent) {
	uint16_t wait = 200;
	uint8_t loopCount = 0;
	uint8_t samplesToAvg = 10;
	uint32_t rawDataSum = 0;
	double avgRawCurrent = 0;
	int16_t ccOffset;
	int8_t boardOffset;
	uint32_t data1, data2, data;

	float ccGain, ccDelta;
	int ccGain_df[4], ccDelta_df[4];

	for (int i = 0; i<4; i++) {
		ccGain_df[i] = 0;
		ccDelta_df[i] = 0;
	}

	ccOffset = BQ_ReadFlash(BQ34110_DF_CC_OFFSET, 2);
	boardOffset = BQ_ReadFlash(BQ34110_DF_BOARD_OFFSET, 1);

	ccGain = BQ_ReadFlash(BQ34110_DF_CC_GAIN, 2);

	uint16_t counterNow, counterPrev;
	BQ_EnterCalibration();

	counterNow = BQ_AnalogCount();
	counterPrev = counterNow;

	for (int loopCount = 0; loopCount < samplesToAvg; ) {
		if (counterNow != counterPrev) {
			rawDataSum += BQ_GetCurrent();
			loopCount++;
			counterPrev = counterNow;
		}
		else {
			delay(wait);
			counterNow = BQ_AnalogCount();
		}
	}

	BQ_ExitCalibration();

	avgRawCurrent = rawDataSum/samplesToAvg;

	ccGain = 0.47095*(forcedLoadCurrent/(float)((int)avgRawCurrent-(ccOffset+boardOffset)/16)); //* pow(2, 12);

	ccDelta = (ccGain * 1193046)/(5.677446*pow(10,5));
	floatConversion(ccGain, ccGain_df);
	floatConversion(ccDelta, ccDelta_df);

	// Write to DF
	do {
		BQ_WriteCCFlash(BQ34110_DF_CC_GAIN, ccGain_df);
		data1 = BQ_ReadFlash(BQ34110_DF_CC_GAIN, 2);
		data2 = BQ_ReadFlash(BQ34110_DF_CC_GAIN + 2, 2);
	} while (data1 != (ccGain_df[0] << 8 | ccGain_df[1]) &&
			 data2 != (ccGain_df[2] << 8 | ccGain_df[3]));

	do {
		BQ_WriteCCFlash(BQ34110_DF_CC_DELTA, ccDelta_df);
		data1 = BQ_ReadFlash(BQ34110_DF_CC_DELTA, 2);
		data2 = BQ_ReadFlash(BQ34110_DF_CC_DELTA + 2, 2);
	} while (data1 != (ccDelta_df[0] << 8 | ccDelta_df[1]) &&
			 data2 != (ccDelta_df[2] << 8 | ccDelta_df[3]));

	delay(300);

	data = BQ_GetCurrent();

//	if (data == 0) {
//		do {
//			ccGain = ccGain*10;
//			ccDelta = ccGain * 1193046;
//			floatConversion(ccGain, ccGain_df);
//			floatConversion(ccDelta, ccDelta_df);
//
//			// Write to DF
//			do {
//				BQ_WriteCCFlash(BQ34110_DF_CC_GAIN, ccGain_df);
//				data1 = BQ_ReadFlash(BQ34110_DF_CC_GAIN, 2);
//				data2 = BQ_ReadFlash(BQ34110_DF_CC_GAIN + 2, 2);
//			} while (data1 != (ccGain_df[0] << 8 | ccGain_df[1]) &&
//					 data2 != (ccGain_df[2] << 8 | ccGain_df[3]));
//
//			do {
//				BQ_WriteCCFlash(BQ34110_DF_CC_DELTA, ccDelta_df);
//				data1 = BQ_ReadFlash(BQ34110_DF_CC_DELTA, 2);
//				data2 = BQ_ReadFlash(BQ34110_DF_CC_DELTA + 2, 2);
//			} while (data1 != (ccDelta_df[0] << 8 | ccDelta_df[1]) &&
//					 data2 != (ccDelta_df[2] << 8 | ccDelta_df[3]));
//
//			data = BQ_GetCurrent();
//		} while ( data == 0 );
//	}
//	if (( data > 1.02*forcedLoadCurrent || data < 0.98*forcedLoadCurrent )) {
//		do {
//			ccGain = ccGain*(forcedLoadCurrent/data);
//			ccDelta = ccGain * 1193046;
//			floatConversion(ccGain, ccGain_df);
//			floatConversion(ccDelta, ccDelta_df);
//
//			// Write to DF
//			do {
//				BQ_WriteCCFlash(BQ34110_DF_CC_GAIN, ccGain_df);
//				data1 = BQ_ReadFlash(BQ34110_DF_CC_GAIN, 2);
//				data2 = BQ_ReadFlash(BQ34110_DF_CC_GAIN + 2, 2);
//			} while (data1 != (ccGain_df[0] << 8 | ccGain_df[1]) &&
//					 data2 != (ccGain_df[2] << 8 | ccGain_df[3]));
//
//			do {
//				BQ_WriteCCFlash(BQ34110_DF_CC_DELTA, ccDelta_df);
//				data1 = BQ_ReadFlash(BQ34110_DF_CC_DELTA, 2);
//				data2 = BQ_ReadFlash(BQ34110_DF_CC_DELTA + 2, 2);
//			} while (data1 != (ccDelta_df[0] << 8 | ccDelta_df[1]) &&
//					 data2 != (ccDelta_df[2] << 8 | ccDelta_df[3]));
//
//			data = BQ_GetCurrent();
//		} while ( data > 1.02*forcedLoadCurrent || data < 0.98*forcedLoadCurrent );
//	}
}

void floatConversion(float val, int* data) {
	float tmpVal, mod_val;
	uint16_t byte2, byte1, byte0;
	int exp;

	exp = 0;

	if (val < 0) mod_val = val * (-1);
	else mod_val = val;

	tmpVal = mod_val;
	tmpVal *= (1 + pow(2, -25));

	if (tmpVal < 0.5) {
		while (tmpVal < 0.5) {
			tmpVal *= 2;
			exp--;
		}
	}
	else if (tmpVal >= 1.0) {
		while (tmpVal >= 1.0) {
			tmpVal /= 2;
			exp++;
		}
	}

	if (exp > 127) exp = 127;
	else if (exp < -128) exp = -128;

	tmpVal = pow(2, 8 - exp) * mod_val - 128;
	byte2 = (int)tmpVal;
	tmpVal = pow (2, 8) * (tmpVal - byte2);
	byte1 = (int)tmpVal;
	tmpVal = pow(2, 8) * (tmpVal - byte1);
	byte0 = (int)tmpVal;

	if (val < 0) {
		byte2 |= 0x80;
	}

	data[0] = exp + 128;
	data[1] = byte2;
	data[2] = byte1;
	data[3] = byte0;
}

//RelativeStateOfCharge() 0x2C and 0x2D % R
uint8_t BQ_GetRSOC() {
    return BQ_ReadRegister(BQ34110_REG_RSOC, 2);
}

//StateOfHealth() 0x2E and 0x2F % R
uint8_t BQ_GetSOH() {
	return BQ_ReadRegister(BQ34110_REG_SOH, 2);
}

//RawVoltage() 0x7C and 0x7D mV R
int BQ_GetVoltage() {
    return BQ_ReadRegister(BQ34110_REG_VOLT, 2);
}

int BQ_GetRawVoltage() {
	return BQ_ReadRegister(BQ34110_REG_RAW_VOLT, 2);
}

//FullChargeCapacity() 0x12 and 0x13 mAh R
int BQ_GetFullCapacity() {
    return BQ_ReadRegister(BQ34110_REG_FCC, 2);
}

//RemainingCapacity() RC 0x10 and 0x11 mAh R
int BQ_GetRemainingCapacity() {
    return BQ_ReadRegister(BQ34110_REG_RC, 2) ;
}

//Current() 0x0C and 0x0D mA R
float BQ_GetCurrent() {
    return BQ_ReadRegister(BQ34110_REG_CURR, 2) ; //return temp in x10 format
}

int BQ_GetRawCurrent() {
	return BQ_ReadRegister(BQ34110_REG_RAW_CURR, 2);
}

int BQ_AnalogCount() {
	return BQ_ReadRegister(0x79, 1);
}

void BQ_Reset() {
	BQ_WriteControl(BQ34110_CNTL_RESET);

    while (!(BQ_ReadRegister(0x3B, 1) & (1<<1))) {
    	delay(1000);

	}
}

void BQ_Init() {
	uint32_t batt_capacity, num_of_cells, voltage_divider;
	batt_capacity = 16000;
	num_of_cells = 4;
	if (PMB_NO == 1) {
		voltage_divider = 19000;
	}
	else {
		voltage_divider = 18760;
	}

	BQ_Reset();
 Serial.println("reset");
//	BQ_WriteControl(BQ34110_CNTL_PIN_VEN_RESET);
	BQ_EnterCalibration();
   Serial.println("BQ calib");
//	BQ_SetFlashUOV();
	// Disable built in voltage divider
	BQ_SetPinCntlConfig();
	BQ_CalibrateNumOfCells();
  Serial.println("Cali cells");
	BQ_SetVoltageDivider(voltage_divider);
  Serial.println("volt div set");
//	BQ_SetMaxPackV();
//	BQ_SetMinPackV();
	BQ_SetDesignCap(batt_capacity); // Set Capacity to 16000 mAh
  Serial.println("batt cap set");
	BQ_SetLFCC(batt_capacity);	   // Set Learned Full Charge Capacity at 12000 mAh for first test without learnign
  Serial.println("LFCC set");
	BQ_CEDVConfig();
  Serial.println("CEDVConfig");
	BQ_ExitCalibration(); // including reset device
  Serial.println("exit cali");

	BQ_Calibrate_CCOffset_BoardOffset();
  Serial.println("CC offset calibrated");
}

//  0x4157 Flash Update OK Voltage I2 0 5000 2800 mV
void BQ_SetFlashUOV() {
	int32_t flashUOV = 0;
//	BQ_WriteFlash(BQ34110_DF_FLASH_UOV, 0x02, 0x0AF0);
	BQ_WriteFlash(BQ34110_DF_FLASH_UOV, 0x02, 0x0064);

	flashUOV = BQ_ReadFlash(BQ34110_DF_FLASH_UOV, 2);
	delay(10);
}

//  0x413D Pin Control Config H1 0x0 0x1F 0x00 Hex
void BQ_SetPinCntlConfig() {
	uint32_t pinCntlConfig, mfgStat;

	//
	do {
		mfgStat = BQ_ReadFlash(BQ34110_DF_MFG_STATUS_INIT, 2);
		BQ_WriteFlash(BQ34110_DF_MFG_STATUS_INIT, 0x02, mfgStat | (1 << 4));

		pinCntlConfig = BQ_ReadFlash(BQ34110_DF_PIN_CNTL_CONFIG, 1);
		BQ_WriteFlash(BQ34110_DF_PIN_CNTL_CONFIG, 0x01, pinCntlConfig | (1 << 4));

		pinCntlConfig = BQ_ReadFlash(BQ34110_DF_PIN_CNTL_CONFIG, 1) & (1<<4);
	} while (pinCntlConfig != 0x10);
}

void BQ_SetLFCC(uint32_t LFCC) {
	uint32_t curr_LFCC = 0;
	curr_LFCC = BQ_ReadFlash(BQ34110_DF_LFCC, 2);
	if (curr_LFCC != LFCC) {
		do {
			BQ_WriteFlash(BQ34110_DF_LFCC, 0x02, LFCC);
			curr_LFCC =  BQ_ReadFlash(BQ34110_DF_LFCC, 2);
		} while (curr_LFCC != LFCC);
	}
}

//  0x4155 Number of Series Cells U1 1 100 1 Num
void BQ_CalibrateNumOfCells() {
	uint32_t numOfCells = 0;
  //Serial.print("Read flash");
	numOfCells = BQ_ReadFlash(BQ34110_DF_SERIES_CELLS, 1);
 // Serial.print("num of cells");
  //Serial.println(numOfCells);

	if (numOfCells != 4) {
		do {
     // Serial.print("num of cells");
     // Serial.println(numOfCells);
			BQ_WriteFlash(BQ34110_DF_SERIES_CELLS, 0x01, 4);
			numOfCells = BQ_ReadFlash(BQ34110_DF_SERIES_CELLS, 1);
		} while (numOfCells != 4);
	}
//	BQ_WriteFlash(BQ34110_DF_SERIES_CELLS, 0x01, 0x0001);
//	BQ_WriteFlash(BQ34110_DF_SERIES_CELLS, 0x01, 0x0004);

//	BQ_WriteMAC(BQ34110_DF_SERIES_CELLS);
//	HAL_Delay(50);
//
//	// Sent in big endian
//	BQ_dataW[0] = BQ34110_REG_MAC_DATA;
//	BQ_dataW[1] = (uint8_t)(0x04);
//	HAL_I2C_Master_Transmit(&hi2c1, BQ34110_ADDRESS, BQ_dataW, 2, HAL_MAX_DELAY);
//	HAL_Delay(300);
//
//	BQ_CheckSum(BQ34110_DF_SERIES_CELLS, 1, 0x04);
//
//	BQ_dataW[0] = BQ34110_REG_MAC_DATA_LEN;
//	BQ_dataW[1] = 0x05;
//	HAL_I2C_Master_Transmit(&hi2c1, BQ34110_ADDRESS, BQ_dataW, 2, 10);
//	HAL_Delay(30);
//
//	numOfCells = BQ_ReadFlash(BQ34110_DF_SERIES_CELLS, 1);
}
//
////  0x4010 Voltage Divider U2 0 65535 5000 mV



void BQ_SetVoltageDivider(uint32_t voltage_divider) {
	uint32_t curr_voltage_divider;
	curr_voltage_divider = BQ_ReadFlash(BQ34110_DF_VOLTAGE_DIVIDER, 2);

	if (curr_voltage_divider != voltage_divider) {
		do {
			BQ_WriteFlash(BQ34110_DF_VOLTAGE_DIVIDER, 0x02, voltage_divider);
			curr_voltage_divider = BQ_ReadFlash(BQ34110_DF_VOLTAGE_DIVIDER, 2);
		} while (curr_voltage_divider != voltage_divider);
	}
}

//  0x4088 Max Pack Voltage I2 0 32767 160 20 mV
void BQ_SetMaxPackV() {
	uint32_t maxPackV;

	BQ_WriteFlash(BQ34110_DF_MAX_PACK_V, 0x02, 0x00A0);
//	BQ_WriteFlash(BQ34110_DF_MAX_PACK_V, 0x02, 0x0348);

	maxPackV = BQ_ReadFlash(BQ34110_DF_MAX_PACK_V, 2);
}

//  0x408A Min Pack Voltage I2 0 32767 175 20 mV
void BQ_SetMinPackV() {
	uint32_t minPackV;

	BQ_WriteFlash(BQ34110_DF_MIN_PACK_V, 0x02, 0x00AF);
//	BQ_WriteFlash(BQ34110_DF_MIN_PACK_V, 0x02, 0x02E6);

	minPackV = BQ_ReadFlash(BQ34110_DF_MIN_PACK_V, 2);
}

//  0x41F5 Design Capacity mAh I2 0 32767 2200 mAh
void BQ_SetDesignCap(uint32_t designCap) {
	uint32_t flashDesignCap;

	//BQ_WriteFlash(BQ34110_DF_DESIGN_CAP, 0x02, 0x0898);
	do {
		 BQ_WriteFlash(BQ34110_DF_DESIGN_CAP, 0x02, designCap);
    
		flashDesignCap = BQ_ReadFlash(BQ34110_DF_DESIGN_CAP, 2);

    //Serial.println(flashDesignCap);
	} while (flashDesignCap != designCap);
}

void BQ_EnterCalibration() {
	uint32_t buffer;
	do {
		BQ_WriteControl(BQ34110_CNTL_CAL_TOGGLE);
    //Serial.println("write control");
		delay(300);
		
		buffer = BQ_ReadMAC(BQ34110_CNTL_MANUF_STATUS, 2);
    //Serial.println(buffer);
	} while (!((buffer >> 8) & (1 << 7)));
}

void BQ_ExitCalibration() {
	uint32_t buffer;
	do {
		BQ_WriteControl(BQ34110_CNTL_CAL_TOGGLE);
		delay(300);
		
		BQ_dataW[0] = 0x00;
		BQ_dataW[1] = 0x57;
		BQ_dataW[2] = 0x00;
        Wire.beginTransmission(BQ34110_ADDRESS);
        Wire.write(BQ_dataW,3);
        Wire.endTransmission();

        delay(300);

        Wire.beginTransmission(BQ34110_ADDRESS);
        Wire.write(0x40);
        Wire.endTransmission();
        Wire.requestFrom(BQ34110_ADDRESS, 2);
        buffer = Wire.read(); 
        buffer = buffer | Wire.read() << 8 ;
        //		buffer = BQ_ReadMAC(BQ34110_CNTL_MANUF_STATUS, 2);
	} while ( ((buffer >> 8) & (1 << 7)));
	
	if ((buffer >> 8) & (1 << 7)) {}
	else {}
}

void BQ_Calibrate_CCOffset_BoardOffset() {
	BQ_EnterCalibration();
	
	uint32_t buffer;

	do {
		BQ_WriteControl(BQ34110_CNTL_BOARD_OFFSET);
		delay(10);
		
		buffer = BQ_ReadControl(BQ34110_CNTL_CONTROL_STATUS);
	} while ( !(buffer & (1 << 5)) || !(buffer & (1 << 4)) );

	do {
        Wire.beginTransmission(BQ34110_ADDRESS);
        Wire.write(0x00);
        Wire.endTransmission();
        Wire.requestFrom(BQ34110_ADDRESS, 2);
        buffer = Wire.read(); 
        buffer = buffer | Wire.read() << 8 ;
        delay(1000);

    }while ( (buffer & (1 << 5)) || (buffer & (1 << 4)) );

    BQ_WriteControl(BQ34110_CNTL_CC_OFFSET_SAVE);

	BQ_ExitCalibration();
}


void BQ_CEDVConfig() {
	uint32_t data = 0;
	int16_t curr_voltageDOD[11];
	int16_t voltageDOD[11] = {4136, 4018, 3922, 3845, 3775, 3730, 3716, 3697, 3614, 3467, 2759};
	//int16_t voltageDOD[11] = {4173, 4043, 3925, 3821, 3725, 3656, 3619, 3582, 3515, 3438, 2713};
	// int16_t fixedEDV[3] = { 1768, 1975, 2042 };
	//int16_t fixedEDV[3] = {3031, 3385, 3501};
	int16_t fixedEDV[3] = {3738, 4175, 4318};

//Set CEDV Gauging Config
//	BQ_WriteFlash(BQ34110_DF_CEDV_CONFIG, 0x01, (1 << FIXED_EDV0_BIT));
//	data = BQ_ReadFlash(BQ34110_DF_DESIGN_VOLTAGE, 2);
//	HAL_Delay(10);

	//Set Design Voltage to 3700

	do {
		BQ_WriteFlash(BQ34110_DF_DESIGN_VOLTAGE, 0x02, 3700);
		data = BQ_ReadFlash(BQ34110_DF_DESIGN_VOLTAGE, 2);
    
	} while (data != 3700);

	//Set SOC Low Threshold to 25%
	do {
//		BQ_WriteFlash(BQ34110_DF_SOC_LOW_TH, 0x01, 10);
   // Serial.println("10 flash");
		BQ_WriteFlash(BQ34110_DF_SOC_LOW_TH, 0x01, 25);
  // Serial.println("25 flash");
		data = BQ_ReadFlash(BQ34110_DF_SOC_LOW_TH, 1);
    
	} while (data != 25);


	//Set SOC Low Recovery to 35%
	do {
		BQ_WriteFlash(BQ34110_DF_SOC_LOW_RECOV, 0x01, 35);
		data = BQ_ReadFlash(BQ34110_DF_SOC_LOW_RECOV, 1);
    
	} while (data != 35);

	//Set Voltage (0-100) % DOD
	for (int i = 0; i < 11; i++) {
		curr_voltageDOD[i] = BQ_ReadFlash(BQ34110_DF_VOLTAGE_ZERO_DOD + 2*i, 2);
		do {
			BQ_WriteFlash(BQ34110_DF_VOLTAGE_ZERO_DOD + 2*i, 0x02, voltageDOD[i]);
			data = BQ_ReadFlash(BQ34110_DF_VOLTAGE_ZERO_DOD + 2*i, 2);
      //Serial.println(data);
      //Serial.println(i);
		} while (data != voltageDOD[i]);
	}

	//Set Fixed EDV 0, 1, 2
	for (int i = 0; i < 3; i++) {
		do {
			BQ_WriteFlash(BQ34110_DF_FIXED_EDV0 + 3*i, 0x02, fixedEDV[i]);
			data = BQ_ReadFlash(BQ34110_DF_FIXED_EDV0 + 3*i, 2);
      //Serial.println(data);
     // Serial.println(i);
		} while (data != fixedEDV[i]);
	}

	// Battery Low Set Threshold
	do {
		BQ_WriteFlash(0x4184, 0x02, 1838);
		data = BQ_ReadFlash(0x4184, 2);
	} while (data != 1838);

	// Battery Low Clear Threshold
	do {
		BQ_WriteFlash(0x4187, 0x02, 1938);
		data = BQ_ReadFlash(0x4187, 2);
	} while (data != 1938);

	// Num Of Cells
//	do {
//		BQ_WriteFlash(BQ34110_DF_SERIES_CELLS, 1, 4);
//		data = BQ_ReadFlash(BQ34110_DF_SERIES_CELLS, 1);
//	} while (data != 4);


	// Operation Config A
	do {
		BQ_WriteFlash(0x413A, 0x02, 0x8200);
		data = BQ_ReadFlash(0x413A, 2);
	} while (data != 0x8200);


}
    
void BQ_Learning() {
	uint16_t opStatus, FCC, LFCC, RC;
	uint8_t VDQbit, EDV2bit;
//	do {
//		opStatus = BQ_ReadRegister(BQ34110_REG_OP_STATUS, 2);
//		VDQbit = opStatus & (1 << OP_STATUS_VDQ_BIT);
//	} while ( VDQbit == 0);

	do {
		opStatus = BQ_ReadRegister(BQ34110_REG_OP_STATUS, 2);
		VDQbit = opStatus & (1 << OP_STATUS_VDQ_BIT);
		EDV2bit = opStatus & (1 << OP_STATUS_EDV2_BIT);
		FCC = BQ_ReadRegister(BQ34110_REG_FCC, 2);
		LFCC = BQ_ReadFlash(BQ34110_DF_LFCC, 2);
		RC = BQ_ReadRegister(BQ34110_REG_RC,2);
	} while (VDQbit == 1 && EDV2bit == 0);
}

void BQ_ReadKeys() {
	BQ_WriteControl(BQ34110_CNTL_SECURITY_KEYS);
	delay(10);

    Wire.beginTransmission(BQ34110_ADDRESS);
    Wire.write(BQ34110_REG_MAC_DATA);
    Wire.endTransmission();
    Wire.requestFrom(BQ34110_ADDRESS+1, 8);
	for (int i = 0; i < 8; i++)
	{
		keys[i] = Wire.read();
	}
    delay(1000);

}

void BQ_Unseal() {
	uint8_t status;

	status = BQ_ReadRegister(BQ34110_REG_OP_STATUS, 1) & 0x06;
//	if (status == 0x04) printf("Unsealed Initial Status");
//	else if (status == 0x06) printf("Sealed Initial Status");
//	else if (status == 0x02) printf("Full Access Initial Status");
//	else if (status == 0x00) printf("Invalid Initial Status");

	BQ_ReadKeys();

	BQ_dataW[0] = BQ34110_REG_CNTL;
	BQ_dataW[1] = keys[0];
	BQ_dataW[2] = keys[1];
    Wire.beginTransmission(BQ34110_ADDRESS);
    Wire.write(BQ_dataW,3);
    Wire.endTransmission();
    delay(10);

    BQ_dataW[0] = BQ34110_REG_CNTL;
	BQ_dataW[1] = keys[2];
	BQ_dataW[2] = keys[3];
    Wire.beginTransmission(BQ34110_ADDRESS);
    Wire.write(BQ_dataW,3);
    Wire.endTransmission();
    delay(10);

    status = BQ_ReadRegister(BQ34110_REG_OP_STATUS, 1) & 0x06;
//	if (status == 0x04) printf("Unsealed Status");
//	else if (status == 0x06) printf("Sealed Status, Failed to Unseal");
//	else if (status == 0x02) printf("Full Access Status, No Need to Unseal");
//	else if (status == 0x00) printf("Invalid Status, No State was Loaded");
}
