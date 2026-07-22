#include <stdint.h>
#include "Mcu.h"
#include "../../lib/core.h"
#include "stm32f103_overlay.h"

#define HSI_STARTUP_TIMEOUT 0x0500U

void SystemInit(void)
{
    /* 
     * 1. Reset the RCC clock configuration to the default reset state 
     * (Enable HSI, clear all prescalers, disable PLL)
     */
    RCC->CR |= 0x00000001U;           /* Turn on HSI (8MHz internal) */
    RCC->CFGR &= 0xF8FF0000U;         /* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE, MCO */
    RCC->CR &= 0xFEF6FFFFU;           /* Reset HSEON, CSSON, PLLON */
    RCC->CR &= 0xFFFBFFFFU;           /* Reset HSEBYP */
    RCC->CFGR &= 0xFF80FFFFU;         /* Reset PLLSRC, PLLXTPRE, PLLMUL, USBPRE */
    RCC->CIR = 0x009F0000U;           /* Disable all clock interrupts and clear pending flags */
    
    /* 
     * 2. Turn on the HSE (High-Speed External) Crystal
     * Assuming a standard 8MHz crystal on the BluePill/STM32F103 board
     */
    RCC->CR |= (1U << 16);            /* Set HSEON */
    
    /* Wait for HSE to become ready (HSERDY) */
    uint32_t startup_counter = 0;
    while (((RCC->CR & (1U << 17)) == 0) && (startup_counter < HSI_STARTUP_TIMEOUT)) {
        startup_counter++;
    }
    
    if ((RCC->CR & (1U << 17)) != 0) {
        /* HSE is stable and ready */
        
        /* 3. Configure Flash Prefetch and Latency for 72MHz (2 wait states) */
        FLASH->ACR |= 0x02U;          /* Latency = 2 */
        FLASH->ACR |= (1U << 4);      /* Enable prefetch buffer */
        
        /* 
         * 4. Configure Bus Prescalers 
         * AHB  = SYSCLK / 1 = 72 MHz
         * APB2 = SYSCLK / 1 = 72 MHz
         * APB1 = SYSCLK / 2 = 36 MHz (CAN peripheral sits on APB1. Max is 36MHz!)
         */
        RCC->CFGR |= (0U << 4);       /* HPRE (AHB)  = DIV1 */
        RCC->CFGR |= (4U << 8);       /* PPRE1 (APB1) = DIV2 */
        RCC->CFGR |= (0U << 11);      /* PPRE2 (APB2) = DIV1 */
        
        /* 
         * 5. Configure the PLL 
         * Source = HSE (8MHz), Multiplier = 9 -> 72MHz 
         */
        RCC->CFGR |= (1U << 16);      /* PLLSRC = HSE */
        RCC->CFGR |= (7U << 18);      /* PLLMUL = 9 */
        
        /* 6. Enable the PLL and wait for it to lock */
        RCC->CR |= (1U << 24);        /* PLLON */
        while ((RCC->CR & (1U << 25)) == 0) {
            /* Wait for PLLRDY */
        }
        
        /* 7. Switch the System Clock Source (SYSCLK) to the PLL */
        RCC->CFGR |= (2U << 0);       /* SW = PLL */
        while ((RCC->CFGR & (3U << 2)) != (2U << 2)) {
            /* Wait for SWS (System Clock Switch Status) to indicate PLL */
        }
    } else {
        /* 
         * HSE failed to start. 
         * In a safety-critical system, you would trigger a HardFault or 
         * fall back to the HSI, assert an error LED, and log the failure. 
         */
        Assert(0); 
    }
}
