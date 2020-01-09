#include "st_can.h"

uint8_t CAN_recvMsg_buff[32] = {0};	//CAN recvmsgbuffer
uint32_t RxFifo = 10;
uint32_t can_publish_loop_gyro=10;
uint32_t can_publish_loop_mag=10;
uint32_t can_publish_loop_acc=10;
uint32_t can_publish_loop_sens=10;
uint32_t can_publish_loop_uptime=0;

uint32_t can_heartbeat_loop=0;
uint8_t CAN_SEND=0;
uint16_t uptime=0;
uint8_t signature=0;

void publishCAN_message(void){
	if(HAL_GetTick()-can_publish_loop_gyro>(1000/stbUP[GYRO])){
	uint8_t Msg[6]={(uint8_t)internalStats[IMU_G_X]>>8,(uint8_t)internalStats[IMU_G_X],(uint8_t)internalStats[IMU_G_Y]>>8,(uint8_t)internalStats[IMU_G_Y],(uint8_t)internalStats[IMU_G_Z]>>8,(uint8_t)internalStats[IMU_G_Z]};
	CAN_SendMsg(CAN_STB_GYRO,Msg,6);
	can_publish_loop_gyro=HAL_GetTick();
	}

	if(HAL_GetTick()-can_publish_loop_gyro>(1000/stbUP[ACC])){
		uint8_t Msg[6]={(uint8_t)internalStats[IMU_A_X]>>8,(uint8_t)internalStats[IMU_A_X],(uint8_t)internalStats[IMU_A_Y]>>8,(uint8_t)internalStats[IMU_A_Y],(uint8_t)internalStats[IMU_A_Z]>>8,(uint8_t)internalStats[IMU_A_Z]};
		CAN_SendMsg(CAN_STB_ACC,Msg,6);
		can_publish_loop_acc=HAL_GetTick();
	}

		if(HAL_GetTick()-can_publish_loop_mag>(1000/stbUP[MAG])){
		uint8_t Msg[6]={internalStats[IMU_M_X]>>8,internalStats[IMU_M_X],internalStats[IMU_M_Y]>>8,internalStats[IMU_M_Y],internalStats[IMU_M_Z]>>8,internalStats[IMU_M_Z]};
		CAN_SendMsg(CAN_STB_MAG,Msg,6);
		can_publish_loop_mag=HAL_GetTick();
		}

	if(HAL_GetTick()-can_publish_loop_sens>(1000/stbUP[SENSOR])){
	uint8_t Msg2[8]={internalStats[INT_PRESS]>>8,internalStats[INT_PRESS],internalStats[HUMIDITY]>>8,internalStats[HUMIDITY],internalStats[ST_TEMP]>>8,internalStats[ST_TEMP],internalStats[EXT_PRESS]>>8,internalStats[EXT_PRESS]};
	CAN_SendMsg(CAN_STB_SENS,Msg2,8);
	can_publish_loop_sens=HAL_GetTick();
	}
	}

void publishCAN_heartbeat(void){
	if(HAL_GetTick()-can_heartbeat_loop>100){
		uint8_t Msg[1]={HEARTBEAT_Telemetry};
		CAN_SendMsg(CAN_HEARTBEAT,Msg,1);
		can_heartbeat_loop=HAL_GetTick();
	}
}

void publishCAN_uptime(void){
	if(HAL_GetTick()-can_publish_loop_uptime>1000){
		uptime=uptime+1;
		uint8_t MSG[3]={uptime>>8,uptime,signature};
		CAN_SendMsg(CAN_STB_STAT,MSG,1);
		can_publish_loop_uptime=HAL_GetTick();
	}
}

void update_can(void){
	RxFifo=CAN_CheckReceive();
	if(RxFifo!=10){
		CAN_RecvMsg(RxFifo,CAN_recvMsg_buff);
		uint16_t CAN_id =(uint16_t) CAN_GetId();
		switch(CAN_id){
		case CAN_HEARTBEAT:{
			uint8_t device= CAN_recvMsg_buff[0];
			boardHB_timeout[device]=HAL_GetTick();
			break;
		}
		case CAN_BATT1_STAT:{
			powerStats[BATT1_CURRENT]=(CAN_recvMsg_buff[1]<<8|CAN_recvMsg_buff[0]);
			powerStats[BATT1_VOLTAGE]=(CAN_recvMsg_buff[3]<<8|CAN_recvMsg_buff[2]);
			break;
		}
		case CAN_PMB1_STAT:{
			internalStats[PMB1_PRESS]=CAN_recvMsg_buff[4];
			internalStats[PMB1_TEMP]=CAN_recvMsg_buff[3];
			powerStats[BATT1_CAPACITY]=CAN_recvMsg_buff[2];
			break;
		}
		case CAN_BATT2_STAT:{
			powerStats[BATT2_CURRENT]=(CAN_recvMsg_buff[1]<<8|CAN_recvMsg_buff[0]);
			powerStats[BATT2_VOLTAGE]=(CAN_recvMsg_buff[3]<<8|CAN_recvMsg_buff[2]);
			break;
		}
		case CAN_PMB2_STAT:{
			internalStats[PMB2_PRESS]=CAN_recvMsg_buff[4];
			internalStats[PMB2_TEMP]=CAN_recvMsg_buff[3];
			powerStats[BATT2_CAPACITY]=CAN_recvMsg_buff[2];
			break;
		}
		case CAN_STB_CONFIG:{
			for (uint8_t i=0;i<STB_UP_COUNT;i++){
				stbUP[i]=CAN_recvMsg_buff[i];
			}
			uint8_t MSG[8]={stbUP[7],stbUP[6],stbUP[5],stbUP[4],stbUP[3],stbUP[2],stbUP[1],stbUP[0]};
			CAN_SendMsg(CAN_STB_UP,MSG,8);
		}
	}
		for (uint8_t i=0;i<HB_COUNT;i++){
			if(boardHB_timeout[i]>HB_TIMEOUT){
			boardHB[i]=0xFFFF;
			}
		else{
			boardHB[i]=0x1111;
			}
		}
	}
}

