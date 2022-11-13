//===========================================
//
//    CAN functions
//
//===========================================
uint32_t id = 0;
uint8_t len = 0;
uint8_t buf[8];
uint32_t last_heartbeat_time = millis();
void CAN_send_heartbeat() {
  if (millis() >= last_heartbeat_time + HB_TIMEOUT) {
    CAN.setupCANFrame(buf, 0, 1, BALL_HEARTBEAT);
    CAN.sendMsgBuf(CAN_HEARTBEAT, 0, 1, buf);
    last_heartbeat_time = millis();
  }
}

void CAN_parse_command() {
  if (CAN_MSGAVAIL == CAN.checkReceive()) { // check if data is coming
    CAN.readMsgBufID(&id, &len, buf); // udpate id and len to recceived
    if (id == CAN_BALL_SHOOTER) {
      int command = CAN.parseCANFrame(buf, 0, 1);
      switch(command) {
        case COCK: {          // Takes abt 20s
          Serial.println("recieve COCK");
          CAN_acknowledgement(ACK_COCK);
          retract_act();
          extend_act();
          CAN_acknowledgement(COCK);
          break;
        }
        case LOAD: {
          Serial.println("recieve LOAD");
          CAN_acknowledgement(ACK_LOAD);
          reload_drum();
          CAN_acknowledgement(LOAD);
          break;
        }
        case FIRE: {
          Serial.println("recieve FIRE");
          CAN_acknowledgement(ACK_FIRE);
          release_latch();
          CAN_acknowledgement(FIRE);
          break;
        }
        default:
          Serial.println("wrong command");
      }
    }
    CAN.clearMsg();     // clear buffer to prevent execution of double msgs
  }
}

void CAN_acknowledgement(int command) {
  CAN.setupCANFrame(buf, 0, 1, command);
  CAN.sendMsgBuf(CAN_BALL_SHOOTER_STATS, 0, 1, buf);  
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
    /* Both masks needs to be set in mcp2515 */
  CAN.init_Mask(0, 0, 0);
  CAN.init_Mask(1, 0, 0);

//  /* set 1 filter to receive from id 21 only */
//  CAN.init_Filt(0, 0, 0x15);
//  CAN.init_Filt(1, 0, 0x0E);
}
