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
