/* date = July 21st 2026 4:08 pm */

#ifndef _CAN_H
#define _CAN_H

#include "../../lib/types.h"

void Can_Init(void);
int Can_Read(can_msg_t *msg);

#endif //_CAN_H
