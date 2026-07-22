#include <stdint.h>
#include "../../lib/core.h"
#include "../../lib/types.h"
#include "../../lib/can_ring.h"
#include "stm32f103_overlay.h"

/* Backing memory for the CAN RX ring buffer */
#define CAN_RX_BUF_SIZE 32U
global can_msg_t can_rx_storage[CAN_RX_BUF_SIZE];
global can_ring_t can_rx_ring;

void Can_Init(void)
{
    can_ring_init(&can_rx_ring, can_rx_storage, CAN_RX_BUF_SIZE);
    /* Hardware register configuration goes here */
}

int Can_Read(can_msg_t *msg) 
{
    return can_ring_get(&can_rx_ring, msg);
}

void USB_LP_CAN_RX0_IRQHandler(void) 
{
    can_msg_t msg;
    
    /* Check if FIFO 0 is not empty (FMPIE0) */
    while ((CAN1->RF0R & CAN1_RF0R_FMP0) > 0U) {
        
        /* Extract IDE (Standard vs Extended ID) */
        uint32_t ide = CAN1->RI0R & (1U << 2);
        
        if (ide == 0U) {
            /* Standard 11-bit ID */
            msg.id.raw = (CAN1->RI0R >> 21) & 0x7FFU;
        } else {
            /* Extended 29-bit ID */
            
            msg.id.raw = (CAN1->RI0R >> 3) & 0x1FFFFFFFU;
        }
        
        /* Extract Data Length Code (DLC) */
        msg.dlc.raw = CAN1->RDT0R & 0x0FU;
        
        /* Extract 8-byte payload data registers */
        msg.data[0] = (uint8_t)(CAN1->RDL0R >> 0);
        msg.data[1] = (uint8_t)(CAN1->RDL0R >> 8);
        msg.data[2] = (uint8_t)(CAN1->RDL0R >> 16);
        msg.data[3] = (uint8_t)(CAN1->RDL0R >> 24);
        msg.data[4] = (uint8_t)(CAN1->RDH0R >> 0);
        msg.data[5] = (uint8_t)(CAN1->RDH0R >> 8);
        msg.data[6] = (uint8_t)(CAN1->RDH0R >> 16);
        msg.data[7] = (uint8_t)(CAN1->RDH0R >> 24);
        
        /* Release FIFO 0 output mailbox */
        CAN1->RF0R |= CAN1_RF0R_RFOM0;
        
        /* Push into the SPSC lock-free ring buffer */
        int success = can_ring_put(&can_rx_ring, msg);
        
        /* If buffer is saturated, trigger assertion halt in debug mode */
        Assert(success);
    }
}