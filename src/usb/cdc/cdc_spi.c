#include <string.h>
#include "types.h"
#include "usb_definitions.h"
#include "usb_cdc_definitions.h"
#include "usb_descriptor.h"
#include "cdc_private.h"

//-----------------------------------------------------------------------------
/* Global Variables */

STATIC CDC_LINE_CODING   gLineCoding   = { 0 };
STATIC CDC_SERIAL_STATE  gNotification = { 0 };
STATIC CDC_PORT          gPort         = { 0 };

//-----------------------------------------------------------------------------

static void spi_Open(void)
{
  //
}

static void spi_SetControlLine(U16 aValue)
{
  //
}

//-----------------------------------------------------------------------------
/** @brief Initializes CDC SPI
 *  @param None
 *  @return None
 */

void CDC_SPI_Init(void)
{
  /* Init Line Coding */
  gLineCoding.dwBaudRate  = 115200;
  gLineCoding.bCharFormat = 0;
  gLineCoding.bParityType = 0;
  gLineCoding.bDataBits   = 8;

  /* Init Notification */
  gNotification.bmRequestType.BM.Recipient = REQUEST_TO_INTERFACE;
  gNotification.bmRequestType.BM.Type      = REQUEST_CLASS;
  gNotification.bmRequestType.BM.Dir       = REQUEST_DEVICE_TO_HOST;
  gNotification.bNotification              = CDC_NTF_SERIAL_STATE;
  gNotification.wValue                     = 0;
  gNotification.wIndex                     = USBD_CDC_SPI_GetInterfaceNumber();
  gNotification.wLength                    = 2;
  gNotification.Data.Raw                   = 0;

  /* Clear Port context */
  memset(&gPort, 0, sizeof(gPort));

  /* Port is not ready yet */
  gPort.ready = FW_FALSE;

//  gPortSPI.rxComplete = FW_FALSE;

  /* Initialize Endpoints */
//  gPortSPI.epOBlkRd        = USBD_CDC_SPI_OEndPointRdWsCb;
//  gPortSPI.epOBlkIsRxEmpty = USBD_CDC_SPI_OEndPointIsRxEmpty;
//  gPortSPI.epIBlkWr        = USBD_CDC_SPI_IEndPointWrWsCb;
//  gPortSPI.epIBlkIsTxEmpty = USBD_CDC_SPI_IEndPointIsTxEmpty;
  gPort.fpEpIIrqWr        = USBD_CDC_SPI_IrqEndPointWr;
  gPort.fpOpen            = spi_Open;
  gPort.fpSetCtrlLine     = spi_SetControlLine;

//  /* Initialize FIFOs */
//  FIFO_Init(&gPortSPI.rxFifo, gPortSPI.rxBuffer, sizeof(gPortSPI.rxBuffer));
//  FIFO_Init(&gPortSPI.txFifo, gPortSPI.txBuffer, sizeof(gPortSPI.txBuffer));
//  gPortSPI.rxFifoPutCb = cdc_RxFifoPutB;
//  gPortSPI.txFifoGetCb = cdc_TxFifoGetB;

  /* Initialize pointers */
  gPort.lineCoding = &gLineCoding;
  gPort.notification = &gNotification;

//  /* Initialize UART Number */
//  gPortSPI.uart = UART2;
}

//-----------------------------------------------------------------------------
/** @brief Returns pointer to CDC SPI instance
 *  @param None
 *  @return CDC SPI instance
 */

CDC_PORT * CDC_SPI_GetPort(void)
{
  return &gPort;
}
