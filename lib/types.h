/* date = July 21st 2026 2:52 pm */

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

typedef struct { uint32_t raw; } can_id_t;
typedef struct { uint8_t  raw; } can_dlc_t;

typedef struct {
	can_id_t  id;
	can_dlc_t dlc;
	uint8_t   data[8];
} can_msg_t;

typedef struct
{
    can_msg_t msg;
    int valid;
} cmd_result_t;

#endif //TYPES_H
