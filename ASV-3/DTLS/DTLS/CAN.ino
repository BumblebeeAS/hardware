//===========================================
//    CAN functions
//===========================================
uint32_t last_heartbeat_time = 0;

/* can bus starting up function */
void CAN_init()
{
START_INIT:
  if (CAN_OK == CAN.begin(CAN_1000KBPS)) { // can baudrate = 1000k bps
    Serial.println("CAN BUS(DTLS) INIT: OK");
  }
  else {
    Serial.println("CAN BUS INIT: FAILED");
    Serial.println("CAN BUS: Reinitializing");
    delay(100);
    goto START_INIT;
  }
}

void CAN_mask() {

  /* Both masks needs to be set in mcp2515 */
  CAN.init_Mask(0, 0, 0x3ff);
  CAN.init_Mask(1, 0, 0x3ff);

  /* set 1 filter to receive from id 21 only */
  CAN.init_Filt(0, 0, 0x15);
  CAN.init_Filt(1, 0, 0x0E);
}

void CAN_publish() {
  // fill it up
}


uint32_t id = 0;
uint8_t len = 0;
uint8_t buf[8];

void CAN_parse_command() {
  if (CAN_MSGAVAIL == CAN.checkReceive()) { // check if data is coming
    CAN.readMsgBufID(&id, &len, buf); // udpate id and len to recceived
    /* If things goes correctly, only 1 id (22) should be received */
#ifdef DEBUG
      Serial.print("[DTLS] CAN data id: ");
      Serial.print(id);
      Serial.print(" length: ");
      Serial.println(len);
      Serial.print(" ");
      Serial.println("[DTLS] buffer data in Hex: ");
      Serial.println(buf[0], HEX);
      Serial.println(buf[1], HEX);
      Serial.println(buf[2], HEX);
      Serial.println(buf[3], HEX);
#endif
      if (id == CAN_DTLS_COMMAND) {
      int dtlsCommand = CAN.parseCANFrame(buf, 0, 1); //convert buf[0] to decimal
      switch (dtlsCommand) {
        case CLOSE:
          parse_steppers_action(CLOSING);
          CAN_send_status(FINISH_CLOSING);
#ifdef DEBUG
          Serial.print("[DTLS] Finish closing ");
          Serial.println(dtlsCommand);  // 1 for close, 2 for open
#endif
          break;
        case OPEN:
          parse_steppers_action(OPENING);
          CAN_send_status(FINISH_OPENING);
#ifdef DEBUG
          Serial.print("[DTLS] Finish opening ");
          Serial.println(dtlsCommand);  // 1 for close, 2 for open
#endif
          break;
        default:
          break;
      }
    }
    if (id == CAN_ACOUSTICS_ACTUATION) {
      int acousCommand = CAN.parseCANFrame(buf, 0, 1); //convert buf[0] to decimal
      switch(acousCommand) {
        case ACOUS_EXTEND: {
          extend_acous();
          break; }
        case ACOUS_RETRACT: {
#ifdef DEBUG
          Serial.print("retract");
#endif
          retract_acous();
          break; }
      default: 
        break;
      }
    }
  }
  CAN.clearMsg();
}

/*  send dtls execution status to can bus.
    0x01:  finish closing
    0x02:  finish opening
*/
void CAN_send_status(uint8_t dtlsStatus) {
  CAN.setupCANFrame(buf, 0, 1, dtlsStatus);
  CAN.sendMsgBuf(CAN_DTLS_STATS, 0, 1, buf);
}

void CAN_send_acous_status(int acous_status) {
  CAN.setupCANFrame(buf, 0, 1, acous_status);
  CAN.sendMsgBuf(CAN_ACOUSTICS_ACTUATION_STATS, 0, 1, buf);  
}

/*  send heartbeat of dtls board at 2Hz */
void CAN_send_heartbeat() {
  if (millis() >= last_heartbeat_time + 500) {
    CAN.setupCANFrame(buf, 0, 1, DTLS_HEARTBEAT);
    CAN.sendMsgBuf(CAN_HEARTBEAT, 0, 1, buf);
    last_heartbeat_time = millis();
  }
}

/*
   Truth table for CAN mask
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
