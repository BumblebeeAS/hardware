// Roboteq.h

#include <can.h>
#include <can_defines.h>

#define ROBOTEQ_CAN1_SEND_INDEX 0x605
#define ROBOTEQ_CAN1_REPLY_INDEX 0x585
#define ROBOTEQ_CAN2_SEND_INDEX 0x606
#define ROBOTEQ_CAN2_REPLY_INDEX 0x586

#define CCS_COMMAND	2
#define CCS_QUERY	4
/********** Motor Controllers Object Dictionary ********/
// COMMANDS
#define INDEX_SET_MOTOR	0x2000
#define INDEX_SET_VELOCITY	0X2002
#define INDEX_SET_NEXT_VELOCITY	0X2014
#define INDEX_EMERGENCY_SHUTDOWN 0x200C
#define INDEX_RELAESE_SHUTDOWN 0x200D

// QUERIES
#define INDEX_READ_MOTOR_AMPS 0x2100
#define INDEX_READ_FAULT_FLAGS 0x2112
#define INDEX_READ_MOTOR_STATUS_FLAGS 0x2122
#define INDEX_READ_ACTUAL_MOTOR_COMMAND 0x2101
#define INDEX_READ_BATTERY_AMPS 0x210C
#define INDEX_READ_BATTERY_VOLTS 0x210D

#define ESC1_ID 1
#define ESC2_ID 1

typedef struct RoboteqStats{
	int16_t motor_current1;
	int16_t motor_current2;
	int16_t battery_current;
  int16_t battery_volt;
	int16_t motor_comand1;
	int16_t motor_comand2;
	uint8_t fault_flags;
	uint8_t motor_status_flags1;
	uint8_t motor_status_flags2;
} RoboteqStats;

class Roboteq
{
private:
	MCP_CAN* CAN;
	uint8_t _buf[8];
	uint32_t _id;
	uint8_t _len;
	RoboteqStats stats;

	uint8_t _ch;
	uint16_t can_send_idx;
	uint16_t can_reply_idx;

	void sendCANmsg(uint16_t index, INT8U subidx, INT8U ccs, INT8U len, INT32U data);
	
	uint16_t getReplyIndex();
	void decodeReply(uint16_t index);

 public:
	Roboteq(MCP_CAN *canptr, uint16_t can_send);
	void init();

	RoboteqStats getRoboteqStats();

	void setMotorSpeed(int32_t speed, uint8_t channel);
	void requestMotorAmps();
	void requestFaultFlags();
	void requestMotorStatusFlags(uint8_t ch);
	void readRoboteqReply(uint32_t id, uint8_t len, uint8_t *buf);
	void readRoboteqReply_fromCAN();

	// Unused
	void kill();
	void unkill();
	void requestBatteryAmps();
	void requestBatteryVolts();
	void requestMotorCommand();
};


