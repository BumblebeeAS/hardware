/*
 * msg_queue.h
 *
 *  Created on: Apr 5, 2021
 *      Author: raeee
 */

#ifndef INC_MSG_QUEUE_H_
#define INC_MSG_QUEUE_H_

#include "stdint.h"

#define BUFSIZE (512)

typedef struct {
	volatile uint8_t data[BUFSIZE];
	volatile uint8_t head;
	volatile uint8_t tail;
	volatile uint8_t size;
} msg_queue;


void MsgQueue_Init(msg_queue* buf);
uint8_t MsgQueue_Empty(msg_queue* buf);
uint8_t MsgQueue_Full(msg_queue* buf);
uint8_t MsgQueue_push(msg_queue* buf, uint8_t d);
uint8_t MsgQueue_pop(msg_queue * buf);




#endif /* INC_MSG_QUEUE_H_ */
