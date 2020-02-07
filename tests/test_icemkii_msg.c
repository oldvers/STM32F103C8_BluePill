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

static U8 msgs[14 * 2] =
{
  /* Message with incorrect CRC */
  0x1B, 0x05, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x0E, 0x21, 0x43, 0x65, 0x87, 0xFF, 0xFF,
  /* Correct message */
  0x1B, 0x05, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x0E, 0x21, 0x43, 0x65, 0x87, 0xC2, 0xC3,
};

//--- Mocks -------------------------------------------------------------------

void USB_IRQHandler(void)
{
  //
}

//-----------------------------------------------------------------------------

void vLEDTask(void * pvParameters)
{
    //LOG("LED Task Started\r\n");

    while(FW_TRUE)
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
    FW_RESULT result = FW_SUCCESS;

    ICEMKII_MESSAGE_Init(&msg, msgBuffer, sizeof(msgBuffer));

    LOG("Test Task Started\r\n");

    //vTaskDelay(200);

    for(i = 0; i < sizeof(msgs); i++)
    {
        LOG(" - Put Byte - %0.2X", msgs[i]);
        result = ICEMKII_MESSAGE_PutByte(&msg, msgs[i]);
        switch ( result )
        {
            case FW_SUCCESS:
                LOG("\r\n");
                break;
            case FW_COMPLETE:
                LOG(" <- COMPLETE\r\n");
                break;
            case FW_ERROR:
                LOG(" <- ERROR: ID = %d\r\n", msg.LastError);
                break;
            default:
                LOG(" <- ERROR: Unexpected result!\r\n");
                break;
        }
    }

    while(FW_TRUE)
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
