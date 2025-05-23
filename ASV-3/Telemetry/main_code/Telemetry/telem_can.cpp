//===========================================
//
//    CAN FUNCTIONS FOR ASV 3 TElEMETRY
//
//===========================================

#include "telem_can.h"

// Initialize CAN bus 
void CAN_init() {
	if (CAN_OK == CAN.begin(CAN_1000KBPS)) {                   // init can bus : baudrate = 1000k
		Serial.println("CAN init ok!");
	}
	else {
		Serial.println("CAN init fail");
		Serial.println("Init CAN again");
		delay(100);
    CAN_init();
	}
}

/* Receive these CAN ID
 *  3:  CAN_HEARTBEAT           0b00000011
 *  5:  CAN_E_STOP              0b00000101
 *  10: CAN_BATT1_STATS         0b00001010
 *  11: CAN_BATT2_STATS         0b00001011
 *  17: CAN_SBC_TEMP            0b00010001
 *  18: CAN_POSB_STATS          0b00010010
 */

// Initialize CAN mask using truth table 
/*
  Truth table
  mask  filter    id bit  reject
  0     X         X       no
  1     0         0       no
  1     0         1       yes
  1     1         0       yes
  1     1         1       no
  Mask 0 connects to filt 0,1
  Mask 1 connects to filt 2,3,4,5
  Mask decide which bit to check
  Filt decide which bit to accept
*/
void CAN_mask() {
//  CAN.init_Mask(0, 0, 0xf0);
//  CAN.init_Mask(1, 0, 0xf0);
//  CAN.init_Filt(0, 0, 0x00);
}

// Receive CAN messages
void CAN_read_msg() {
    if (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBufID(&id, &len, buf);    // read data,  len: data length, buf: data buf
    switch (CAN.getCanId()) {
      case CAN_HEARTBEAT: {
        read_heartbeat();
        break;
      }
      case CAN_BATT1_STATS: {
        read_batt_stats(1);       // 1 for batt 1 
        batt1_timeout = millis();
        break;
      }
      case CAN_BATT2_STATS: {
        read_batt_stats(2);       // 2 for batt 2
        batt2_timeout = millis();
        break;
      }
      case CAN_SBC_TEMP: {
        internalStats[CPU_TEMP] = CAN.parseCANFrame(buf, 0, 1);
        sbc_timeout = millis();
        break;
      }
      case CAN_POSB_STATS: {
        read_posb_stats();
        posb_timeout = millis();
        break;
      }
      case CAN_E_STOP: {
        uint8_t kill = CAN.parseCANFrame(buf, 0, 1);
        hard_kill = kill & 0b00001000;
        SBC_kill = kill & 0b00000100; 
        OCS_kill = kill & 0b00000001;
        radio_kill = kill & 0b00010000;
        break;
      }
      case 25: {
         uint8_t device = CAN.parseCANFrame(buf, 0, 1);
         heartbeat_timeout[device] = millis();
         break;
      }
      default: {
        #ifdef CANDEBUG
//          Serial.println(CAN.getCanId());
        #endif
        break;
      }
    }
  }
}

// Byte 5: Temp (celsius), Byte 4-3: Current (0.1A), Byte 3-2: Voltage (0.01V), Byte 0: Capacity (%) 
void read_batt_stats(int batt_no) {
  switch (batt_no) {  
    case 1:
      powerStats[BATT1_CAPACITY] = CAN.parseCANFrame(buf, 0, 1);
      powerStats[BATT1_VOLTAGE] = CAN.parseCANFrame(buf, 1, 2);
      powerStats[BATT1_CURRENT] = CAN.parseCANFrame(buf, 3, 2);
      break;
    case 2: 
      powerStats[BATT2_CAPACITY] = CAN.parseCANFrame(buf, 0, 1);
      powerStats[BATT2_VOLTAGE] = CAN.parseCANFrame(buf, 1, 2);
      powerStats[BATT2_CURRENT] = CAN.parseCANFrame(buf, 3, 2);
      break;
    default: 
      #ifdef CANDEBUG
        Serial.println("Too many batteries");
      #endif
      break;
  }
}

void read_heartbeat() {
   uint8_t device = CAN.parseCANFrame(buf, 0, 1);
   heartbeat_timeout[device] = millis();
   #ifdef CANDEBUG
//   Serial.println("Heartbeat: ");
//   Serial.println(device);
   #endif 
}

void read_posb_stats() {
   internalStats[POSB_TEMP] = CAN.parseCANFrame(buf, 0, 1);
   internalStats[HUMIDITY] = CAN.parseCANFrame(buf, 1, 1);
   internalStats[INT_PRESS] = CAN.parseCANFrame(buf, 2, 2) == 0 ? 0xFFFF : CAN.parseCANFrame(buf, 2, 2);
   internalStats[HULL_LEAK] = CAN.parseCANFrame(buf, 4, 1);
//   Serial.println("stats");
//   Serial.println(internalStats[POSB_TEMP]);
//   Serial.println(internalStats[HUMIDITY]);
//   Serial.println(internalStats[INT_PRESS]);
//   Serial.println(internalStats[HULL_LEAK]);
}

void CAN_publish_hb(int hb) {
  buf[0] = hb;
  CAN.sendMsgBuf(CAN_HEARTBEAT, 0, 1, buf);
}

void CAN_publish_controllink() {
  len = 3;
  buf[0] = control_mode;
  buf[1] = internalStats[RSSI_FRSKY];
  buf[2] = internalStats[RSSI_OCS];
  CAN.sendMsgBuf(CAN_CONTROL_LINK, 0, len, buf);
}

void CAN_publish_kill() {
  // OCS kill
//  CAN.setupCANFrame(buf, 0, 1, OCS_kill ? 1 : 0);
//  CAN.setupCANFrame(buf, 1, 1, 0);
//  CAN.sendMsgBuf(CAN_SOFT_E_STOP, 0, 2, buf);
  // Frsky kill
  CAN.setupCANFrame(buf, 0, 1, frsky_kill ? 1 : 0);
  CAN.setupCANFrame(buf, 1, 1, 1);
  CAN.sendMsgBuf(CAN_SOFT_E_STOP, 0, 2, buf);
}

void CAN_publish_hydroact(int hydrophone_act) {
  if (hydrophone_act != 0) {
    CAN.setupCANFrame(buf, 0, 1, hydrophone_act);
    CAN.sendMsgBuf(CAN_BATT_CTRL, 0, 1, buf);
  }
}
