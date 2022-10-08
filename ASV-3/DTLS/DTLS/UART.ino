// -------------------------------
//  tmc2209 in uart mode
//  - step & dir pin must be pulled to ground
//  - uart pins must be physically shorted
//  - DIAG pin pulled high during stall (SGTHRS)
//  - single line rx tx transmission (1k resistor)
//  - talk to me baby
// -------------------------------

void check_all_UART() {
  Serial.print(F(" Testing UART of Stepper 0: ...  "));
  stepper_talk(stepper0);
  Serial.print(F(" Testing UART of Stepper 1: ...  "));
  stepper_talk(stepper1);
  Serial.print(F(" Testing UART of Stepper 2: ...  "));
  stepper_talk(stepper2);
  Serial.print(F(" Testing UART of Stepper 3: ...  "));
  stepper_talk(stepper3);
  Serial.println(F("UART test complete."));
}

bool stepper_talk(TMC2209Stepper s) {
  auto versionS = s.version();
  if (versionS == 0x21) {
    Serial.println("Success.");
    return true; //stepper talking through UART
  }

  else {
    uint8_t result = s.test_connection();
    if (result) {
      Serial.println("Failed!");
      Serial.print("Likely cause: ");

      switch (result) {
        case 1: Serial.println("loose connection"); break;
        case 2: Serial.println("no power"); break;
      }
      return false;
    }
    else {
      Serial.println("unknown error.");
      return false;
    }
  }
}
