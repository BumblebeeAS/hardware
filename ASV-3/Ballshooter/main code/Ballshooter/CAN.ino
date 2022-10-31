//===========================================
//
//    CAN functions
//
//===========================================

void CAN_send_heartbeat() {

}

void CAN_parse_command() {
  
}

void CAN_acknowledgement() {
  
}
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
}
