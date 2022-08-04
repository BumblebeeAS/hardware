#include "msg_queue.h"


void MsgQueue_Init(msg_queue* buf) {
  unsigned int i;
  for (i=0; i<BUFSIZE; i++)
    buf->data[i] = 0;  // to simplify our lives when debugging
  buf->head = 0;
  buf->tail = 0;
  buf->size = 0;
}

uint8_t MsgQueue_Empty(msg_queue* buf){ return (uint8_t)( (buf->tail - buf->head) &0xff ) == 0u; }
uint8_t MsgQueue_Full(msg_queue* buf){ return (uint8_t)( (buf->tail - buf->head) &0xff ) == BUFSIZE; }

uint8_t MsgQueue_push(msg_queue* buf, uint8_t d) {
  if (!MsgQueue_Full(buf)) {
    buf->data[buf->tail++] = d;
    buf->tail %= BUFSIZE;
    buf->size++;
    return 1; // success
  } else
    return 0; // failure
}

uint8_t MsgQueue_pop(msg_queue * buf) {
  // Must check to see if queue is empty before dequeueing
  uint8_t t=0;
  if (!MsgQueue_Empty(buf)) {
    t = buf->data[buf->head];
    buf->data[buf->head++] = 0; // to simplify debugging
    buf->head %= BUFSIZE;
    buf->size--;
  }
  return t;
}
