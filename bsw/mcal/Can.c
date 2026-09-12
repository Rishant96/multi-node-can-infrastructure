#include <stdint.h>
#include "../../lib/core.h"
#include "../../lib/types.h"
#include "../../lib/can_ring.h"
#include "stm32f103_overlay.h"

/* Backing memory for the CAN RX ring buffer */
#define CAN_RX_BUF_SIZE 32U
global can_msg_t can_rx_storage[CAN_RX_BUF_SIZE];
global can_ring_t g_can_rx_ring;

/* 
 * USB LP CAN RX0 Interrupt Vector Number on Cortex-M3 NVIC 
 * IRQ 20 = Position 20 in the NVIC ISER register
 */
#define NVIC_ISER0_CAN1_RX0_BIT (1U << 20)

void Can_Init(void)
{
    can_ring_init(&g_can_rx_ring, can_rx_storage, CAN_RX_BUF_SIZE);
    
    /* Hardware register configuration goes here */
    RCC->APB2ENR |= (1U << 0);  /* Enable AFIO clock */
    RCC->APB2ENR |= (1U << 2);  /* Enable GPIOA clock */
    RCC->APB1ENR |= (1U << 25); /* Enable CAN1 clock */
    
    GPIOA->CRH &= ~(0xFFU << 12); /* Clear PA11 and PA12 configuration bits */
    GPIOA->CRH |=  (0x4U << 12);  /* Set PA11 to Input Floating */
    GPIOA->CRH |=  (0xBU << 16);  /* Set PA12 to AF Output Push-Pull 50MHz */
    
    CAN1->MCR |= (1U << 0); /* Set INRQ (Initialization Request) */
    
    uint32_t timeout = 0U;
    /* Wait for hardware confirmation (INAK bit in MSR) */
    while (((CAN1->MSR & (1U << 0)) == 0U) && (timeout < 0x0000FFFFU)) {
        timeout++;
    }
    
    CAN1->MCR &= ~(1U << 1); /* Clear SLEEP bit */
    
    CAN1->BTR = 0U;
    CAN1->BTR |= ( 3U << 0);  /* BRP  = 3  (Prescaler = 4) */
    CAN1->BTR |= (11U << 16); /* TS1  = 11 (12 Time Quanta) */
    CAN1->BTR |= ( 4U << 20); /* TS2  = 4  (5 Time Quanta) */
    CAN1->BTR |= ( 0U << 24); /* SJW  = 0  (1 Time Quanta) */
    //CAN1->BTR |= ( 1U << 31); /* SILM = 1  (Silent mode enable) */
    //CAN1->BTR |= ( 1U << 30); /* LBKM = 1  (Loopback mode enable) */
    
    
    CAN1->FMR |= (1U << 0);   /* Set FINIT bit to enter Filter Init Mode */
    CAN1->FA1R &= ~(1U << 0); /* Deactivate Filter 0 */
    CAN1->FS1R &= ~(1U << 0);  /* Set Filter 0 to single 16-bit scale */
    CAN1->FM1R |= (1U << 0); /* Set Filter 0 to ID Mask mode */
    
    uint32_t target_id = (0x123U << (16-11));
    
    CAN1->sFilterRegister[0].FR1 = (target_id << 16) | target_id; 
    CAN1->sFilterRegister[0].FR2 = (target_id << 16) | target_id; 
    
    CAN1->FA1R |= (1U << 0);  /* Activate Filter 0 */
    CAN1->FMR &= ~(1U << 0);  /* Exit Filter Init Mode */
    
    CAN1->IER |= (1U << 1);   /* Enable FMPIE0 (FIFO 0 Message Pending Interrupt) */
    CAN1->MCR &= ~(1U << 0);  /* Clear INRQ to enter Normal Mode */
    
    /* Wait for hardware to acknowledge normal operating mode */
    timeout = 0U;
    while (((CAN1->MSR & (1U << 0)) != 0U) && (timeout < 0x0000FFFFU)) {
        timeout++;
    }
    
    NVIC->ISER[0] |= NVIC_ISER0_CAN1_RX0_BIT;
}

int Can_Read(can_msg_t *msg) 
{
    return can_ring_get(&g_can_rx_ring, msg);
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
        int success = can_ring_put(&g_can_rx_ring, msg);
        
        /* If buffer is saturated, trigger assertion halt in debug mode */
        Assert(success);
    }
}