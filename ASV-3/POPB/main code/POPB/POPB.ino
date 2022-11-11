#include <can.h>
#include <can_asv3_defines.h>
#include "defines.h"
#include <math.h>

MCP_CAN CAN(8);
uint32_t id = 0;
uint8_t len = 0;
uint8_t buf[8];
uint32_t autoResetLoop = 0;
uint32_t CanHeartbeatLoop = 0;
uint32_t CanStatusLoop = 0;
uint32_t SerialLoop = 0;
uint32_t TempLoop = 0;
uint8_t LsControl = 0x15; //Indicates which LS are turned on/off

bool SBC_stat_on = false;
bool relay_on = false;
uint32_t RelayLoop = 0;
float temp1=0, temp2=0;

const float temp_A1 = 335.4016434;
const float temp_B1 = 25.6523550;
const float temp_C1 = 0.2605970;
const float temp_D1 = 0.0063292;

void setup()
{
	Serial.begin(115200);
	Serial.println("This is POPB!");

  /* Initialise Load Switch Pins*/
  pinMode(LS_NAVTIC, OUTPUT);
  pinMode(LS_OUSTER, OUTPUT);
  pinMode(LS_VELODYNE, OUTPUT);
  pinMode(RL_SBC, OUTPUT);
  pinMode(RL_INVERTER, OUTPUT);

  /* Startup Sequence*/
  //First off everything
  digitalWrite(LS_NAVTIC, LOW);
  digitalWrite(LS_OUSTER, LOW);
  digitalWrite(LS_VELODYNE, LOW);
  digitalWrite(RL_SBC, LOW);
  digitalWrite(RL_INVERTER, LOW);

  //Staggered turn on
  digitalWrite(LS_NAVTIC, HIGH);
  delay(STAGGER_TIME);
  digitalWrite(LS_OUSTER, HIGH);
  delay(STAGGER_TIME);
  digitalWrite(LS_VELODYNE, HIGH);
  
	CAN_init();

	// CAN Masking
	CAN.init_Mask(0, 0, 0x3FF);
	CAN.init_Mask(1, 0, 0x3FF);
	CAN.init_Filt(0, 0, CAN_POPB_CONTROL);
  CAN.init_Filt(1, 0, CAN_HEARTBEAT);
}

void loop()
{
  if (Serial.available()){
    byte input = Serial.read();
    if(input == '1'){
      Serial.println("Relay on");
      digitalWrite(RL_SBC, HIGH);
      delay(10);
    }
    else if(input == '2'){
      Serial.println("Relay on");
      digitalWrite(RL_SBC, LOW);
      delay(10);
    }
  else if(input == '3'){
      Serial.println("INV on");
      digitalWrite(RL_INVERTER, LOW);
      delay(10);
    }
   else if(input == '4'){
    Serial.println("INV off");
      digitalWrite(RL_INVERTER, HIGH);
      delay(10);
   }
   else if(input == 'n'){
      Serial.println("Restart all sensors");
      digitalWrite(LS_NAVTIC, LOW);
      digitalWrite(LS_OUSTER, LOW);
      digitalWrite(LS_VELODYNE, LOW);
      delay(10);
   }
   else if(input == 'f'){
      digitalWrite(LS_NAVTIC, HIGH);
      digitalWrite(LS_OUSTER, HIGH);
      digitalWrite(LS_VELODYNE, HIGH);
      delay(10);
   }
   else if(input == 's'){
    SBC_on();
   }
  }
	// POPB Heartbeat
	if ((millis() - CanHeartbeatLoop) > CAN_HEARTBEAT_INTERVAL) {
		publishCanHB();
		CanHeartbeatLoop = millis();
	}

	if ((millis() - CanStatusLoop) > CAN_STATUS_INTERVAL) {
		publishCanStatus();
		CanStatusLoop = millis();
	}

  if (millis() - TempLoop > TEMP_INTERVAL){
    temp1 = Measure_Temp(TEMP1);
    temp2 = Measure_Temp(TEMP2);
    if (DEBUG_MODE){
      Serial.print("Temp1: ");
      Serial.print(temp1);
      Serial.print(" Temp2: ");
      Serial.println(temp2);
    }
    TempLoop = millis();
  }
 

  if (relay_on){
    if (millis() - RelayLoop > RELAY_INTERVAL){
      Serial.println("sbc off");
      digitalWrite(RL_SBC, LOW);
      delay(10);
      relay_on = false;
    }
  }

	// Check for incoming CAN message
	if (checkCanMsg()) {
		// Update load switch states
		if (DEBUG_MODE) 
			Serial.println("Updating load switch states");
		
		updateLoadSwitch();
	}

	if (DEBUG_MODE && ((millis() - SerialLoop) > SERIAL_INTERVAL)) {
		Serial.print("Status: ");
		Serial.println(LsControl, HEX);
		SerialLoop = millis();
	}
}

void CAN_init()
{
START_INIT:
	if (CAN_OK == CAN.begin(CAN_1000KBPS))                   // init can bus : baudrate = 1000Kbps
	{
		Serial.println("CAN BUS: OK");
}
	else
	{
		Serial.println("CAN BUS: FAILED");
		Serial.println("CAN BUS: Reinitializing");
		delay(1000);
		goto START_INIT;
	}
	Serial.println("INITIATING TRANSMISSION...");
}

void publishCanStatus() {
  CAN.setupCANFrame(buf, 0, 1, LsControl);
	CAN.setupCANFrame(buf, 1, 2, temp1);
  CAN.setupCANFrame(buf, 3, 2, temp2);
	CAN.sendMsgBuf(CAN_POPB_STATS, 0, 5, buf);
}

void publishCanHB() {
  CAN.setupCANFrame(buf, 0, 1, HEARTBEAT_POPB);
	CAN.sendMsgBuf(CAN_HEARTBEAT, 0, 1, buf);
}

boolean checkCanMsg() {
	//Serial.println("checking CAN");
	if (CAN_MSGAVAIL == CAN.checkReceive()) {
		CAN.readMsgBufID(&id, &len, buf);    // read data,  len: data length, buf: data buf
		boolean mine = false;
		switch (id) {
		case CAN_POPB_CONTROL:
			LsControl = CAN.parseCANFrame(buf, 0, 1);
			mine = true;
			if (DEBUG_MODE)
				Serial.println(LsControl);
			break;
    case CAN_HEARTBEAT:
      int heartbeat;
      heartbeat = CAN.parseCANFrame(buf, 0, 1);
      if (heartbeat == HEARTBEAT_BATT1 && !SBC_stat_on){
        SBC_on();
        SBC_stat_on = true;
      }
		default:
			//Serial.println("Others");
			break;
		}
		CAN.clearMsg();
		return mine;
	}
	else
		return false;
}

void updateLoadSwitch() {
	if ((LsControl & ENABLE_NAVTIC) > 0) {
		digitalWrite(LS_NAVTIC, HIGH);
		if (DEBUG_MODE)
			Serial.print("NAV:H | ");
	}
	else {
		digitalWrite(LS_NAVTIC, LOW);
		if (DEBUG_MODE)
			Serial.print("NAV:L | ");
	}

	if ((LsControl & ENABLE_OUSTER) > 0) {
		digitalWrite(LS_OUSTER, HIGH);
		if (DEBUG_MODE)
			Serial.print("OUSTER:H | ");
	}
	else {
		digitalWrite(LS_OUSTER, LOW);
		if (DEBUG_MODE)
			Serial.print("OUSTER:L | ");
	}

	if ((LsControl & ENABLE_VELODYNE) > 0) {
		digitalWrite(LS_VELODYNE, HIGH);
		if (DEBUG_MODE)
			Serial.print("VELO:H | ");
	}
	else {
		digitalWrite(LS_VELODYNE, LOW);
		if (DEBUG_MODE)
			Serial.print("VELO:L | ");
	}
  if ((LsControl & ENABLE_INVERTER) > 0) {
    digitalWrite(RL_INVERTER, LOW);
    if (DEBUG_MODE)
      Serial.print("INVERTER:L | ");
  }
  else{
    digitalWrite(RL_INVERTER, HIGH);
    if (DEBUG_MODE)
      Serial.print("INVERTER:H | ");
  }
	if (DEBUG_MODE)
		Serial.println(LsControl, HEX);
  if ((LsControl & ENABLE_SBC) > 0) {
    SBC_on();
    }
  }

void SBC_on(){
    RelayLoop = millis();
    if (relay_on) return;
    relay_on = true;
    digitalWrite(RL_SBC, HIGH);
    if (DEBUG_MODE)
      Serial.print("SBC on");
}

float Measure_Temp(int pin){
  int t = analogRead(pin);  
//  Serial.print("analog: ");
//  Serial.println(t);
  float vt = ((float) t+1)/1024*5;  
  float r = vt*2000/(5-vt);
//  Serial.print("resistance: ");
//  Serial.println(r);
  float calc_temp = 1/((temp_A1+temp_B1*log(r/10000)+temp_C1*pow(log(r/10000), 2)+temp_D1*pow(log(r/10000),3))/100000)-273.15;
  return calc_temp;
}
