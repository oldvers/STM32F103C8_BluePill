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

    while(FW_TRUE) {};
}

//-----------------------------------------------------------------------------
