#include <stdio.h>

#include "stm32f1xx.h"
#include "types.h"
#include "board.h"
#include "gpio.h"
#include "debug.h"
#include "uniquedevid.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "usb_device.h"
#include "fifo.h"

#include "usb_defs.h"
#include "usb_cfg.h"

void vLEDTask(void * pvParameters)
{
    /* LED */
    GPIO_Init(LED_PORT, LED_PIN, GPIO_TYPE_OUT_OD_2MHZ, 1);

    while(FW_TRUE)
    {
        /* LED */
        GPIO_Lo(LED_PORT, LED_PIN);
        vTaskDelay(500);
        GPIO_Hi(LED_PORT, LED_PIN);
        vTaskDelay(500);
        //LOG("LED\r\n");
    }
    //vTaskDelete(NULL);
}

extern const U8 USB_ConfigDescriptor[];

void vDualUartTask(void * pvParameters)
{
    USB_CONFIGURATION_DESCRIPTOR * cd;

    LOG("Double UART Converter Task Started\r\n");
    
    cd = (USB_CONFIGURATION_DESCRIPTOR *)USB_ConfigDescriptor;
    
    LOG("Config Desc Total Length = 0x%04X = %d\r\n",
        cd->wTotalLength, cd->wTotalLength);
    
    LOG("--- CDC 1 ------------------------------------\r\n");
    LOG("Interface Count = %d\r\n",     USB_CDC_IF_CNT);
    LOG("    IF Num 0    = %d\r\n",     USB_CDC_IF_NUM);
    LOG("Endpoint Count  = %d\r\n",     USB_CDC_EP_CNT);
    LOG("    EP 0 Blk O  = 0x%02X\r\n", USB_CDC_EP_BLK_O);
    LOG("    EP 1 Blk I  = 0x%02X\r\n", USB_CDC_EP_BLK_I);
    
    LOG("--- CDC 2 ------------------------------------\r\n");
    LOG("Interface Count = %d\r\n",     USB_CDD_IF_CNT);
    LOG("    IF Num 0    = %d\r\n",     USB_CDD_IF_NUM);
    LOG("Endpoint Count  = %d\r\n",     USB_CDD_EP_CNT);
    LOG("    EP 0 Blk O  = 0x%02X\r\n", USB_CDD_EP_BLK_O);
    LOG("    EP 1 Blk I  = 0x%02X\r\n", USB_CDD_EP_BLK_I);
    
    LOG("--- Total ------------------------------------\r\n");
    LOG("Interface Count = %d\r\n",     USB_IF_CNT);
    LOG("Endpoint Count  = %d\r\n",     USB_EP_CNT);
    LOG("    EP 0 Ctl I  = 0x%02X\r\n", EP0_I);
    LOG("    EP 1 Ctl O  = 0x%02X\r\n", EP0_O);

//    /* Free PB3 from JTAG */
//    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
//    AFIO->MAPR &= ~(7 << 24);
//    AFIO->MAPR |= (2 << 24);
//
//    /* Init PB3 */
//    GPIO_Init(SWD_SWO_PORT, SWD_SWO_PIN, GPIO_TYPE_OUT_PP_50MHZ, 0);
//
//    /* Enable WiFi Module */
//    GPIO_Init(WIFI_EN_PORT, WIFI_EN_PIN, GPIO_TYPE_OUT_PP_2MHZ, 1);
//
//
//
//    /* Test Pins */
//    GPIO_Init(GPIOA,  7, GPIO_TYPE_OUT_PP_50MHZ, 0);
//    GPIO_Init(GPIOB, 11, GPIO_TYPE_OUT_PP_50MHZ, 0);




    /* Init PB2 to OD Hi-Z - Switch-off 1k5 PullUp from USB D+ */
    GPIO_Init(USB_PUP_PORT, USB_PUP_PIN, GPIO_TYPE_OUT_OD_2MHZ, 1);

    /* Delay */
    vTaskDelay(200);

    /* Init USB. Switch-on 1k5 PullUp to USB D+ - connect USB device */
    USBD_Init();
    GPIO_Lo(USB_PUP_PORT, USB_PUP_PIN);

    vTaskDelay(5000);

    while(FW_TRUE)
    {
        vTaskDelay(5000);
    }
    //vTaskDelete(NULL);
}


int main(void)
{
    LOG("STM32F103C8 Started!\r\n");
    LOG("ID0 = 0x%04X\r\n", UDID_0);
    LOG("ID1 = 0x%04X\r\n", UDID_1);
    LOG("ID2 = 0x%08X\r\n", UDID_2);
    LOG("ID2 = 0x%08X\r\n", UDID_3);
    LOG("Memory Size = %d kB\r\n", FLASH_SIZE);
    LOG("SysClock = %d Hz\r\n", SystemCoreClock);

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
        vDualUartTask,
        "Dual UART",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL
    );

    vTaskStartScheduler();

    while(FW_TRUE) {};
}
