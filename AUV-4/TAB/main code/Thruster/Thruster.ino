//###################################################
//###################################################
//
//####     ####
//#  #     #  #      ######  ######## ########
//#  ####  #  ####   #    ## #  ##  # #  ##  #
//#     ## #     ##  ####  # #  ##  # #  ##  #
//#  ##  # #  ##  # ##     # #  ##  # #  ##  #
//#  ##  # #  ##  # #  ##  # #  ##  # ##    ##
//#     ## #     ## ##     # ##     #  ##  ##
// # ####   # ####   #######  #######   ####    
//
//
// BBAUV 4.0 Thruster Board
// Firmware Version : v1.0
// 
// Written by Yong Jie
// Edited by Titus   
// Change log v1.0: 
// Added actuation board to the thruster can lines
// 1. Forward actuation control message from SBC to thruster CAN line
// 2. Forward heartbeat from actuation board to main CAN line
//
//###################################################
//###################################################

#include <Arduino.h>
#include "VESC_Define.h"
#include "Define.h"
#include "auv_4.0_can_def.h"
#include <SPI.h>
#include <can.h>

//Declare CAN
MCP_CAN CAN(CAN_Chip_Select);
uint32_t id = 0;
uint8_t len = 0; //length of CAN message, taken care by library
uint8_t buf[8];  //Buffer for CAN message

MCP_CAN CAN_THRUSTER(CAN_THRUSTER_Chip_Select);
uint32_t id_thurster = 0;
uint8_t len_thurster = 0; //length of CAN message, taken care by library
uint8_t buf_thurster[8];  //Buffer for CAN message

//ESC 
int8_t vesc_id[8] = {1,2,3,4,5,6,7,8};
int32_t duty_cycle = 0;
float vesc_duty[8];

int vesc_duty_fb[8];
int vesc_curr_fb[8];
int32_t vesc_rpm_fb[8];
uint32_t vesc_timeout[8];
int vesc_voltage[8];

//Timer
uint32_t ESC_loop = 0;
uint32_t ESC_rate = 50;
uint32_t report_loop = 0;
uint32_t heartbeat_loop = 0;

void setup()
{
    Serial.begin(115200);

    //CAN init
    CAN_THRUSTER_init();
    CAN_init();
    
}

void loop()
{
    if(millis() - heartbeat_loop > HEARTBEAT_TIMEOUT){
        publishCAN_heartbeat(4);
        heartbeat_loop = millis();
    }
    checkCANmsg();
}

//===========================================
//
//        CAN FUNCTIONS
//
//===========================================

void CAN_init() {
START_INIT:
  if (CAN_OK == CAN.begin(CAN_1000KBPS)) {                   // init can bus : baudrate = 1000k
#if DEBUG_MODE == NORMAL
    Serial.println("CAN init ok!"); 
#endif
  }
  else {
#if DEBUG_MODE == NORMAL
    Serial.println("CAN init fail");
    Serial.println("Init CAN again");
    delay(1000);
#endif
    goto START_INIT;
  }
}

void CAN_THRUSTER_init() {
START_INIT:
  if (CAN_OK == CAN_THRUSTER.begin(CAN_500KBPS)) {                   // init can bus : baudrate = 1000k
#if DEBUG_MODE == NORMAL
    Serial.println("CAN_THRUSTER init ok!");
#endif
  }
  else {
#if DEBUG_MODE == NORMAL
    Serial.println("CAN_THRUSTER init fail");
    Serial.println("Init CAN_THRUSTER again");
    delay(1000);
#endif
    goto START_INIT;
  }
}

void checkCANmsg() {

    if (CAN_MSGAVAIL == CAN.checkReceive()) {
        int vesc_idx;
        CAN.readMsgBufID(&id, &len, buf);    // read data,  len: data length, buf: data buf
        switch (CAN.getCanId()) {
            case CAN_thruster_1:         
    			      for (int i = 0; i < 4; i++) {
                    vesc_idx = i;
                    vesc_duty[vesc_idx] = (int(CAN.parseCANFrame(buf, i * 2, 2)) - 3200)/3200.0;         
                    duty_cycle = int32_t(vesc_duty[vesc_idx] * 100000);  
                    buf_thurster[0] = (duty_cycle >> 24) & 0xFF;
                    buf_thurster[1] = (duty_cycle >> 16) & 0xFF;
                    buf_thurster[2] = (duty_cycle >> 8)  & 0xFF;
                    buf_thurster[3] = (duty_cycle)       & 0xFF;
                    //Send to vesc_id, extended frame, 4 bytes of data
                    CAN_THRUSTER.sendMsgBuf((CAN_PACKET_SET_DUTY << 8 | vesc_id[vesc_idx]), 1, 4, buf_thurster);
                }
                break;
            case CAN_thruster_2:
    			      for (int i = 0; i < 4; i++) {
                    vesc_idx = i + 4;
                    vesc_duty[vesc_idx] = (int(CAN.parseCANFrame(buf, i * 2, 2)) - 3200)/3200.0;    
                    duty_cycle = int32_t(vesc_duty[vesc_idx] * 100000);  
                    buf_thurster[0] = (duty_cycle >> 24) & 0xFF;
                    buf_thurster[1] = (duty_cycle >> 16) & 0xFF;
                    buf_thurster[2] = (duty_cycle >> 8)  & 0xFF;
                    buf_thurster[3] = (duty_cycle)       & 0xFF;
                    //Send to vesc_id, extended frame, 4 bytes of data
                    CAN_THRUSTER.sendMsgBuf((CAN_PACKET_SET_DUTY << 8 | vesc_id[vesc_idx]), 1, 4, buf_thurster);
                }
                break;  
            case CAN_ACT_CONTROL: // receive manipulator control, fwd to actuation board
                uint8_t maniControl[8] = {CAN.parseCANFrame(buf, 0, 1)};
                CAN_THRUSTER.sendMsgBuf(CAN_ACT_CONTROL, 0, 1, maniControl);
            default:
                break;
        }
        CAN.clearMsg();
    }    

    if (CAN_MSGAVAIL == CAN_THRUSTER.checkReceive()) {
          CAN_THRUSTER.readMsgBufID(&id_thurster, &len_thurster, buf_thurster);    // read data,  len: data length, buf: data buf
          int esc_id = CAN_THRUSTER.getCanId() & 0xFF;      
          esc_id = esc_id - 1;
//          Serial.println(esc_id);
          int command = CAN_THRUSTER.getCanId()>>8 & 0xFF;
          switch (command) {
              case CAN_PACKET_STATUS:
                  vesc_rpm_fb[esc_id] = CAN_THRUSTER.parseCANFrame(buf_thurster, 0, 4); 
                  vesc_curr_fb[esc_id] = CAN_THRUSTER.parseCANFrame(buf_thurster, 4, 2);  
                  vesc_duty_fb[esc_id] = CAN_THRUSTER.parseCANFrame(buf_thurster, 6, 2);   
  //                vesc_timeout[esc_id] = millis();
                  break;
              case CAN_PACKET_STATUS_5:
                  vesc_voltage[esc_id] = CAN_THRUSTER.parseCANFrame(buf_thurster, 4, 2);  
  //                vesc_timeout[esc_id] = millis();
                  break;            
              default:
                if (CAN_THRUSTER.getCanId() == CAN_ACT_HEARTBEAT_THRUSTER) {
//                  Serial.println("Recevied actuation hb");
                  uint8_t HB[1] = { CAN_ACT_HEARTBEAT };  //CAN_ACT_HEARTBEAT
                  CAN.sendMsgBuf(CAN_HEARTBEAT, 0, 1, HB); //CAN_HEARTBEAT
                }
                break;
          }
        CAN_THRUSTER.clearMsg();
    }
}

void publishCAN_heartbeat(int device_id)
{
    buf[0] = device_id;
    CAN.sendMsgBuf(CAN_HEARTBEAT, 0, 1, buf);
}

void publishCAN_RPM()
{
    CAN.setupCANFrame(buf, 0, 2, vesc_rpm_fb[0]);
    CAN.setupCANFrame(buf, 2, 2, vesc_rpm_fb[1]);
    CAN.setupCANFrame(buf, 4, 2, vesc_rpm_fb[2]);
    CAN.setupCANFrame(buf, 6, 2, vesc_rpm_fb[3]);    
    CAN.sendMsgBuf(CAN_THT_STAT_1, 0, 8, buf);
    CAN.setupCANFrame(buf, 0, 2, vesc_rpm_fb[4]);
    CAN.setupCANFrame(buf, 2, 2, vesc_rpm_fb[5]);
    CAN.setupCANFrame(buf, 4, 2, vesc_rpm_fb[6]);
    CAN.setupCANFrame(buf, 6, 2, vesc_rpm_fb[7]);    
    CAN.sendMsgBuf(CAN_THT_STAT_2, 0, 8, buf);
}

void publishCAN_DUTY()
{
    CAN.setupCANFrame(buf, 0, 2, vesc_duty_fb[0]);
    CAN.setupCANFrame(buf, 2, 2, vesc_duty_fb[1]);
    CAN.setupCANFrame(buf, 4, 2, vesc_duty_fb[2]);
    CAN.setupCANFrame(buf, 6, 2, vesc_duty_fb[3]);    
    CAN.sendMsgBuf(CAN_THT_PWM_1, 0, 8, buf);
    CAN.setupCANFrame(buf, 0, 2, vesc_duty_fb[4]);
    CAN.setupCANFrame(buf, 2, 2, vesc_duty_fb[5]);
    CAN.setupCANFrame(buf, 4, 2, vesc_duty_fb[6]);
    CAN.setupCANFrame(buf, 6, 2, vesc_duty_fb[7]);    
    CAN.sendMsgBuf(CAN_THT_PWM_2, 0, 8, buf);
}
