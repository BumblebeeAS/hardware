
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
