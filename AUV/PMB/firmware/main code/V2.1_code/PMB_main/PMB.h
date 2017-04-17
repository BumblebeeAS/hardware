// PMB.h
// contains the class definition of the PMB
#ifndef PMB_H
#define PMB_H

//Include self-written
#include "calibration.h"
#include "PMB_defines.h"
#include "can_defines.h"

//Include libraries
#include <Adafruit_ADS1015.h>
#include <TempAD7414.h>
#include <can.h>
#include <EEPROMPlus.h>
#include <SSD1306_text.h>
//#include <Adafruit_SSD1306.h>

class PMB{
private:
	uint16_t shunt_voltage_raw_array[MEDIAN_FILTER_SIZE] 	= {0};
	uint8_t shunt_voltage_raw_index 						= 0;
	uint16_t shunt_voltage_filtered 						= 0;
	float shunt_current  									= 0.0;
	uint32_t old_time                                       = 0.0;
	uint32_t new_time                                       = 0.0;
							
	float capacity_used 									= 0.0;
	float capacity_left 									= 0.0;
							
	uint8_t percentage_initial 								= 100;
	uint8_t percentage_left 								= 100;
				
	uint16_t cell_voltage[CELLS]   							= {0};
	//for filtering cell 6
	uint16_t cell6_raw_array[MEDIAN_FILTER_SIZE] 			= {0};
	uint8_t cell6_raw_index 								= 0;

  //uint16_t reading = 0;
  
  bool batt_low                           = false;
        
	uint8_t board_pressure 									= 0;
	uint8_t board_temperature 								= 0;

	//for CAN
	bool FLAGMsg 											= false;
	//to store the msg ids for the specific PMB no
	uint8_t ID_CAN_PMB_stats[3]								= {CAN_PMB2_stats, CAN_PMB2_stats2, CAN_PMB2_stats3};
	uint8_t ID_CAN_PMB_BUS_stats							= CAN_PMB2_BUS_stats;
	uint8_t ID_CAN_HB										= HEARTBEAT_PMB2;

	uint8_t PMB_stats1[8]									= {0, 1, 2, 3, 4, 5, 6, 7};
	uint8_t PMB_stats2[8] 									= {8, 9, 10, 11, 12, 13, 14, 15};
	uint8_t PMB_stats3[6] 									= {16, 17, 18, 19, 20, 21};

	//all the components
	// ADS1115 ADS;
	Adafruit_ADS1115 ADS;
	TempAD7414 TempSensor;
	MCP_CAN CAN;
	EEPROMPlus EEPROM;
	SSD1306_text display;
	//Adafruit_SSD1306 display;

	void CAN_init();
	void CAN_checkForPmb1();
	void getCanId();
	
	void displayTextOLED(char* text, uint8_t size, uint8_t pos);
	void getCapFromVolt();
	void getCapFromStorage();
	uint16_t median(uint16_t buffer[]);
  uint16_t mean(uint16_t buffer[]);
	uint16_t extractMin(uint16_t *source, uint8_t size);

public:
	PMB();

	void init();
	void readCellVoltages();
	void getShuntCurrent();
	void calculateCapacity();
	void readPressure();
	void readTemperature();
	void publishSerial();
	void MCP2515_ISR();
	void publishPMBStats();
	void publishCANStats();
	void logEEPROM();
	void updateDisplay();
	void shutDownPMB();
	void shutDownVehicle();
	void powerUpVehicle();
	void displayLowWarning();
};


#endif
