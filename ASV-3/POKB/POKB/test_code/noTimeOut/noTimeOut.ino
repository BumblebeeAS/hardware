#include "n2420.h"
#include <can.h>
#include <can_asv3_defines.h>
#include "defines.h"

#define CAN_CHIP_SELECT 8
#define SERIAL_BAUD_RATE 115200
#define N2420_BAUD_RATE 115200
#define REMOTE_KILL_ADDRESS 4

#define CONTACTOR_CONTROL 11 //NMOS, Active High
#define ONBOARD_SWITCH 21      
#define KILL 31

#define RECEIVE_REMOTE_KILL_TIMEOUT 500
#define RECEIVE_POSB_HEARTBEAT_TIMEOUT 3000
#define UPDATE_CONTACTOR_TIMEOUT 100
#define SEND_POKB_HEARTBEAT_TIMEOUT 500
#define SEND_POKB_STATUS_TIMEOUT 1000

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
uint32_t receivePOSBHeartbeatTime = 0;
uint32_t updateContactorTime = 0;
uint32_t sendPOKBHeartbeatTime = 0;
uint32_t sendPOKBStatusTime = 0;

int noData = 0;

// Control Variables
bool onboardKill = false;
bool remoteKill = false;
bool softwareKill = false;

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
  receivePOSBHeartbeatTime = millis();
  updateContactorTime = millis();
  sendPOKBHeartbeatTime = millis();
  sendPOKBStatusTime = millis();
}

void loop() {
  // Receive Onboard Kill via ATmega 2560 Input Pin
  onboardKill = !digitalRead(ONBOARD_SWITCH);

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
    if (onboardKill || remoteKill || softwareKill) {
      sendPOKBStatus(true);
    }

    else {
      sendPOKBStatus(false);
    }

    sendPOKBStatusTime = millis();
  }

  //Display Status
  Serial.print("Onboard: ");
  Serial.print(onboardKill);
  Serial.print(" Remote: ");
  Serial.print(remoteKill);
  Serial.print(" Software: ");
  Serial.println(softwareKill);
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
        remoteKill = false;
        noData = 0;
      } 
      else if (inByte == 0x44) {
        remoteKill = true;
        noData = 0;
      } 
      else {
        noData++;
      }
    }
  }


  // if dont have nvm
  else {
    
  }


  
//  if (noData >= 20) {
//      remoteKill = true; NO TIMEOUT KILL
//      noData = 0;
//      Serial.println("Connection timeout");
//   }
}

void receiveCanMessage() {
  if (CAN.checkReceive() == CAN_MSGAVAIL) {
    CAN.readMsgBufID(&id, &len, buf);

    switch (id) {
    case CAN_HEARTBEAT:

    if (buf[0] == HEARTBEAT_POSB)
        receivePOSBHeartbeatTime = millis();
      break;
    case CAN_SOFT_E_STOP:
      softwareKill = !buf[0];
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

  /*
  if ((millis() - receivePOSBHeartbeatTime) > RECEIVE_POSB_HEARTBEAT_TIMEOUT) {
    // digitalWrite(CONTACTOR_CONTROL, LOW);
  }
  */

  // else if ((millis() - updateContactorTime) > UPDATE_CONTACTOR_TIMEOUT) {
  if ((millis() - updateContactorTime) > UPDATE_CONTACTOR_TIMEOUT) {
    // Serial.print("Onboard Kill: ");
    // Serial.print(onboardKill);
    // Serial.print(" | ");
    // Serial.print("Remote Kill: ");
    // Serial.print(remoteKill);
    // Serial.print(" | ");
    // Serial.print("Software Kill: ");
    // Serial.println(softwareKill);

    if (onboardKill || remoteKill || softwareKill) {
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

void sendPOKBStatus(bool pokbStatus) {
  CAN.setupCANFrame(buf, 0, 1, pokbStatus);
  CAN.sendMsgBuf(CAN_E_STOP, 0, 1, buf);
}
