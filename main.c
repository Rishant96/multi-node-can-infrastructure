/* date = July 21st 2026 */

#include <stdint.h>
#include "lib/core.h"
#include "lib/types.h"
#include "lib/can_ring.h"

/* External MCAL driver hooks */
extern void Can_Init(void);
extern int Can_Read(can_msg_t *msg);

int main(void) 
{
    Can_Init();
    
    /* System initialization hooks will go here */
    
    for (;;) {
        can_msg_t incoming_msg;
        
        if (Can_Read(&incoming_msg)) {
            /* Process application domain logic here */
        }
    }
    
    return 0;
}
