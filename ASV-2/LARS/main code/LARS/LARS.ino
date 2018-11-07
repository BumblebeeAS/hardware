#include "can.h"
#include "can_asv_defines.h"
#include"SmartMotor.h"
#include "defines.h"

//#define _GET_POSN_10HZ_
#define _DEBUG_MOTOR_STEPS_

//#define _SERIAL_

MCP_CAN CAN(8);
uint32_t id = 0;
uint8_t len = 0;
uint8_t buf[8] = { 0 };

int32_t timeout_SBC;

SmartMotor lars;
int mode;
int temp_mode;
int32_t posn;
int32_t motor_position;
int32_t motor_goal;
int32_t motor_velocity;
int32_t motor_acceleration; 
int32_t motor_trajectory_flag;
uint32_t position_loop;
uint32_t velocity_loop;
uint32_t request_loop;
uint32_t stats_loop;
uint32_t recover_loop;
uint32_t prox_loop;
int mode_counter = 0;

int launched_flag = 0;
int recovered_flag = 0;
int estop_flag = 0;

int sign_posn_meters = 0;
int sign_motor_position_meters = 0;

uint8_t read_buffer[10];
uint8_t read_ctr = 0;

uint16_t posn_meters = 0;
uint16_t motor_position_meters = 0;

uint8_t prox_state;
int e_stop_counter = 0;
int recovery_step_counter = 0;
int recovery_in_progress = 0;

uint32_t heartbeat_loop;
uint32_t can_LARS_stats_loop;
uint32_t stats_serial_loop;
uint32_t step_start;

void setup() {
  Serial.begin(9600);
  pinMode(PROXIMITY_SENSOR, INPUT);
  CAN_init();

  timeout_SBC = millis();

  // put your setup code here, to run once:
  lars.init();
  lars.startup();
  posn = 0;
  motor_goal = 0;
  motor_trajectory_flag = 0;
  
  lars.setAcceleration(4000);
  lars.setVelocity(100000);
  lars.setPosition(0);

  stats_loop = millis();
  heartbeat_loop = millis();
  can_LARS_stats_loop = millis();
  stats_serial_loop = millis();

  position_loop = millis();
  velocity_loop = millis();
  request_loop = millis();
  recover_loop = millis();
  prox_loop = millis();

  launched_flag = 0;
  recovered_flag = 1;

  estop_flag = 0;
}

void loop() {
	// put your main code here, to run repeatedly:
	lars.readMessage();

	//===============================
	//    SEND REQUEST
	//===============================

#ifdef _GET_POSN_10HZ_
  // Request position at 10Hz
	if ((millis() - position_loop) > POSN_TIMEOUT)
	{
		lars.requestPosition();
		motor_position = lars.getPosition();
		Serial.print("POSN: ");
		Serial.println(motor_position);
		position_loop = millis();
	}
#else
  // Request posn, velocity, acc every 100ms
	if ((millis() - request_loop) > REQUEST_TIMEOUT)
	{
		mode_counter++;
		switch (mode_counter)
		{
		case 1:
			lars.requestPosition();
			break;
		case 2:
			lars.requestGoal();
			break;
		case 3:
			lars.requestTrajectoryFlag();
			break;
		case 10:
			mode_counter = 0;
		default:
			break;
		}
		request_loop = millis();
	}
#endif

// Debug using Serial Monitor to set Motor steps



#ifdef _SERIAL_
	if (Serial.available())
	{
#ifdef _DEBUG_MOTOR_STEPS_
		//===============================
		//    SEND POSITION CMD
		//===============================
		// Type in serial monitor to set absolute position
		//while (Serial.available())
		//{
			uint8_t input = Serial.read();

			if (input == 'S')
			{
				mode = MODE_ESTOP;
				read_ctr = 0;
				Serial.println("STOPY");
				//break;
			}
			else if (input == 'G')
			{
				mode = MODE_RESUME;
				read_ctr = 0;
				//break;
			}

			else if (input == 'M')
			{
				posn = 560000;
				mode = MODE_STEP;
				read_ctr = 0;
				//break;
			}


			else if ((input == '\r') || (input == '\n'))
			{
				if (read_ctr != 0)
				{
					read_buffer[read_ctr] = '\0';
					posn = strtol(read_buffer, NULL, 10);
					mode = MODE_STEP;
					read_ctr = 0;
				}
			}
			else {
				read_buffer[read_ctr] = input;
				read_ctr++;
			}
		
#else
		// Debug mode operations using Serial (only for Launch(1) and Recover(2))
		uint8_t input = Serial.read();

		if (input == 'S')
		{
			mode = MODE_ESTOP;
		}
		else if (input == 'G')
		{
			//resume();
			//mode = MODE_IDLE;
			mode = MODE_RESUME;
		}
		else if (input == '2')
		{
			if ((launched_flag == 0) & (recovered_flag == 1)) {
				mode = MODE_LAUNCH;
			}
		}
		else if (input == '3')
		{
			if ((launched_flag == 1) & (recovered_flag == 0)) {
				mode = MODE_RECOVER;
			}
		}
	#endif
	}
#else
	// Check Motor command using CAN
	checkCanMsg();
#endif

	// Change motor operation based on mode commanded, then stop
	/*Serial.print("stop: ");
	Serial.print(estop_flag);
	Serial.print("   mode: ");
	Serial.println(mode);*/

	//if (checkProximity()) {
	//	Serial.println("KILLLLLL");
	//	mode = MODE_ESTOP;
	//}
	
	if (mode == MODE_ESTOP) 
	{
		if (!estop_flag) estop();
		//Serial.print("estop flag: ");
		//Serial.println(estop_flag);
	}
	else
	{
		
		if (estop_flag) // unset estop flag, resume motor
		{
			resume();
			Serial.println("RESUMED");
		}

		// check 3 modes
		if (mode == MODE_LAUNCH)
		{
			launch();
			
			mode = MODE_IDLE;
		}
		
		else if (mode == MODE_RECOVER)
		{
			if (recovery_step_counter == 0) {				// First step: Reel back to below ASV)
				recovery_in_progress = 1;
				init_recover();
				recover_loop = millis();
			}
			else if (checkProximity() | (recovery_step_counter == (RECOVERY_STEPS + 1))) {	// Final step: AUV recovered
				recovery_in_progress = 0;
				end_recover();
				mode = MODE_IDLE;
			}
			//Serial.print("recovery in progress ");
			//Serial.println(recovery_in_progress);
		}

		else if (mode == MODE_STEP)
		{
			Serial.println("STEPPY");
			moveTo();
			step_start = millis();
			mode = MODE_STEPPING;
		}
		
		else if (mode == MODE_RESUME)
		{
			mode = (recovery_in_progress) ? MODE_RECOVER : MODE_IDLE;
		}

		else if (mode == MODE_STEPPING)
		{
			if (checkProximity())
			{
				lars.setRun(false);
				mode = MODE_IDLE;
			}

			if ((millis() - step_start > STEP_WAIT) & !motor_trajectory_flag)
			{
				mode = MODE_IDLE;
			}

		}

		// clear recovery settings
		// in case recovery mode was killed halfway
		if (mode != MODE_RECOVER & recovery_in_progress)
		{
			recovery_step_counter = 0;
			recovery_in_progress = 0;
			recovered_flag = 1;
			launched_flag = 0;
		}

	}

	// In recovery mode, move every 2s & motor not moving
	if ((!estop_flag) & recovery_in_progress & ((millis() - recover_loop) > RECOVER_PERIOD) & (!motor_trajectory_flag))
	{
		Serial.print("Recovering Cycle: ");
		Serial.println(recovery_step_counter);
		smallMoveTo();
		recovery_step_counter += 1;
		recover_loop = millis();
	}

	// Update Motor stats every 100ms
	if ((millis() - stats_loop) > REQUEST_TIMEOUT)
	{
		motor_position = lars.getPosition();
		motor_goal = lars.getGoal();
		motor_trajectory_flag = lars.getTrajectoryFlag();
		stats_loop = millis();
	}
	
	// Send CAN Heartbeat
	if ((millis() - heartbeat_loop) > HB_LOOP)
	{
		publishCAN_heartbeat();
		heartbeat_loop = millis();
	}

	// Send CAN_LARS_stats and print stats to serial
	if ((millis() - can_LARS_stats_loop) > CAN_LARS_STATS_LOOP)
	{
		publishCAN_LARS_stats();
		can_LARS_stats_loop = millis();
	}

	//// Stop Motor if SBC HB drops
	//if ((millis() - timeout_SBC) > SBC_HB_TIMEOUT) {
	//	estop();
	//	mode = MODE_IDLE;
	//}

	// Debug Serial
	if ((millis() - stats_serial_loop) > STATS_SERIAL_LOOP) {
		motor_stats_serial();
		stats_serial_loop = millis();
	}
}

//======= LARS =========

// E-Stop the motor
void estop()
{
  lars.setRun(false);
  estop_flag = 1;
  Serial.println("ESTOP");
}

void resume() {
  lars.setPosition(motor_goal);
  //Serial.println(motor_goal);
  lars.setRun(true);
  estop_flag = 0;
  Serial.println("Resuming Trajectory");
}

// Unreel to POSN_LAUNCH at VEL_LAUNCH
void launch()
{
  Serial.println("Launching");
  posn = POSN_LAUNCH;
  lars.setVelocity(VEL_LAUNCH);
  lars.setPosition(posn);
  lars.setRun(true);
  //delay(1000);
  launched_flag = 1;
  recovered_flag = 0;
  
}

// Reel to POSN_RECOVER at VEL_RECOVER

// Then continue to reel at steps of POSN_RECOVER_STEP, and poll for proximity sensor
void init_recover()
{
	Serial.println("Start Recovering");
	posn = POSN_RECOVER;
	lars.setVelocity(VEL_NORMAL);
	lars.setPosition(posn);
	Serial.println(posn);
	lars.setRun(true);
	recovery_step_counter += 1;
}

void end_recover()
{
	lars.setRun(false);  // Stop motor
	mode = MODE_IDLE;
	recovery_step_counter = 0;
	recovered_flag = 1;
	launched_flag = 0;
	Serial.println("Recovery Complete");
}

void moveTo()
{
  Serial.print("Moving to target: ");
  Serial.println(posn);
  lars.setVelocity(VEL_NORMAL);
  lars.setPosition(posn);
  lars.setRun(true);
}

void smallMoveTo()
{
	posn -= POSN_RECOVER_STEP;
	lars.setVelocity(VEL_RECOVER);
	Serial.print("Next step: ");
	Serial.println(posn);

	//if (posn < 0)
	//	posn = 0;
	lars.setPosition(posn);
	lars.setRun(true);
}


bool checkProximity()
{ 
  // Check Proximity sensor
  prox_state = digitalRead(PROXIMITY_SENSOR);
  e_stop_counter += prox_state;
  if (e_stop_counter > 20) {
	  e_stop_counter = 0;
	  Serial.println("DOCKED");
	  return true;
  }
  else
	  return false;
}

//======= CAN =========

void CAN_init()
{
START_INIT:
  if (CAN_OK == CAN.begin(CAN_1000KBPS))                   // init can bus : baudrate = 1000Kbps
  {
    Serial.println("CAN BUS: OK");
}
  else
  {
    Serial.println("CAN BUS: FAILED");
    Serial.println("CAN BUS: Reinitializing");
    delay(1000);
    goto START_INIT;
  }
  Serial.println("INITIATING TRANSMISSION...");
}

void publishCAN_LARS_stats(){
  if (posn < 0) {
	  sign_posn_meters = 1;
	  posn_meters = posn / POSN_SCALE * -1;		// negate to send magnitude
  }
  else {
	  sign_posn_meters = 0;
	  posn_meters = posn / POSN_SCALE;
  }

  if (motor_position < 0) {
	  sign_motor_position_meters = 1;
	  motor_position_meters = motor_position / POSN_SCALE * -1;		// negate to send magnitude
  }
  else {
	  sign_motor_position_meters = 0;
	  motor_position_meters = motor_position / POSN_SCALE;
  }

  buf[4] = motor_trajectory_flag | (launched_flag << 1) | (recovered_flag << 2) | (estop_flag << 3) | (sign_posn_meters << 4) | (sign_motor_position_meters << 5);

  Serial.print("Negative: ");
  Serial.println(sign_posn_meters);
  Serial.print("target sending meters: ");
  Serial.println(posn_meters);
  Serial.print("current sending meters: ");
  Serial.println(motor_position_meters);

  CAN.setupCANFrame(buf, 0, 2, posn_meters);
  CAN.setupCANFrame(buf, 2, 2, motor_position_meters);
  CAN.sendMsgBuf(CAN_LARS_stats, 0, 5, buf);
}

void publishCAN_heartbeat() {
  buf[0] = HEARTBEAT_LARS;
  CAN.sendMsgBuf(CAN_heartbeat, 0, 1, buf);
}

boolean checkCanMsg() {
  //Serial.println("checking CAN");
	if (CAN_MSGAVAIL == CAN.checkReceive()) {
		CAN.readMsgBufID(&id, &len, buf);    // read data,  len: data length, buf: data buf
		boolean mine = false;
		switch (id) {
		case CAN_LARS:
			// Check checksum
			Serial.println("CAN_LARS");
			if (isCrcCorrect()) {
				temp_mode = CAN.parseCANFrame(buf, 0, 1);
				Serial.print("temp_mode = ");
				Serial.println(temp_mode);
				if (temp_mode == MODE_LAUNCH) {
					if ((launched_flag == 0) & (recovered_flag == 1)) {
						mode = MODE_LAUNCH;
					}
				}
				else if (temp_mode == MODE_RECOVER) {
					if ((launched_flag == 1) & (recovered_flag == 0)) {
						mode = MODE_RECOVER;
					}
				}
				else {
					mode = temp_mode;
				}
				if (mode == MODE_STEP)
				{
					posn = ((CAN.parseCANFrame(buf, 2, 1) << 8) | CAN.parseCANFrame(buf, 1, 1)) * POSN_SCALE;
					if ((CAN.parseCANFrame(buf, 3, 1)) == 1) {
						posn *= -1;					// negate to obtain negative step command
					}
					Serial.print("posn received: ");
					Serial.println(posn);
				}
				mine = true;
			}
			break;
		case CAN_heartbeat:
			temp_mode = CAN.parseCANFrame(buf, 0, 1);
			if (temp_mode == HEARTBEAT_Cogswell)
				//Serial.println("SBC Heartbeat");
				timeout_SBC = millis();
			mine = true;
			break;
		default:
			//Serial.println("Others");
			break;
		}
	CAN.clearMsg();
	return mine;
	}
  else
    return false;
}

bool isCrcCorrect()
{
	if (buf[4] == buf[0] ^ buf[1] ^ buf[2] ^ buf[3]) {
		Serial.println("CRC ok");
		return true;
	}
	else {
		Serial.print("CRC error: ");
		Serial.print(buf[3]);
		Serial.print(" VS 0: ");
		Serial.print(buf[0]);
		Serial.print(" 1: ");
		Serial.print(buf[1]);
		Serial.print(" 2: ");
		Serial.println(buf[2]);
		return false;
	}
}

void motor_stats_serial()
{
	Serial.print("Target Position: ");
	Serial.println(posn);
	Serial.print("Current Posistion: ");
	Serial.println(motor_position);
	Serial.print("In Progess: ");
	Serial.println(motor_trajectory_flag);
}
