#include <stdio.h>

#include "stm32f1xx.h"
#include "debug.h"
#include "uniquedevid.h"
#include "FreeRTOS.h"
#include "task.h"

extern void Prepare_And_Run_Test( void );

//-----------------------------------------------------------------------------

int main(void)
{
    LOG("STM32F103C8 Started!\r\n");
    LOG("ID0 = 0x%04X\r\n", UDID_0);
    LOG("ID1 = 0x%04X\r\n", UDID_1);
    LOG("ID2 = 0x%08X\r\n", UDID_2);
    LOG("ID2 = 0x%08X\r\n", UDID_3);
    LOG("Memory Size = %d kB\r\n", FLASH_SIZE);
    LOG("SysClock = %d Hz\r\n", SystemCoreClock);
  
    Prepare_And_Run_Test();
    
    vTaskStartScheduler();
    
    while(TRUE) {};
}

//-----------------------------------------------------------------------------

void Fault(U32 stack[])
{
    enum {r0, r1, r2, r3, r12, lr, pc, psr};
    
    LOG("Hard Fault\r\n");
    LOG("  SHCSR    = 0x%08x\r\n", SCB->SHCSR);
    LOG("  CFSR     = 0x%08x\r\n", SCB->CFSR);
    LOG("  HFSR     = 0x%08x\r\n", SCB->HFSR);
    LOG("  MMFAR    = 0x%08x\r\n", SCB->MMFAR);
    LOG("  BFAR     = 0x%08x\r\n", SCB->BFAR);  
    
    LOG("  R0       = 0x%08x\r\n", stack[r0]);
    LOG("  R1       = 0x%08x\r\n", stack[r1]);
    LOG("  R2       = 0x%08x\r\n", stack[r2]);
    LOG("  R3       = 0x%08x\r\n", stack[r3]);
    LOG("  R12      = 0x%08x\r\n", stack[r12]);
    LOG("  LR [R14] = 0x%08x - Subroutine call return address\r\n", stack[lr]);
    LOG("  PC [R15] = 0x%08x - Program counter\r\n", stack[pc]);
    LOG("  PSR      = 0x%08x\r\n", stack[psr]);
    
    while(TRUE) {};
}

//-----------------------------------------------------------------------------
