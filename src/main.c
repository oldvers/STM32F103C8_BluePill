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


void vLEDTask(void * pvParameters)
{
    /* LED */
    GPIO_Init(LED_PORT, LED_PIN, GPIO_TYPE_OUT_OD_2MHZ);

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

void vDualUartTask(void * pvParameters)
{
    LOG("Double UART Converter Task Started\r\n");

    /* Free PB3 from JTAG */
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    AFIO->MAPR &= ~(7 << 24);
    AFIO->MAPR |= (2 << 24);

    /* Init PB3 */
    GPIO_Init(SWD_SWO_PORT, SWD_SWO_PIN, GPIO_TYPE_OUT_PP_50MHZ);

    /* Enable WiFi Module */
    GPIO_Init(WIFI_EN_PORT, WIFI_EN_PIN, GPIO_TYPE_OUT_PP_2MHZ);
    GPIO_Hi(WIFI_EN_PORT, WIFI_EN_PIN);



    /* Test Pins */
    GPIO_Init(GPIOA,  7, GPIO_TYPE_OUT_PP_50MHZ);
    GPIO_Init(GPIOB, 11, GPIO_TYPE_OUT_PP_50MHZ);




    /* Init PB2 to OD Hi-Z - Switch-off 1k5 PullUp from USB D+ */
    GPIO_Init(USB_PUP_PORT, USB_PUP_PIN, GPIO_TYPE_OUT_OD_2MHZ);
    GPIO_Hi(USB_PUP_PORT, USB_PUP_PIN);

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
