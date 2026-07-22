#include "core.h"
#include "can_ring.h"

void can_ring_init(can_ring_t *ring, can_msg_t *buffer_memory, uint32_t capacity)
{
    Assert(ring != 0);
    Assert(buffer_memory != 0);
    Assert(capacity > 0);
    Assert((capacity & (capacity - 1U)) == 0U);
    
    ring->buffer = buffer_memory;
    ring->mask = capacity - 1U;
    ring->head = 0U;
    ring->tail = 0U;
}

int can_ring_put(can_ring_t *ring, can_msg_t msg)
{
    uint32_t current_head = ring->head;
    uint32_t next_head = (current_head + 1U) & ring->mask;
    
    if (next_head == ring->tail)
    {
        return 0;
    }
    
    ring->buffer[current_head] = msg;
    
    __DMB();
    
    ring->head = next_head;
    
    return 1;
}

int can_ring_get(can_ring_t *ring, can_msg_t *msg)
{
    uint32_t current_tail = ring->tail;
    
    if (current_tail == ring->head)
    {
        return 0;
    }
    
    *msg = ring->buffer[current_tail];
    
    __DMB();
    
    ring->tail = (current_tail + 1U) & ring->mask;
    return 1;
}