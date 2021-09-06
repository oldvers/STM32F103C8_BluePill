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

////-----------------------------------------------------------------------------
///** @brief Puts received Byte from USB EP buffer to the Rx FIFO
// *  @param pByte - Pointer to the container for Byte
// *  @return None
// */
//
//static void cdc_RxFifoPutB(U8 * pByte)
//{
//  //(void)FIFO_Put(&gPortB.rxFifo, pByte);
//}
//
////-----------------------------------------------------------------------------
///** @brief Gets Byte that need to be transmitted from the Tx FIFO
// *  @param pByte - Pointer to the container for Byte
// *  @return None
// */
//
//static void cdc_TxFifoGetB(U8 * pByte)
//{
//  //(void)FIFO_Get(&gPortB.txFifo, pByte);
//}
//
////-----------------------------------------------------------------------------
///** @brief Puts received Byte from UART to the Tx FIFO
// *  @param pByte - Pointer to the container for Byte
// *  @return TRUE if byte has been put successfully
// */
//
//static FW_BOOLEAN uart_FifoPutB(U8 * pByte)
//{
//  return FW_FALSE; //(FW_BOOLEAN)(FW_SUCCESS == FIFO_Put(&gPortB.txFifo, pByte));
//}
//
////-----------------------------------------------------------------------------
///** @brief Gets Byte that need to be transmitted from the Rx FIFO
// *  @param pByte - Pointer to the container for Byte
// *  @return None
// */
//
//static FW_BOOLEAN uart_FifoGetB(U8 * pByte)
//{
//  return FW_FALSE; //(FW_BOOLEAN)(FW_SUCCESS == FIFO_Get(&gPortB.rxFifo, pByte));
//}
//

static void i2c_Open(void)
{
  //
}

static void i2c_SetControlLine(U16 aValue)
{
  //
}

//-----------------------------------------------------------------------------
/** @brief Initializes CDC I2C
 *  @param None
 *  @return None
 */

void CDC_I2C_Init(void)
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
  gNotification.wIndex                     = USBD_CDC_I2C_GetInterfaceNumber();
  gNotification.wLength                    = 2;
  gNotification.Data.Raw                   = 0;

  /* Clear Port context */
  memset(&gPort, 0, sizeof(gPort));
  /* Port is not ready yet */
  gPort.ready = FW_FALSE;

  /* Initialize Endpoints */
  //gPortI2C.epOBlkRd        = USBD_CDC_I2C_OEndPointRdWsCb;
  //gPortI2C.epOBlkIsRxEmpty = USBD_CDC_I2C_OEndPointIsRxEmpty;
  //gPortI2C.epIBlkWr        = USBD_CDC_I2C_IEndPointWrWsCb;
  //gPortI2C.epIBlkIsTxEmpty = USBD_CDC_I2C_IEndPointIsTxEmpty;
  gPort.fpEpIIrqWr        = USBD_CDC_I2C_IrqEndPointWr;
  gPort.fpOpen            = i2c_Open;
  gPort.fpSetCtrlLine     = i2c_SetControlLine;
  /* Initialize FIFOs */
  //FIFO_Init(&gPortI2C.rxFifo, gPortI2C.rxBuffer, sizeof(gPortI2C.rxBuffer));
  //FIFO_Init(&gPortI2C.txFifo, gPortI2C.txBuffer, sizeof(gPortI2C.txBuffer));
  //gPortI2C.rxFifoPutCb = cdc_RxFifoPutB;
  //gPortI2C.txFifoGetCb = cdc_TxFifoGetB;

  /* Initialize pointers */
  gPort.lineCoding = &gLineCoding;
  gPort.notification = &gNotification;
}

//-----------------------------------------------------------------------------
/** @brief Returns pointer to CDC I2C instance
 *  @param None
 *  @return CDC I2C instance
 */

CDC_PORT * CDC_I2C_GetPort(void)
{
  return &gPort;
}
