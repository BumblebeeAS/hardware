
void move_one_stepper(TMC2209Stepper s, int dir, int threshold, int spd, int t) {
  uint32_t ms = millis();
  uint32_t start_time = millis();
  bool STALLED = false;
  
  switch (dir)
  {
    case 0: //moving forward
      s.VACTUAL(spd);
      s.VACTUAL(spd);
      s.VACTUAL(spd);     // start stall guarding only after 10 microsteps 
      s.VACTUAL(spd);
      s.VACTUAL(spd);
      s.VACTUAL(spd);
      s.VACTUAL(spd);
      s.VACTUAL(spd);
      s.VACTUAL(spd);
      s.VACTUAL(spd);
//      Serial.println(threshold);
      while ((ms - start_time) < t && !STALLED) { 
        STALLED = stall_guard(s, threshold);
        ms = millis();
      } 
      
      // loop came to a stop 
      s.VACTUAL(0);
      break;
      
    case 1: //moving backword
      s.VACTUAL(-spd);
      s.VACTUAL(-spd);
      s.VACTUAL(-spd);      // start stall guarding only after 10 microsteps
      s.VACTUAL(-spd);
      s.VACTUAL(-spd);
      s.VACTUAL(-spd);
      s.VACTUAL(-spd);
      s.VACTUAL(-spd);
      s.VACTUAL(-spd);
      s.VACTUAL(-spd);
      
      while ((ms - start_time) < t && !STALLED) { 
        STALLED = stall_guard(s, threshold);
        ms = millis();
      } 
      
      s.VACTUAL(0);
      break;
      
    default: // stop
      s.VACTUAL(0);
//      Serial.println("Error direction. Stepper stopped");
  }
}

bool stall_guard(TMC2209Stepper s, int stall_value) {   
    int load = (int) s.SG_RESULT();
//    Serial.print("Load Value: ");
//    Serial.println(load);
    if (load > 40 && load < stall_value )
    {
      s.VACTUAL(0);
      return true; // break from the function if stall deteced
    }
    return false;
}

// Acoustics actaution
void extend_acous() {
  digitalWrite(ACOUS_DIR_PIN, HIGH);     // Not sure if dir is correct, to find out at sea trial lol 
  digitalWrite(ACOUS_PWM_PIN, HIGH);
  delay(ACOUS_DELAY);      // Didn't wire up limit switches so no idea if its done. Arbitary delay to send ok msg.
  digitalWrite(ACOUS_PWM_PIN, LOW);
  CAN_send_acous_status(1);
  
}

void retract_acous() {
  digitalWrite(ACOUS_DIR_PIN, LOW);
  digitalWrite(ACOUS_PWM_PIN, HIGH);
  delay(ACOUS_DELAY);
  digitalWrite(ACOUS_PWM_PIN, LOW);
  CAN_send_acous_status(2);

}
