/* date = July 21st 2026 2:53 pm */

#ifndef RING_BUF_H
#define RING_BUF_H

#include <stdint.h>

typedef struct {
	uint8_t *base;
	uint16_t capacity;
	uint16_t mask;
	volatile uint16_t head;
	volatile uint16_t tail;
} ring_buf_t;

#endif //RING_BUF_H
