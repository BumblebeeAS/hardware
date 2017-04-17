//Library with the implementation of all the functions of the PMB
#include "PMB.h"

//local variables
PMB thisPMB;
uint32_t lastMainLoop = 0;
uint32_t lastMsgLoop = 0;
uint32_t lastCANStatusLoop = 0;

void setup(){
  Serial.begin(9600);
  delay(500);
	thisPMB.init();
}

void loop(){
	//timed loop
	uint32_t now = millis();

	//See if need to shutdown
	if(digitalRead(PIN_POWEROFF_SIGNAL)){
		//Serial.println(digitalRead(PIN_POWEROFF_SIGNAL));
	    thisPMB.shutDownPMB();
	}

	//main loop for calculation
	if(now - lastMainLoop >= MAIN_LOOP_INTERVAL){
		lastMainLoop = now;
		//Serial.println("main loop");
		thisPMB.readCellVoltages();
		//Serial.println("cell volt");
		thisPMB.getShuntCurrent();
		//Serial.println("shunt curr");
		thisPMB.calculateCapacity();
		//Serial.println("capacity");
		// TODO: Code EEPROM
		//thisPMB.logEEPROM();
	}
	
	//secondary loop for transmitting messages
	if(now - lastMsgLoop >= MSG_LOOP_INTERVAL){
    	lastMsgLoop = now;
		thisPMB.readPressure();
		thisPMB.readTemperature();
		//serial for debugging purposes
		thisPMB.publishSerial();
    thisPMB.publishPMBStats();
		thisPMB.updateDisplay();
	}

	//tertiary loop for transmitting CAN status
	if(now - lastCANStatusLoop >= CAN_STATUS_LOOP_INTERVAL){
    	lastCANStatusLoop = now;
		thisPMB.publishCANStats();
	}
}
