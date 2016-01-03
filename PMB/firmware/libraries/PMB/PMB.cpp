// implementation for the PMB class

#include <PMB.h>

PMB::PMB()
	:ADS(ADDR_ADC)
	,TempSensor(CONFIG_TEMP_SENS, ADDR_TEMP_SENS)
	,CAN(PIN_CAN_SS)
	,EEPROM(ADDR_EEPROM0, ADDR_EEPROM1)
	// ,display(DISPLAY_RESET)
	{}

void PMB::init(){
	//init MCU
	analogReference(EXTERNAL);

	//init Digital Pins
	pinMode(PIN_POWEROFF_SIGNAL, INPUT);
	pinMode(PIN_PMB_POWER, OUTPUT);
	pinMode(PIN_VEHICLE_POWER, OUTPUT);
	
	//init devices
	// display.begin(SSD1306_SWITCHCAPVCC, ADDR_OLED);
	ADS.init();
	TempSensor.initTempAD7414();
	CAN_init();
	EEPROM.init();

	//check for other PMB on the CANBUS 3 times
	uint8_t n = 0;
	while(n<3){
		checkForPMB1();
		n++;
		delay(500);
	}
	//init current sensing
	//do multiple times to initialise the array
	for (int i = 0; i < MEDIAN_FILTER_SIZE; ++i){
		readShuntVoltage();
	}
	calculateCurrent();

	if(shunt_current < 0.5){
		getCapFromBattVolt();
	}else{
		getCapFromStorage();
	}

	//read other values so they are initialised
	readCellVoltages();
	readPressure();
	readTemperature();

	//finally turn power to the vehicle on
	powerUpVehicle();
}

void PMB::getCapFromBattVolt(){
	int tempBattVoltage = 0;
	for (int i = 0; i < 20; ++i){
		tempBattVoltage += ((analogRead(7) * cell6_adc_ratio) + cell6_adc_offset);
	}

	tempBattVoltage = float(tempBattVoltage/20.0);  
	percentage_left = pow(tempBattVoltage,3)*coef_a + pow(tempBattVoltage,2)*coef_b + tempBattVoltage*coef_c + coef_d;
	capacity_left = percentage_left/100.0 * BATTERY_CAPACITY;
}

void PMB::getCapFromStorage(){
	// uint8_t* buf;
	// EEPROM.dumbMultiRead(10, buf);
	// capacity_left = uint16_t(CAN.parseCANFrame(buf, 0, 2));
}

void PMB::readCellVoltages(){
	cell_voltage[5] = ((analogRead(7) * cell6_adc_ratio) + cell6_adc_offset);
    cell_voltage[4] = ((analogRead(6) * cell5_adc_ratio) + cell5_adc_offset);
    cell_voltage[3] = ((analogRead(3) * cell4_adc_ratio) + cell4_adc_offset);
    cell_voltage[2] = ((analogRead(2) * cell3_adc_ratio) + cell3_adc_offset);
    cell_voltage[1] = ((analogRead(1) * cell2_adc_ratio) + cell2_adc_offset);
    cell_voltage[0] = ((analogRead(0) * cell1_adc_ratio) + cell1_adc_offset);
}

void PMB::readShuntVoltage(){
	uint16_t raw_ADC_value = ADS.readChannel(CHANNEL_CURRENT_SENS);
	// shunt_voltage = raw_ADC_value/32767*4.096/40;
	shunt_voltage_raw_array[shunt_voltage_raw_index] = raw_ADC_value;

	shunt_voltage_filtered = median(shunt_voltage_raw_array);

	shunt_voltage_raw_index++;
	shunt_voltage_raw_index = shunt_voltage_raw_index%MEDIAN_FILTER_SIZE;
}

//assume buffer size is odd, is equl to MEDIAN_FILTER_SIZE
uint16_t PMB::median(uint16_t buffer[]){
	//using algorithm similar to quicksort
	uint8_t median_index = MEDIAN_FILTER_SIZE/2+1;
	uint16_t median_val = buffer[0]; 

	uint16_t temp_buff[MEDIAN_FILTER_SIZE] = {0};

	for (int k = 0; k < MEDIAN_FILTER_SIZE; k++){
		temp_buff[k] = buffer[k];
	}

	for (int i = 0; i <= median_index; i++){
		median_val = extractMin(&temp_buff[0], MEDIAN_FILTER_SIZE);
	}

	return median_val;
}

uint16_t PMB::extractMin(uint16_t *source, uint8_t size){
	uint16_t temp_min = 0xFFFF;
	uint8_t min_index = 0;
	for (int i = 0; i < size; i++){
		if(source[i] < temp_min){
			min_index = i;
			temp_min = source[i];
		}
	}
	source[min_index] = 0xFFFF;
	return temp_min;
}

void PMB::calculateCurrent(){
	shunt_current = float((shunt_voltage_filtered*CURRENT_RATIO)+CURRENT_OFFSET);
}

void PMB::calculateCapacity(){
	capacity_used += shunt_current*MAIN_LOOP_INTERVAL/1000.0/3600.0;
    capacity_left = BATTERY_CAPACITY - capacity_used;
    percentage_left = capacity_left/100.0;
}

void PMB::readPressure(){
	uint16_t raw_ADC_value = ADS.readChannel(CHANNEL_PRESSURE);
	board_pressure = uint8_t(((float)raw_ADC_value*0.0001875) / (INTPRES_REF*0.0040) + 10);
}

void PMB::readTemperature(){
	board_temperature = uint16_t(TempSensor.getTemp());
}

void PMB::writeIntSerial(int data, int len){
  if(ascii){
    Serial.print(data);
  } else {
    char* a = (char*)&data;  
    int sent= Serial.write(a);

    //to make up for remaining bytes
    if(sent<len){
      for(int i=0; i<(len-sent); i++){
        Serial.write(0);
      }
    }
  }  
}

void PMB::publishSerial(){
  //everything written must be an int, so convert first in loop
	if (PMB_DEBUG_MODE){
		Serial.print("Board Pressure: ");
		Serial.println(board_pressure);
		//cell Volts  6x2 (3700-4200)
		Serial.print("cell voltages: ");
		Serial.print(cell_voltage[0]);
		Serial.print(cell_voltage[1]);
		Serial.print(cell_voltage[2]);
		Serial.print(cell_voltage[3]);
		Serial.print(cell_voltage[4]);
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
	}else{
		//start byte
		Serial.write('#');
		//pressure    x1 (0-255)
		writeIntSerial(int(board_pressure), 1);
		//cell Volts  6x2 (3700-4200)
		writeIntSerial(int(cell_voltage[0]), 2);
		writeIntSerial(int(cell_voltage[1]), 2);
		writeIntSerial(int(cell_voltage[2]), 2);
		writeIntSerial(int(cell_voltage[3]), 2);
		writeIntSerial(int(cell_voltage[4]), 2);
		writeIntSerial(int(cell_voltage[5]), 2);
		//current     x2 (0-15000)
		writeIntSerial(int(shunt_current), 2);
		//capacity    x1 (0-100)
		writeIntSerial(int(percentage_left), 1);
		//used mAh    x2 (0-65536)
		writeIntSerial(int(capacity_left), 2);
		//temp x1     (0-255)
		writeIntSerial(int(board_temperature), 1);
	}
}

void PMB::publishPMBStats(){
	uint8_t PMB_stats1[8] = {0, 1, 2, 3 , 4, 5, 6, 7};
	uint8_t PMB_stats2[8] = {8, 9, 10, 11, 12, 13, 14, 15};
	uint8_t PMB_stats3[5] = {16, 17, 18};

	//Frame 1: PMB_stats1
	CAN.setupCANFrame(PMB_stats1, 0, 2, cell_voltage[0]);
	CAN.setupCANFrame(PMB_stats1, 2, 2, cell_voltage[1]);
	CAN.setupCANFrame(PMB_stats1, 4, 2, cell_voltage[2]);
	CAN.setupCANFrame(PMB_stats1, 6, 2, cell_voltage[3]);

	//Frame 2: PMB_stats2
	CAN.setupCANFrame(PMB_stats2, 0, 2, cell_voltage[4]);
	CAN.setupCANFrame(PMB_stats2, 2, 2, cell_voltage[5]);
	CAN.setupCANFrame(PMB_stats2, 4, 2, uint16_t(shunt_current));
	CAN.setupCANFrame(PMB_stats2, 6, 2, uint16_t(shunt_current));

	//Frame 3: PMB_stats3
	CAN.setupCANFrame(PMB_stats3, 0, 2, uint16_t(capacity_used));
	CAN.setupCANFrame(PMB_stats3, 2, 1, percentage_left);
	CAN.setupCANFrame(PMB_stats3, 3, 1, board_temperature);
	CAN.setupCANFrame(PMB_stats3, 4, 1, board_pressure);

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
}

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

void PMB::checkForPMB1(){
	uint8_t len = 0; //length of CAN message, taken care by library
	uint8_t buf[8] = {0}; 
	//if there is stuff in buffer
	if (CAN_MSGAVAIL == CAN.checkReceive()){
		FLAGMsg = false;
		// Serial.println("interrupt!");
		//read where is it from
		CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

		switch (CAN.getCanId()){
			case CAN_PMB1_stats || CAN_PMB1_stats2 || CAN_PMB1_stats3://ID = 3 (number)
				//PMB1 present, make this PMB2
				ID_CAN_PMB_stats[0] = CAN_PMB2_stats;
				ID_CAN_PMB_stats[1] = CAN_PMB2_stats2;
				ID_CAN_PMB_stats[2] = CAN_PMB2_stats3;
				ID_CAN_PMB_BUS_stats = CAN_PMB2_BUS_stats;
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

// void PMB::updateDisplay(){	
//   display.clearDisplay();
//   display.setTextSize(5);
//   display.setTextColor(WHITE);
//   display.setCursor(0,0);
//   display.println("PMB V2 aww yiss");
//   display.display();
//   delay(2000);
//   display.clearDisplay();

// }

void PMB::shutDownPMB(){
	// display.clearDisplay();
	// display.setTextSize(5);
	// display.setTextColor(WHITE);
	// display.setCursor(0,0);
	// display.println("PMB SHUTDOWN!");
	// display.display();
	// delay(2000);
	// display.clearDisplay();
	digitalWrite(PIN_PMB_POWER, HIGH);
}

void PMB::shutDownVehicle(){
	// display.clearDisplay();
	// display.setTextSize(5);
	// display.setTextColor(WHITE);
	// display.setCursor(0,0);
	// display.println("VEHICLE SHUTDOWN");
	// display.display();
	// delay(2000);
	// display.clearDisplay();
	digitalWrite(PIN_VEHICLE_POWER, LOW);
}

void PMB::powerUpVehicle(){
	// display.clearDisplay();
	// display.setTextSize(5);
	// display.setTextColor(WHITE);
	// display.setCursor(0,0);
	// display.println("VEHICLE POWERUP");
	// display.display();
	// delay(2000);
	// display.clearDisplay();
	digitalWrite(PIN_VEHICLE_POWER, HIGH);
}