//===========================================
//
//    FRSKY FUNCTIONS FOR ASV 3 TElEMETRY
//
//===========================================

#include "telem_frsky.h"

// Get RSSI in DB, if above threshold then set hb for frsky
void frsky_get_rssi() {
  // Map RSSI from 1000 to 2000 duty cycle to 0 to 100 dB
  internalStats[RSSI_FRSKY] = calculate_rssi();
//  Serial.println(internalStats[RSSI_FRSKY]);
  if ((internalStats[RSSI_FRSKY] != 255) && (internalStats[RSSI_FRSKY] > RSSI_THRESHOLD)) {
    heartbeat_timeout[FRSKY] = millis();
  }
  else {
    internalStats[RSSI_FRSKY] = 0xFFFF;
  }
}

// get rssi and map from 0 to 100
int calculate_rssi() {
	// Map from [1000 to 2000] to [0 to 100]
	int cppm = constrain(frsky.get_ch(FRISKY_RSSI), 1500, 2000);
//  Serial.println("RSSI");
//  Serial.println(frsky.get_ch(FRISKY_RSSI));
//  Serial.println(cppm);
	cppm -= 1500;
//  Serial.println(map(cppm, 0, 500, 0, 100));
	return map(cppm, 0, 500, 0, 100);
}


// Get control mode from frsky based on arm toggle
void frsky_get_controlmode() {
  if (frsky.get_ch(FRISKY_ARM) > 1800)
  {
    control_mode_frsky = MANUAL_FRSKY;
    #ifdef FRSKYDEBUG
    Serial.println("Manual Frsky");
    #endif 
  }
  else if (frsky.get_ch(FRISKY_ARM) > 1200)
  {
    control_mode_frsky = STATION_KEEP;
    #ifdef FRSKYDEBUG
    Serial.println("Station keep");
    #endif 
  }
  else
  {
    control_mode_frsky = AUTONOMOUS;
    #ifdef FRSKYDEBUG
    Serial.println("Autonomous");
    #endif 
  }
}

int prev = NOTHING;
int frsky_get_hydrophone_act() {
  if (frsky.get_ch(FRISKY_HYDRO) == 1500) {
    return NOTHING;
  }
  if (frsky.get_ch(FRISKY_HYDRO) == 1000) {
    if (prev != EXTEND_ACT) {
      prev = EXTEND_ACT;
      return EXTEND_ACT;
    } else {
      return NOTHING;
    }
  }
  if (frsky.get_ch(FRISKY_HYDRO) == 2000) {
    if (prev != RETRACT_ACT) {
      prev = RETRACT_ACT;
      return RETRACT_ACT;
    } else {
      return NOTHING;
    }
  }
  return NOTHING;
}

// Scale and batt capacity then send over i2c to DAC for transmission to x8r -> frsky
void frsky_send_batt_capacity() {
  uint32_t capacity;
  if (powerStats[BATT1_CAPACITY] == 0xFFFF) {
    capacity = powerStats[BATT2_CAPACITY];
  } else if (powerStats[BATT2_CAPACITY] == 0xFFFF) {
    capacity = powerStats[BATT1_CAPACITY];
  } else {
    capacity =  min(powerStats[BATT1_CAPACITY], powerStats[BATT2_CAPACITY]);
  }
  float scaled_capacity = capacity * 3.3 / 100;
  uint16_t DAC_input = scaled_capacity * 4096 / 5;
  Wire.beginTransmission(I2C_ADDR_DAC);
  Wire.write(DAC_input >> 8);     // top 4 bit of the 12bit voltage
  Wire.write(DAC_input & 0xFF);    // bot 8 bit of the 12bit voltage
  Wire.endTransmission(true);
}

void frsky_get_kill() {
  int tempbut = frsky.get_ch(FRISKY_KILL);
//  Serial.println(tempbut);
  if (tempbut <= 1500 && frsky_alive)
  {
    frsky_kill = false;
    #ifdef FRSKYDEBUG
    Serial.println("Alive");
    #endif 
  }
  else
  {
    frsky_kill = true;
    #ifdef FRSKYDEBUG
    Serial.println("Kill");
    #endif 
  }
}
