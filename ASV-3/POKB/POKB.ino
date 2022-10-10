#include "n2420.h"
#include <can.h>
#include <can_asv3_defines.h>
#include "defines.h"

// CAN Setup
uint32_t id = 0;
uint8_t len = 0;
uint8_t buf[8];

MCP_CAN CAN(CAN_CHIP_SELECT);

// N2420 Setup
N2420 n2420 (POKB);
uint8_t inByte = 0;
uint8_t* inBuf;

// Time Counter Variables
uint32_t receiveRemoteKillTime = 0;
uint32_t receiveTelemHeartbeatTime = 0;
uint32_t receivePOSBHeartbeatTime = 0;
uint32_t updateContactorTime = 0;
uint32_t sendPOKBHeartbeatTime = 0;
uint32_t sendPOKBStatusTime = 0;

int noData = 0;

byte pokbStatus = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(SERIAL_BAUD_RATE);
  Serial1.begin(N2420_BAUD_RATE);

  n2420.setSerial(&Serial1);

  pinMode(CONTACTOR_CONTROL, OUTPUT);
  digitalWrite(CONTACTOR_CONTROL, LOW);

  pinMode(ONBOARD_SWITCH, INPUT);

  Serial.println("Plenty of Kill Board.");

  canInitialise();

  receiveRemoteKillTime = millis();
  receiveTelemHeartbeatTime = millis();
  receivePOSBHeartbeatTime = millis();
  updateContactorTime = millis();
  sendPOKBHeartbeatTime = millis();
  sendPOKBStatusTime = millis();
}

void loop() {
  // Receive Onboard Kill via ATmega 2560 Input Pin
  if (digitalRead(ONBOARD_SWITCH)) {
    bitClear(pokbStatus,3);
  }
  else {
    bitSet(pokbStatus,3);
  }

  // Receive Remote Kill via N2420
  if ((millis() - receiveRemoteKillTime) > RECEIVE_REMOTE_KILL_TIMEOUT) {
    receiveRemoteKill();
    receiveRemoteKillTime = millis();
  }

  // Receive Software Kill and POSB Heartbeat via CAN
  receiveCanMessage();

  // Update contactor
  updateContactor();

  // Send POKB Heartbeat via CAN
  if ((millis() - sendPOKBHeartbeatTime) > SEND_POKB_HEARTBEAT_TIMEOUT) {
    sendPOKBHeartbeat();
    sendPOKBHeartbeatTime = millis();
  }

  // Send POKB Status via CAN
  if ((millis() - sendPOKBStatusTime) > SEND_POKB_STATUS_TIMEOUT) {
    sendPOKBStatus();
    sendPOKBStatusTime = millis();
  }
}

void canInitialise() {
START_INIT:
  if (CAN.begin(CAN_1000KBPS) == CAN_OK) {
    Serial.println("CAN Bus: Initialisation successful.");
  }

  else {
    Serial.println("CAN Bus: Initialisation failed.");
    Serial.println("CAN Bus: Re-initialising.");
    delay(1000);
    goto START_INIT;
  }

  Serial.println("Starting transmission...");
}

void receiveRemoteKill() {
  // Continuously read packets
  n2420.readPacket();


  if (n2420.isAvailable()) {
    Serial.println("Response available.");
    Serial.print("ID: ");
    Serial.println(n2420.getReceivingAddress() );

    if (n2420.getReceivingAddress() == REMOTE_KILL_ADDRESS){    //REMOTE_KILL_ADDRESS is defined as 4 from library
      // Get data
      inBuf = n2420.showReceived();
      inByte = *(inBuf+2);
      Serial.print("inByte: ");
      Serial.println(inByte, HEX);
  
      //remoteKill = (inByte == 0x15) ? false : true;
      //noData = 0;
      if (inByte == 0x15) {
        bitClear(pokbStatus,4);
        noData = 0;
      } 
      else if (inByte == 0x44) {
        bitSet(pokbStatus,4);
        noData = 0;
      } 
      else {
        noData++;
      }
    }
  }

  // Uncomment bottom if want to have radio timeout
  
  else {
    noData++;
    Serial.print("n240 unavail noData: ");
    Serial.println(noData);
  }

  
  if (noData >= 20) {
      remoteKill = true;
      noData = 0;
      Serial.println("Connection timeout kill.");
   }
  
}

void receiveCanMessage() {
  if (CAN.checkReceive() == CAN_MSGAVAIL) {
    CAN.readMsgBufID(&id, &len, buf);

    switch (id) {

    case CAN_HEARTBEAT:
      if (buf[0] == HEARTBEAT_TELEM) {
        receiveTelemHeartbeatTime = millis();
        bitClear(pokbStatus,5);
      }
      else if (buf[0] == HEARTBEAT_POSB) {
        receivePOSBHeartbeatTime = millis();
        bitClear(pokbStatus,6);
      }
      break;
     
    case CAN_SOFT_E_STOP:
      if (buf[0] == 1){ // if killed
       // Serial.print("Killed: ");
        //Serial.println(buf[1]);
        bitSet(pokbStatus,buf[1]);
      }
      else { // if alive
       // Serial.print("Not Killed: ");
       // Serial.println(buf[1]);
        bitClear(pokbStatus,buf[1]);
      }   
      break;
      
    default:
      break;
    }

    CAN.clearMsg();
  }
}

void updateContactor() {
  // Failsafe
  // Check POSB Heartbeat 
  // can change to check tele mheartbeat 
  if ((millis() - receiveTelemHeartbeatTime) > RECEIVE_TELEM_HEARTBEAT_TIMEOUT) {
    bitSet(pokbStatus,5);
    digitalWrite(CONTACTOR_CONTROL, LOW);
  }

  if ((millis() - receivePOSBHeartbeatTime) > RECEIVE_POSB_HEARTBEAT_TIMEOUT) {
    bitSet(pokbStatus,6);
    digitalWrite(CONTACTOR_CONTROL, LOW);
  }
  

  // else if ((millis() - updateContactorTime) > UPDATE_CONTACTOR_TIMEOUT) {
  if ((millis() - updateContactorTime) > UPDATE_CONTACTOR_TIMEOUT) {

   
     Serial.println("ESTOP Status (POSB,Telem,Radio,Hard,SBC,FRSKY,OCS): ");
     Serial.println(pokbStatus,BIN);
    

    if (pokbStatus) {
      digitalWrite(CONTACTOR_CONTROL, LOW);
      digitalWrite(KILL, HIGH);
      //Serial.println("Contactor switched off.");
    }

    else {
      digitalWrite(CONTACTOR_CONTROL, HIGH);
      digitalWrite(KILL, LOW);
      //Serial.println("Contactor switched on.");
    }

    updateContactorTime = millis();
  }
}

void sendPOKBHeartbeat() {
  CAN.setupCANFrame(buf, 0, 1, HEARTBEAT_POKB);
  CAN.sendMsgBuf(CAN_HEARTBEAT, 0, 1, buf);
}

void sendPOKBStatus() {
  CAN.setupCANFrame(buf, 0, 1, pokbStatus);
  CAN.sendMsgBuf(CAN_E_STOP, 0, 1, buf);
}
