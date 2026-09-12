/* date = July 21st 2026 */

#include <stdint.h>
#include "lib/core.h"
#include "lib/types.h"
#include "lib/can_ring.h"
#include "bsw/mcal/stm32f103_overlay.h"

/* External MCAL driver hooks */
extern void SystemInit(void);
extern void Can_Init(void);
extern int Can_Read(can_msg_t *msg);

int main(void) 
{
    SystemInit();
    Can_Init();
    
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    GPIOC->CRH &= ~(0xFU << 20);
    GPIOC->CRH |=  (0x3U << 20);
    GPIOC->BSRR = (1U << 13);
    
    for (;;) {
        //can_msg_t incoming_msg;
        
        
        if ((CAN1->TSR & CAN1_TSR_TME0) != 0U) {
            CAN1->TI0R = (0x123U << 21);
            CAN1->TDT0R = 1U;
            CAN1->TDL0R = 0xAAU;
            CAN1->TI0R |= CAN1_TIXR_TXRQ;
        }
        
        for (volatile uint32_t i = 0U; i < 1000000U; i++) { }
        
        /*
        if (Can_Read(&incoming_msg)) {
            if (incoming_msg.id.raw == 0x123U && incoming_msg.data[0] == 0xAAU) {
                
                GPIOC->ODR ^= (1U << 13);
                for(volatile uint32_t i = 0U; i < 500000U; i++) { }
                
            }
        }*/
    }
    
    return 0;
}
