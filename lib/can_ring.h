/* date = July 21st 2026 3:01 pm */

#ifndef CAN_RING_H
#define CAN_RING_H

#include <stdint.h>
#include "types.h"

/* 
  * Capacity must be a power of 2.
 */
typedef struct {
    can_msg_t *buffer;
    uint32_t mask;          /* Capacity - 1 for bitwise wrapping */
    volatile uint32_t head;
    volatile uint32_t tail;
} can_ring_t;

void can_ring_init(can_ring_t *ring, can_msg_t *buffer_memory, uint32_t capacity);
int can_ring_put(can_ring_t *ring, can_msg_t msg);
int can_ring_get(can_ring_t *ring, can_msg_t *msg);

#endif //CAN_RING_H
