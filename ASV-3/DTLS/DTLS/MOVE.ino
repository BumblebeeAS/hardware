/* move one stepper, just to testing.*/
void move_one_stepper(TMC2209Stepper s, int dir, int threshold, int spd, int t) {
  // this code will block other steppers from moving
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

/* main function to move the steppers */
void parse_steppers_action(int dir) {
  bool STALLED = false;
//  uint32_t now = millis();
//  uint32_t start_time = millis();

  if (dir == FORWARD) { // moving forward
     all_stepper_move(steppers,2, gSpeed);
  } else {  // moving backward
    all_stepper_move(steppers,2, -gSpeed);
  }

  while (!STALLED) {
     STALLED = stall_guard(stepper0, STALL_THRESHOLD[0])||stall_guard(stepper1, STALL_THRESHOLD[1]);
  } // loop only exit when all motors stalled
  
  // a safety function that does nothing fundamentally
  all_stepper_stop(steppers, 2);
}

/* move all steppers */
void all_stepper_move(TMC2209Stepper *s, int num_of_steppers,int spd) {
  for (int i = 0; i < num_of_steppers; i ++) {
    s[i].VACTUAL(spd);
    s[i].VACTUAL(spd);
    s[i].VACTUAL(spd);
  }
}

/* stop all steppers */
void all_stepper_stop(TMC2209Stepper *s, int num_of_steppers) {
  for (int i = 0; i < num_of_steppers; i ++) {
    s[i].VACTUAL(0);
    s[i].VACTUAL(0);
    s[i].VACTUAL(0);
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
