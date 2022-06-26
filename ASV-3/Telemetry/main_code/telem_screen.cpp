//===========================================
//
//    SCREEN FUNCTIONS FOR ASV TElEMETRY
//
//===========================================
#include "telem_screen.h"

void screen_prepare() {
  screen.set_cursor(0 + OFFSET, 0);
  screen.write_string("Int press:");
  screen.write_string("Humidity:");
  screen.write_string("CPU temp:");
  screen.write_string("POSB temp:");
  screen.write_string("RSSI OCS:");
  screen.write_string("RSSI RC:");
  screen.write_string("Hull leak:");
  screen.write_string("POSB OK:");
  screen.write_string("POPB OK:");
  screen.write_string("POKB OK:");
  screen.write_string("SBC OK:");
  screen.write_string("OCS OK:");
  screen.write_string("Frsky OK:");

  screen.set_cursor(300 + OFFSET, 0);
  screen.write_string("Batt1 capacity:");
  screen.write_string("Batt2 capacity:");
  screen.write_string("Batt1 current:");
  screen.write_string("Batt2 current:");
  screen.write_string("Batt1 voltage:");
  screen.write_string("Batt2 voltage:");
  screen.write_string("Logic Bkplane OK:");
  screen.write_string("Ballshooter OK:");
  screen.write_string("Act Hydrophone OK:");
  screen.write_string("Batt1 OK:");
  screen.write_string("Batt2 OK:");
  screen.write_string("ESC1 OK:");
  screen.write_string("ESC2 OK:");

}

// Display stats
void screen_update_stats() {
  screen.set_cursor(170 + OFFSET, 0);
  for (int i = 0; i < INT_STAT_COUNT; i++)
  {
    if (i == HULL_LEAK){ 
      if (internalStats[HULL_LEAK] == 1) {
        screen.write_string("YES");
      } else if (internalStats[HULL_LEAK] == 0) {
        screen.write_string("NO");
      } else {
        screen.write_string("N/A");
      }
    } else if (i == INT_PRESS) {
      screen.write_value_with_dp(internalStats[i], 1);           // Display pressure kpa with 1dp
    } else {
      screen.write_value_int(internalStats[i]);
    }
    #ifdef DEBUG 
    Serial.print(i);
    Serial.print(": ");
    Serial.println(internalStats[i]);
    #endif 
  }

  screen.set_cursor(650 + OFFSET, 0);
  for (int i = 0; i < POWER_STAT_COUNT; i++)
  {
    if (i == BATT1_CURRENT || i == BATT2_CURRENT) {
      screen.write_value_with_dp(powerStats[i], 1);           // Display pressure as A with 1dp 
    } else if (i == BATT1_VOLTAGE || i == BATT1_VOLTAGE) {
      screen.write_value_with_dp(powerStats[i], 2);           // Display pressure as V with 1dp 
    } else {
      screen.write_value_int(powerStats[i]);
    }
    #ifdef DEBUG 
    Serial.print(i);
    Serial.print(": ");
    Serial.println(internalStats[i]);
    #endif 
  }
}

// Display heartbeats
void screen_update_hb() {
  int i; 
  screen.set_cursor(170 + OFFSET, 245);            // do right half
  for (i = 1; i < 9; i++) {
    if (i != TELEMETRY && i != DTLS)                           // Skip Telemetry and DTLS HB 
    {
      if ((millis() - heartbeat_timeout[i]) > HB_TIMEOUT) {
        screen.write_value_string("NO");
      }
      else
        screen.write_value_string("YES");
    }
  }   
  screen.set_cursor(650 + OFFSET, 210);           // do left half
  for (; i < HB_COUNT; i++) {                     
    if (i != ACTUATED_THRUSTERS) {                // skip actuated thrusters for now 
      if ((millis() - heartbeat_timeout[i]) > HB_TIMEOUT) {
        screen.write_value_string("NO");
      }
      else
        screen.write_value_string("YES");
    }
  }
}

//==========================================
//          UPDATE DATA
//==========================================
uint32_t posb_timeout = millis();
uint32_t ocs_timeout = millis();
uint32_t frsky_timeout = millis();
uint32_t sbc_timeout = millis();
uint32_t batt1_timeout = millis();
uint32_t batt2_timeout = millis();

void screen_reset_stats() {
  reset_posb_stats();
  reset_ocs_stats();
  reset_rc_stats();
  reset_sbc_stats();
  reset_batt1_stats();
  reset_batt2_stats();
}
void reset_posb_stats() {
  if ((millis() - posb_timeout) > STAT_TIMEOUT) {
    internalStats[INT_PRESS] = 0xFFFF;
    internalStats[HUMIDITY] = 0xFFFF;
    internalStats[POSB_TEMP] = 0xFFFF;
    internalStats[HULL_LEAK] = 0xFFFF;
    posb_timeout = millis();
  }
}
void reset_ocs_stats() {
  if ((millis() - ocs_timeout) > STAT_TIMEOUT) {
    internalStats[RSSI_OCS] = 0xFFFF;
    ocs_timeout = millis();
  }
}
void reset_rc_stats() {
  frsky_timeout = frsky.get_last_int_time(); // rc_timeout is in micros
  if ((micros() - frsky_timeout) > STAT_TIMEOUT * 1000) {
    internalStats[RSSI_FRSKY] = 0xFFFF;
    frsky.reset();
    frsky_timeout = micros();
  }
}
void reset_sbc_stats() {
  if ((millis() - sbc_timeout) > STAT_TIMEOUT) {
    internalStats[CPU_TEMP] = 0xFFFF;
    sbc_timeout = millis();
  }
}
void reset_batt1_stats()
{
  if ((millis() - batt1_timeout) > STAT_TIMEOUT) {
    powerStats[BATT1_CAPACITY] = 0xFFFF;
    powerStats[BATT1_CURRENT] = 0xFFFF;
    powerStats[BATT1_VOLTAGE] = 0xFFFF;
    batt1_timeout = millis();
  }
}
void reset_batt2_stats() {
  if ((millis() - batt2_timeout) > STAT_TIMEOUT) {
    powerStats[BATT2_CAPACITY] = 0xFFFF;
    powerStats[BATT2_CURRENT] = 0xFFFF;
    powerStats[BATT2_VOLTAGE] = 0xFFFF;
    batt2_timeout = millis();
  }
}
