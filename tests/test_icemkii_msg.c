#include "stm32f1xx.h"
#include "types.h"
//#include "gpio.h"
#include "debug.h"
#include "uniquedevid.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

//#include "usb_device.h"

//#include "fifo.h"
#include "icemkii_message.h"

// --- Incorrect CRC -------------------
// MESSAGE_START   - 27 - 0x1B
// SEQUENCE_NUMBER      - 0x0005
// MESSAGE_SIZE         - 0x00000004
// TOKEN           - 14 - 0x0E
// MESSAGE_BODY         - 0x87654321
// CRC                  - 0xXXXX
static U8 msgErrCrc[14] =
{
  0x1B, 0x05, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x0E, 0x21, 0x43, 0x65, 0x87, 0xFF, 0xFF,
};

//-----------------------------------------------------------------------------

void vLEDTask(void * pvParameters)
{
    //LOG("LED Task Started\r\n");
  
    while(TRUE)
    {
        //LOG("LED Hi\r\n");
        vTaskDelay(50);
        //LOG("LED Lo\r\n");
        vTaskDelay(50);
    }
    //vTaskDelete(NULL);
}

//-----------------------------------------------------------------------------

static U8 msgBuffer[100];
static ICEMKII_MESSAGE msg = { 0 };

void vTestTask(void * pvParameters)
{
    U32 i = 0;
    
    msg.MaxSize = sizeof(msgBuffer);
    msg.Buffer = msgBuffer;
    
    LOG("Test Task Started\r\n");
  
    //vTaskDelay(200);
    
    for(i = 0; i < sizeof(msgErrCrc); i++)
    {
        ICEMKII_MESSAGE_PutByte(&msg, msgErrCrc[i]);
    }
    
    while(TRUE)
    {
        //LOG("Template Task Iteration\r\n");
        vTaskDelay(500);
    }
    //vTaskDelete(NULL);
}

//-----------------------------------------------------------------------------

void Prepare_And_Run_Test( void )
{
    LOG("Preparing Template Test To Run...\r\n");

    xTaskCreate
    (
        vLEDTask,
        "LED",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL
    );
    
    xTaskCreate
    (
        vTestTask,
        "Template",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL
    );
}

//-----------------------------------------------------------------------------
