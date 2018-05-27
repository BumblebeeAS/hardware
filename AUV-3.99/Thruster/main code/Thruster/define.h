#ifndef _DEFINES_H
#define _DEFINES_H

#define CAN_Chip_Select 8

// == Thruster mapping ==
// AUV	| Board	|  PWM
//	1	|	1	|	3
//	2	|	5	|	9
//	3	|	2	|	5
//	4	|	6	|	10
//	5	|	7	|	11
//	6	|	3	|	6
//	7	|	4	|	7
//	8	|	8	|	12

#define THRUSTER_1 3
#define THRUSTER_2 9

#define THRUSTER_3 5
#define THRUSTER_4 10

#define THRUSTER_5 11
#define THRUSTER_6 6

#define THRUSTER_7 7
#define THRUSTER_8 12

#define FORWARD_MAX 1000
#define FORWARD_MIN 1450
#define REVERSE_MIN 1550
#define REVERSE_MAX 2000

#define HEARTBEAT_TIMEOUT 500

#endif // _DEFINES_H