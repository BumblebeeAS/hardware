
// implementation for the PMB class

#include "PMB.h"

PMB::PMB()
	:ADS(ADDR_ADC)
	,TempSensor(CONFIG_TEMP_SENS, ADDR_TEMP_SENS)
	,CAN(PIN_CAN_SS)
	,EEPROM(ADDR_EEPROM0, ADDR_EEPROM1)
	,display(PIN_OLED_RESET)
	{}

void PMB::init(){
	
	/*-----MCU Initialisation-----*/

	//Set reference voltage to AREF pin
	analogReference(EXTERNAL);
	//init Digital Pins
	pinMode(PIN_POWEROFF_SIGNAL, INPUT);
	pinMode(PIN_PMB_POWER, OUTPUT);
	pinMode(PIN_VEHICLE_POWER, OUTPUT);
	
	/*-----END MCU Initialisation-----*/

	/*-----Peripherals Initialisation-----*/

	//init devices
	display.init();
 
	// ADS.init();
	ADS.begin();
	TempSensor.initTempAD7414();
	CAN_init();
	EEPROM.init();

	/*-----END Peripherals Initialisation-----*/

	getCanId();

	//init current sensing
	//do multiple times to initialise the array
	for (uint8_t i = 0; i < MEDIAN_FILTER_SIZE; ++i){
		getShuntCurrent();
    cell6_raw_array[cell6_raw_index] = analogRead(PIN_CELL6);
    cell6_raw_index = (cell6_raw_index+1) % MEDIAN_FILTER_SIZE;
		delay(10);
	}
 
	// Serial.println(shunt_current);

	// TODO: Implement check for random shutdown (to determine get cap from eeprom)
	getCapFromVolt();

	//read other values so they are initialised
	readCellVoltages();
  readPressure();
	readTemperature();

	//finally turn power to the vehicle on
	powerUpVehicle();
}

void PMB::getCanId(){
	
	//Hardcoded way to get PMB CAN ID
	if(PMB_no%2){
		ID_CAN_PMB_stats[0] = CAN_PMB1_stats;
		ID_CAN_PMB_stats[1] = CAN_PMB1_stats2;
		ID_CAN_PMB_stats[2] = CAN_PMB1_stats3;
		ID_CAN_PMB_BUS_stats = CAN_PMB1_BUS_stats;
		ID_CAN_HB = HEARTBEAT_PMB1;
	}else{		
		ID_CAN_PMB_stats[0] = CAN_PMB2_stats;
		ID_CAN_PMB_stats[1] = CAN_PMB2_stats2;
		ID_CAN_PMB_stats[2] = CAN_PMB2_stats3;
		ID_CAN_PMB_BUS_stats = CAN_PMB2_BUS_stats;
		ID_CAN_HB = HEARTBEAT_PMB2;
	}

	// TODO: Automatic way to get PMB CAN ID
	// uint8_t n = 0;
	// while(n<20){
	// 	CAN_checkForPmb1();
	// 	n++;
	// 	delay(100);
	// }

}

void PMB::getCapFromVolt(){
	Serial.println("getting from volt!");
	float tempBattVoltage = 0;
	// Take average of 10 (milli) voltage readings in volt
	for (uint8_t i = 0; i < 10; ++i){
		tempBattVoltage += (analogRead(PIN_CELL6) * cell6_adc_ratio) + cell6_adc_offset;
	}
	tempBattVoltage = tempBattVoltage/10.0/1000.0;
	//Serial.println(tempBattVoltage);

	// Calculate % of battery left from calibrated data
	float tempPercent = pow(tempBattVoltage,4)*coef_a + pow(tempBattVoltage,3)*coef_b + pow(tempBattVoltage,2)*coef_c + tempBattVoltage*coef_d + coef_e;
	//Serial.println(tempPercent);
	
	// Limiting range of percentage
	if(tempPercent < 0 ){
		percentage_left = 0.0;
	}else if(tempPercent >  100){
		percentage_left = 100.0;
	}else{
		percentage_left = int(tempPercent);
	}

	//Serial.println(percentage_left);
	//Serial.println(capacity_left);
	capacity_left = percentage_left / 100.0 * BATTERY_CAPACITY;
}

void PMB::getCapFromStorage(){
	// uint8_t* buf;
	// EEPROM.dumbMultiRead(10, buf);
	// capacity_left = uint16_t(CAN.parseCANFrame(buf, 0, 2));
}

void PMB::readCellVoltages(){

  cell6_raw_array[cell6_raw_index] = analogRead(PIN_CELL6);
  //reading = cell6_raw_array[cell6_raw_index];
  cell6_raw_index = (cell6_raw_index+1) % MEDIAN_FILTER_SIZE;
  cell_voltage[5] = ((mean(cell6_raw_array) * cell6_adc_ratio) + cell6_adc_offset);
  
  //cell_voltage[5] = ((reading * cell6_adc_ratio) + cell6_adc_offset);
  
	// 0 - 4 is garbage value
  cell_voltage[4] = ((analogRead(PIN_CELL5) * cell5_adc_ratio) + cell5_adc_offset);
  cell_voltage[3] = ((analogRead(PIN_CELL4) * cell4_adc_ratio) + cell4_adc_offset);
  cell_voltage[2] = ((analogRead(PIN_CELL3) * cell3_adc_ratio) + cell3_adc_offset);
  cell_voltage[1] = ((analogRead(PIN_CELL2) * cell2_adc_ratio) + cell2_adc_offset);
  cell_voltage[0] = ((analogRead(PIN_CELL1) * cell1_adc_ratio) + cell1_adc_offset);

  // Flag if battery low
  if (cell_voltage[5] < BATT_LOW_VOLT) displayLowWarning();

  // Auto-shutdown if battery super low
  if (cell_voltage[5] < BATT_SHUTDOWN_VOLT && cell_voltage[5] > 15) shutDownPMB();
  
}

void PMB::getShuntCurrent(){

  int16_t temp = ADS.readADC_SingleEnded(CHANNEL_CURRENT_SENS_A);
	uint16_t ads_raw = (temp < 0) ? 0 : temp;
//  temp = ADS.readADC_SingleEnded(CHANNEL_CURRENT_SENS_B);
//  uint16_t ads_raw_C = (temp < 0) ? 0 : temp;
	shunt_voltage_raw_array[shunt_voltage_raw_index] = ads_raw;
	shunt_voltage_filtered = mean(shunt_voltage_raw_array);
  //reading = ADS.readADC_SingleEnded(CHANNEL_CURRENT_SENS_A);
  //reading1 = ADS.readADC_SingleEnded(CHANNEL_CURRENT_SENS_B);
	
	// TODO: Recalibrate offsets
	shunt_current = float((shunt_voltage_filtered * CURRENT_RATIO) + CURRENT_OFFSET);
  if (shunt_current < 0) shunt_current = 0;

	shunt_voltage_raw_index = (shunt_voltage_raw_index+1) % MEDIAN_FILTER_SIZE;
}

//Ensure that buffer size == MEDIAN_FILTER_SIZE (odd number)
uint16_t PMB::median(uint16_t buffer[]){
	
	uint8_t median_index = MEDIAN_FILTER_SIZE/2+1; //get index of median
	uint16_t temp_buff[MEDIAN_FILTER_SIZE] = {0};
	uint16_t median_val = 0;

	//Clone a copy of buffer
	for (uint8_t k = 0; k < MEDIAN_FILTER_SIZE; k++){
		temp_buff[k] = buffer[k];
	}

	//Retrieve median by extracting the smallest (median_index - 1) numbers
	for (uint8_t i = 0; i < median_index; i++){
		median_val = extractMin(&temp_buff[0], MEDIAN_FILTER_SIZE);
	}

	return median_val;
}

int16_t PMB::avg(int16_t buffer[]){
  int32_t accum = 0;
  for(uint8_t i=0; i< MEDIAN_FILTER_SIZE; i++){
    accum += buffer[i];
  }
  int16_t avg = accum/MEDIAN_FILTER_SIZE;
  return avg;
}

uint16_t PMB::mean(uint16_t buffer[]){
  uint32_t accum = 0;
  for(uint8_t i=0; i< MEDIAN_FILTER_SIZE; i++){
    accum += buffer[i];
  }
  uint16_t avg = accum/MEDIAN_FILTER_SIZE;
  return avg;
}

uint16_t PMB::extractMin(uint16_t *source, uint8_t size){
	uint16_t temp_min = 0xFFFF;
	uint8_t min_index = 0;
	for (uint8_t i = 0; i < size; i++){
		if(source[i] < temp_min){
			min_index = i;
			temp_min = source[i];
		}
	}
	source[min_index] = 0xFFFF;
	return temp_min;
}	

void PMB::calculateCapacity(){
	
    new_time = millis()*1.0;
    capacity_left = capacity_left - (shunt_current * (new_time - old_time)/1000.0);
	percentage_left = capacity_left * 100.0 / BATTERY_CAPACITY ;
    old_time = new_time;
}

void PMB::readPressure(){
	//## delay(10); //WHY DELAY HERE?
	uint16_t ads_raw = ADS.readADC_SingleEnded(CHANNEL_PRESSURE);
    // Serial.print("Pres raw: ");
    // Serial.println(ads_raw);
	board_pressure = uint8_t(((float)(ads_raw)*0.0001900) / (INTPRES_REF*0.0040) + 10);
}

void PMB::readTemperature(){
	board_temperature = uint16_t(TempSensor.getTemp());
}

void PMB::publishSerial(){
		Serial.print("Board Pressure: ");
		Serial.println(board_pressure);
		//cell Volts  6x2 (3700-4200)
		Serial.print("cell voltages: ");
		Serial.print(cell_voltage[0]);
		Serial.print(" : ");
		Serial.print(cell_voltage[1]);
		Serial.print(" : ");
		Serial.print(cell_voltage[2]);
		Serial.print(" : ");
		Serial.print(cell_voltage[3]);
		Serial.print(" : ");
		Serial.print(cell_voltage[4]);
		Serial.print(" : ");
		Serial.println(cell_voltage[5]);
		//current     x2 (0-15000)
		Serial.print("Shunt current: ");
		Serial.println(shunt_current);
		//capacity    x1 (0-100)
		Serial.print("Percentage left:");
		Serial.println(percentage_left);
		//used mAh    x2 (0-65536)
		Serial.print("Capacity left: ");
		Serial.println(capacity_left);
		//temp x1     (0-255)
		Serial.print("Board temp: ");
		Serial.println(board_temperature);
}

void PMB::publishPMBStats(){
	//Frame 1: PMB_stats1
	CAN.setupCANFrame(PMB_stats1, 0, 2, cell_voltage[0]);
	CAN.setupCANFrame(PMB_stats1, 2, 2, cell_voltage[1]);
	CAN.setupCANFrame(PMB_stats1, 4, 2, cell_voltage[2]);
	CAN.setupCANFrame(PMB_stats1, 6, 2, cell_voltage[3]);

	//Frame 2: PMB_stats2
	CAN.setupCANFrame(PMB_stats2, 0, 2, cell_voltage[4]);
	CAN.setupCANFrame(PMB_stats2, 2, 2, cell_voltage[5]);
	// Serial.println(cell_voltage[5], HEX);
	// Serial.println(PMB_stats2[2], HEX);
	// Serial.println(PMB_stats2[3]);
	// Serial.println(PMB_stats2[3], HEX);
	CAN.setupCANFrame(PMB_stats2, 4, 2, uint16_t(shunt_current));
	CAN.setupCANFrame(PMB_stats2, 6, 2, uint16_t(cell_voltage[5]));

	//Frame 3: PMB_stats3
	CAN.setupCANFrame(PMB_stats3, 0, 2, uint16_t(capacity_used));
	CAN.setupCANFrame(PMB_stats3, 2, 1, percentage_left);
	CAN.setupCANFrame(PMB_stats3, 3, 1, board_temperature);
	CAN.setupCANFrame(PMB_stats3, 4, 1, board_pressure);
  CAN.setupCANFrame(PMB_stats3, 5, 1, (batt_low)? 1: 0);

	//Send those messages
	CAN.sendMsgBuf(ID_CAN_PMB_stats[0], 0, 8, PMB_stats1);
	CAN.sendMsgBuf(ID_CAN_PMB_stats[1], 0, 8, PMB_stats2);
	CAN.sendMsgBuf(ID_CAN_PMB_stats[2], 0, 5, PMB_stats3);
}

void PMB::publishCANStats(){		
	uint8_t PMB_CAN_stats[3] = {19, 20, 21};

	PMB_CAN_stats[0]=CAN.checkError();
	PMB_CAN_stats[1]=CAN.checkTXStatus(0);//check buffer 0
	PMB_CAN_stats[2]=CAN.checkTXStatus(1);//check buffer 1
	
	CAN.sendMsgBuf(ID_CAN_PMB_BUS_stats, 0, 3, PMB_CAN_stats);

	//SEND HEARTBEAT
	uint8_t hb[1] = {ID_CAN_HB};
	CAN.sendMsgBuf(CAN_heartbeat, 0, 1, hb);
}

// Only used by automatic checking of PMB CAN ID
void PMB::MCP2515_ISR(){
  FLAGMsg = true;      
}

void PMB::CAN_init(){
START_INIT:
    if(CAN_OK == CAN.begin(CAN_1000KBPS)){                   // init can bus : baudrate = 500k
        #if DEBUG_MODE == NORMAL
          Serial.println("CAN init ok!");
        #endif           
    }
    else{
        #if DEBUG_MODE == NORMAL
          Serial.println("CAN init fail");
          Serial.println("Init CAN again");
          delay(1000);
        #endif           
        goto START_INIT;
    }

    CAN.init_Mask(0, 0, 0x3ff);// there are 2 mask in mcp2515,
    CAN.init_Mask(1, 0, 0x3ff);// you need to set both of them
    
    // register number, extension,
	CAN.init_Filt(0, 0, CAN_PMB1_stats);
	CAN.init_Filt(1, 0, CAN_PMB1_stats2);
	CAN.init_Filt(2, 0, CAN_PMB1_stats3);
	CAN.init_Filt(3, 0, CAN_PMB2_stats);
	CAN.init_Filt(4, 0, CAN_PMB2_stats2);
	CAN.init_Filt(4, 0, CAN_PMB2_stats3);

    //Interrupt not working yet 0 for Pin D2, Interrupt service routine ,Falling edge 

}

void PMB::CAN_checkForPmb1(){
	uint8_t len = 0; //length of CAN message, taken care by library
	uint8_t buf[8] = {0}; 
				Serial.println("checking");
	//if there is stuff in buffer
	if (CAN_MSGAVAIL == CAN.checkReceive()){
		FLAGMsg = false;
		// Serial.println("interrupt!");
		//read where is it from
		CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
		int tempID = CAN.getCanId();
		Serial.println(tempID);
		switch (tempID){
			case CAN_PMB1_stats || CAN_PMB1_stats2 || CAN_PMB1_stats3://ID = 3 (number)
				//PMB1 present, make this PMB2
				ID_CAN_PMB_stats[0] = CAN_PMB2_stats;
				ID_CAN_PMB_stats[1] = CAN_PMB2_stats2;
				ID_CAN_PMB_stats[2] = CAN_PMB2_stats3;
				ID_CAN_PMB_BUS_stats = CAN_PMB2_BUS_stats;
				ID_CAN_HB = HEARTBEAT_PMB2;
				Serial.println("making it 2");
				break;
			default:;
			//TODO=>throw an error 

		}
		CAN.clearMsg();
	}
}

void PMB::logEEPROM(){
	// uint8_t* buf;
	// CAN.setupCANFrame(buf, 0, 2, int(capacity_left));
	// EEPROM.dumbMultiWrite(10, buf);
}

void PMB::displayTextOLED(char* text, uint8_t size, uint8_t pos){
    display.clear();
	  display.setCursor(pos, 10);
    display.setTextSize(size, 1);
    display.write(text);
}

void PMB::updateDisplay(){	
	//display.clear();
   display.setTextSize(1, 1);
   display.setCursor(0, 0);
   display.write("Battery PMB ");
   display.print(PMB_no);
   display.setCursor(1, 0);
   display.write("Batt %:      ");
   display.setCursor(1, 48);
   display.print(percentage_left);
   display.setCursor(2, 0);
   display.write("Batt Volt: ");
   display.print(cell_voltage[5]);
   display.setCursor(3, 0);
   display.write("Current drawn:        ");
   display.setCursor(3, 88);
   display.print(shunt_current);
   display.setCursor(4, 0);
   display.write("C: ");
   display.print(capacity_left);
   display.setCursor(5, 0);
   display.write("Pod Temp: ");
   display.print(board_temperature);
   display.setCursor(6, 0);
   display.write("Pod Pres: ");
   display.print(board_pressure);
   display.setCursor(7, 0);
//   display.write("|| ");
//   display.print(reading);
//   display.write(" ||");
//   display.print(reading1);
//   display.write(" ||");
   display.write("Low Batt: ");
   (batt_low) ? display.print("YES") : display.print("NO");
}

void PMB::shutDownPMB(){
	displayTextOLED("PMB SHUTDOWN", 1, 2);
	delay(4000);
	digitalWrite(PIN_PMB_POWER, HIGH);
}

void PMB::shutDownVehicle(){
  //displayTextOLED("VEHICLE SHUTDOWN", 1, 2);	
	delay(2000);
	digitalWrite(PIN_VEHICLE_POWER, LOW);
}

void PMB::powerUpVehicle(){
	delay(1000);
	digitalWrite(PIN_VEHICLE_POWER, HIGH);
	display.clear();
}

void PMB::displayLowWarning(){
  //LOW BATTERY WARNING
  batt_low = true;
}
