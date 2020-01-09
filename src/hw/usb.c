#include "stm32f1xx.h"
#include "types.h"
#include "interrupts.h"
#include "usb.h"
#include "debug.h"

/* Address Mask */
#define USB_EP_ADDR_MASK    (0xFFFE)
/* Count Mask */
#define USB_EP_COUNT_MASK   (0x03FF)
/* Endpoint Register */
#define EPREG(num)          (*((volatile U32 *)(USB_BASE + 4 * (num))))

/* Endpoint Buffer Descriptor */
typedef __packed struct EpBuffDescription_s
{
  U32 ADDR_TX;
  U32 COUNT_TX;
  U32 ADDR_RX;
  U32 COUNT_RX;
} EpBuffDescription_t, * EpBuffDescription_p;

/* Endpoint Configuration */
typedef __packed struct EpConfiguration_s
{
  U16      IMaxSize; /* In Enpoint max transfer size  */
  U16      IRsvd;
  USB_CbEp ICb;      /* In Enpoint Callback function  */
  U16      OMaxSize; /* Out Enpoint max transfer size */
  U16      ORsvd;
  USB_CbEp OCb;      /* Out Enpoint Callback function */
} EpConfiguration_t, * EpConfiguration_p;

/* Pointer to Endpoint Buffer Descriptors */
static EpBuffDescription_p pEpBuffDscr = (EpBuffDescription_p)USB_PMAADDR;
/* Endpoint Free Buffer Address */
static U16 gEpFreeBuffAddr                           = 0;
/* Endpoint Count */
static U8  gMaxEpCount                               = 1;
/* Control Endpoint max packet size */
static U8  gCtrlEpMaxPacketSize                      = 8;
/* Callback Functions */
static USB_CbGeneric     pUSB_CbReset                = NULL;
static USB_CbGeneric     pUSB_CbSuspend              = NULL;
static USB_CbGeneric     pUSB_CbWakeUp               = NULL;
static USB_CbGeneric     pUSB_CbSOF                  = NULL;
static USB_CbError       pUSB_CbError                = NULL;
static EpConfiguration_t USB_EpCfg[USB_EP_QUANTITY]  = {0};

//-----------------------------------------------------------------------------
/** @brief Resets endpoint
 *  @param aNumber - Endpoint Number
 *  @return None
 *  @note aNumber - bits 0..2 = Address, bit 7 = Direction
 */
static void usb_EpReset(U32 aNumber)
{
  U32 num, val;

  num = aNumber & USB_EP_NUM_MASK;
  val = EPREG(num);
  if (aNumber & USB_EP_DIR_MASK)
  {
    /* IN Endpoint */
    EPREG(num) = val & (USB_EPREG_MASK | USB_EP_DTOG_TX);
  }
  else
  {
    /* OUT Endpoint */
    EPREG(num) = val & (USB_EPREG_MASK | USB_EP_DTOG_RX);
  }
}

//-----------------------------------------------------------------------------
/** @brief Sets endpoint status
 *  @param aNumber - endpoint number
 *  @param aStatus - new status
 *  @return None
 *  @note aNumber - bits 0..2 = Address, bit 7 = Direction
 */
static void usb_EpSetStatus(U32 aNumber, U32 aStatus)
{
  U32 num, val;

  num = aNumber & USB_EP_NUM_MASK;
  val = EPREG(num);
  if (aNumber & USB_EP_DIR_MASK)
  {
    /* IN Endpoint */
    EPREG(num) =
      (val ^ (aStatus & USB_EPTX_STAT)) & (USB_EPREG_MASK | USB_EPTX_STAT);
  }
  else
  {
    /* OUT Endpoint */
    EPREG(num) =
      (val ^ (aStatus & USB_EPRX_STAT)) & (USB_EPREG_MASK | USB_EPRX_STAT);
  }
}

//-----------------------------------------------------------------------------
/** @brief Sets endpoint status
 *  @param aNumber - endpoint number
 *  @param aStatus - new status
 *  @return None
 *  @note aNumber - bits 0..2 = Address, bit 7 = Direction
 */
void USB_SetCb_Reset(USB_CbGeneric pCbReset)
{
  pUSB_CbReset = pCbReset;
}

//-----------------------------------------------------------------------------
/** @brief Registers Suspend callback function
 *  @param pCbSuspend - Pointer to function
 *  @return None
 */
void USB_SetCb_Suspend(USB_CbGeneric pCbSuspend)
{
  pUSB_CbSuspend = pCbSuspend;
}

//-----------------------------------------------------------------------------
/** @brief Registers WakeUp callback function
 *  @param pCbWakeUp - Pointer to function
 *  @return None
 */
void USB_SetCb_WakeUp(USB_CbGeneric pCbWakeUp)
{
  pUSB_CbWakeUp = pCbWakeUp;
}

//-----------------------------------------------------------------------------
/** @brief Registers SOF callback function
 *  @param pCbSOF - Pointer to function
 *  @return None
 */
void USB_SetCb_SOF(USB_CbGeneric pCbSOF)
{
  pUSB_CbSOF = pCbSOF;
}

//-----------------------------------------------------------------------------
/** @brief Registers Error callback function
 *  @param pCbError - Pointer to function
 *  @return None
 */
void USB_SetCb_Error(USB_CbError pCbError)
{
  pUSB_CbError = pCbError;
}

//-----------------------------------------------------------------------------
/** @brief Registers Endpoint callback function
 *  @param aNumber - Number of endpoint
 *  @param pCbEp - Pointer to function
 *  @return None
 *  @note aNumber - bits 0..2 = Address, bit 7 = Direction
 */
void USB_SetCb_Ep(U32 aNumber, USB_CbEp pCbEp)
{
  U32 num = (aNumber & USB_EP_NUM_MASK);

  if (0 != (aNumber & USB_EP_DIR_MASK))
  {
    /* IN */
    USB_EpCfg[num].ICb = pCbEp;
  }
  else
  {
    /* OUT */
    USB_EpCfg[num].OCb = pCbEp;
  }
}

//-----------------------------------------------------------------------------
/** @brief Initializes USB peripheral
 *  @param aMaxEpCount - Maximum count of endpoints
 *  @param aCtrlEpMaxPacketSize - Maximum packet size for control endpoint
 *  @return None
 *  @note Called by the User to initialize USB
 */
void USB_Init(U32 aMaxEpCount, U32 aCtrlEpMaxPacketSize)
{
  for (U32 num = 0; num < USB_EP_QUANTITY; num++)
  {
    USB_EpCfg[num].ICb = NULL;
    USB_EpCfg[num].OCb = NULL;
    USB_EpCfg[num].IMaxSize = 0;
    USB_EpCfg[num].OMaxSize = 0;
  }

  gMaxEpCount = aMaxEpCount;
  gCtrlEpMaxPacketSize = aCtrlEpMaxPacketSize;
  gEpFreeBuffAddr = gMaxEpCount * sizeof(EpBuffDescription_t);

  /* Setup USB clock prescaler */
  if (72000000 > SystemCoreClock)
  {
    RCC->CFGR |= RCC_CFGR_USBPRE;
  }

  /* Enable USB peripheral clock */
  RCC->APB1ENR |= RCC_APB1ENR_USBEN;
  
  /* Reset USB peripheral */
  RCC->APB1RSTR |= RCC_APB1RSTR_USBRST;
  RCC->APB1RSTR &= (~RCC_APB1RSTR_USBRST);

  /* Enable USB Interrupts */
  NVIC_ClearPendingIRQ(USB_LP_CAN1_RX0_IRQn);
  NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, IRQ_PRIORITY_USB);
  NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
}

//-----------------------------------------------------------------------------
/** @brief De-Initializes USB peripheral
 *  @param None
 *  @return None
 *  @note Called by the User to de-initialize USB
 */
void USB_DeInit(void)
{
  /* Disable USB Interrupts */
  NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
  NVIC_ClearPendingIRQ(USB_LP_CAN1_RX0_IRQn);

  /* Disable USB peripheral clock */
  RCC->APB1ENR &= (~RCC_APB1ENR_USBEN);
}

//-----------------------------------------------------------------------------
/** @brief Connects USB peripheral
 *  @param aConnnect - Connect/Disconnect
 *  @return None
 *  @note Called by the User to Connect/Disconnect USB
 */
void USB_Connect(U32 aConnect)
{
  /* Force USB Reset */
  USB->CNTR = USB_CNTR_FRES;
  /* Clear Interrupt Status */
  USB->ISTR = 0;
  if (aConnect)
  {
    /* USB Reset Interrupt Mask */
    USB->CNTR = USB_CNTR_RESETM;
  }
  else
  {
    /* Switch Off USB Device */
    USB->CNTR = USB_CNTR_FRES | USB_CNTR_PDWN;
  }
}

//-----------------------------------------------------------------------------
/** @brief Resets USB peripheral
 *  @param None
 *  @return None
 *  @note Called automatically on USB Reset
 */
void USB_Reset(void)
{
  U16 CNTR;

  /* Double Buffering is not yet supported */
  /* Clear Interrupt Status */
  USB->ISTR = 0;

  CNTR = USB_CNTR_CTRM | USB_CNTR_RESETM;
  if (NULL != pUSB_CbSuspend) CNTR |= USB_CNTR_SUSPM;
  if (NULL != pUSB_CbWakeUp) CNTR |= USB_CNTR_WKUPM;
  if (NULL != pUSB_CbError) CNTR |= USB_CNTR_ERRM | USB_CNTR_PMAOVRM;
  if (NULL != pUSB_CbSOF) CNTR |= USB_CNTR_SOFM | USB_CNTR_ESOFM;
  USB->CNTR = CNTR;

  gEpFreeBuffAddr = gMaxEpCount * sizeof(EpBuffDescription_t);

  /* Set BTABLE Address */
  USB->BTABLE = 0;

  /* Setup Control Endpoint 0 */
  pEpBuffDscr->ADDR_TX = gEpFreeBuffAddr;
  gEpFreeBuffAddr += gCtrlEpMaxPacketSize;
  pEpBuffDscr->ADDR_RX = gEpFreeBuffAddr;
  gEpFreeBuffAddr += gCtrlEpMaxPacketSize;
  if (gCtrlEpMaxPacketSize > 62)
  {
    pEpBuffDscr->COUNT_RX = ((gCtrlEpMaxPacketSize << 5) - 1) | 0x8000;
  }
  else
  {
    pEpBuffDscr->COUNT_RX =   gCtrlEpMaxPacketSize << 9;
  }
  EPREG(0) = USB_EP_CONTROL | USB_EP_RX_VALID;

  /* Enable USB Default Address */
  USB->DADDR = USB_DADDR_EF | 0;
}

//-----------------------------------------------------------------------------
/** @brief Prepares internal variables for EP reconfiguration
 *  @param None
 *  @return None
 *  @note Should be called when EP0 IN/OUT has been already initialized
 */
void USB_PreapareReConfig(void)
{
  /* Recalculate Free EP Buffer pointer */
  gEpFreeBuffAddr = gMaxEpCount * sizeof(EpBuffDescription_t);
  gEpFreeBuffAddr += (gCtrlEpMaxPacketSize << 1);
}

//-----------------------------------------------------------------------------
/** @brief Suspends USB peripheral
 *  @param None
 *  @return None
 *  @note Called automatically on USB Suspend
 */
void USB_Suspend(void)
{
  /* Force Suspend */
  USB->CNTR |= USB_CNTR_FSUSP;
  /* Low Power Mode */
  USB->CNTR |= USB_CNTR_LP_MODE;
}

//-----------------------------------------------------------------------------
/** @brief Resumes USB peripheral
 *  @param None
 *  @return None
 *  @note Called automatically on USB Resume
 */
void USB_Resume(void)
{
  /* Performed by Hardware */
}

//-----------------------------------------------------------------------------
/** @brief USB Remote Wakeup
 *  @param None
 *  @return None
 *  @note Called automatically on USB Remote Wakeup
 */
void USB_WakeUp(void)
{
  /* Clear Suspend */
  USB->CNTR &= ~(USB_CNTR_FSUSP);
}

//-----------------------------------------------------------------------------
/** @brief USB Remote Wakeup Configuration Function
 *  @param aConfig - Enable/Disable
 *  @return None
 */
void USB_WakeUpConfigure(U32 aConfig)
{
  /* Not needed */
}

//-----------------------------------------------------------------------------
/** @brief USB Set Address Function
 *  @param aAddress - USB Address
 *  @return None
 */
void USB_SetAddress(U32 aAddress)
{
  USB->DADDR = USB_DADDR_EF | aAddress;
}

//-----------------------------------------------------------------------------
/** @brief USB Configure Function
 *  @param aConfig - Configure/Deconfigure
 *  @return None
 */
void USB_Configure(U32 aConfig)
{
  aConfig = aConfig;
}

//-----------------------------------------------------------------------------
/** @brief Configures USB Endpoint according to parameters
 *  @param aAddress - Endpoint address
 *  @param aMaxPacketSize - Maximum packet size
 *  @param aAttributes - Endpoint attributes
 *  @return None
 */
void USB_EpConfigure(U8 aAddress, U16 aMaxPacketSize, USB_EP_TYPE aType)
{
  /* Double Buffering is not yet supported */
  U32 num, val;

  num = aAddress & USB_EP_NUM_MASK;

  val = aMaxPacketSize;
  /* Check endpoint direction (IN - 0x80 Mask) */
  if (aAddress & USB_EP_DIR_MASK)
  {
    /* IN */
    USB_EpCfg[num].IMaxSize = aMaxPacketSize;
    (pEpBuffDscr + num)->ADDR_TX = gEpFreeBuffAddr;
    val = (val + 1) & ~1;
  }
  else
  {
    /* OUT */
    USB_EpCfg[num].OMaxSize = aMaxPacketSize;
    (pEpBuffDscr + num)->ADDR_RX = gEpFreeBuffAddr;
    if (val > 62)
    {
      val = (val + 31) & ~31;
      (pEpBuffDscr + num)->COUNT_RX = ((val << 5) - 1) | 0x8000;
    }
    else
    {
      val = (val + 1)  & ~1;
      (pEpBuffDscr + num)->COUNT_RX =   val << 9;
    }
  }
  gEpFreeBuffAddr += val;
  
  val = (aType << USB_EP_TYPE_MASK_Pos) & USB_EP_TYPE_MASK;
  val |= num;
  EPREG(num) = val;
}

//-----------------------------------------------------------------------------
/** @brief Sets Direction for USB Control Endpoint
 *  @param aDirection - Out (== 0), In (!= 0)
 *  @return None
 */
void USB_EpDirCtrl(U32 aDirection)
{
  /* Not needed */
}

//-----------------------------------------------------------------------------
/** @brief Enables USB Endpoint
 *  @param aNumber - Endpoint number
 *  @return None
 *  @note aNumber - bits 0..2 = Address, bit 7 = Direction
 */
void USB_EpEnable(U32 aNumber)
{
  usb_EpSetStatus(aNumber, USB_EP_TX_NAK | USB_EP_RX_VALID);
}

//-----------------------------------------------------------------------------
/** @brief Disables USB Endpoint
 *  @param aNumber - Endpoint Number
 *  @return None
 *  @note aNumber - bits 0..2 = Address, bit 7 = Direction
 */
void USB_EpDisable(U32 aNumber)
{
  usb_EpSetStatus(aNumber, USB_EP_TX_DIS | USB_EP_RX_DIS);
}

//-----------------------------------------------------------------------------
/** @brief Resets USB Endpoint
 *  @param aNumber - Endpoint Number
 *  @return None
 *  @note aNumber - bits 0..2 = Address, bit 7 = Direction
 */
void USB_EpReset(U32 aNumber)
{
  usb_EpReset(aNumber);
}

//-----------------------------------------------------------------------------
/** @brief Sets Stall for USB Endpoint
 *  @param aNumber - Endpoint Number
 *  @return None
 *  @note aNumber - bits 0..2 = Address, bit 7 = Direction
 */
void USB_EpSetStall(U32 aNumber)
{
  usb_EpSetStatus(aNumber, USB_EP_TX_STALL | USB_EP_RX_STALL);
}

//-----------------------------------------------------------------------------
/** @brief Clear Stall for USB Endpoint
 *  @param aNumber - Endpoint Number
 *  @return None
 *  @note aNumber - bits 0..2 = Address, bit 7 = Direction
 */
void USB_EpClrStall(U32 aNumber)
{
  usb_EpSetStatus(aNumber, USB_EP_TX_VALID | USB_EP_RX_VALID);
}

//-----------------------------------------------------------------------------
/** @brief Reads USB Endpoint count of data available
 *  @param aNumber - Endpoint Number
 *  @return Number of bytes available
 *  @note aNumber - bits 0..2 = Address, bit 7 = Direction
 */
U32 USB_EpIsDataAvailable(U32 aNumber)
{
  U32 num, cnt;

  /* Nothing is available for IN Endpoints */
  if (0 != (aNumber & USB_EP_DIR_MASK)) return 0;

  num = aNumber & USB_EP_NUM_MASK;
  cnt = (pEpBuffDscr + num)->COUNT_RX & USB_EP_COUNT_MASK;

  return (cnt);
}

//-----------------------------------------------------------------------------
/** @brief Reads USB Endpoint Data
 *  @param aNumber - Endpoint Number
 *  @param pData - Pointer to Data Buffer
 *  @param aSize - Number of bytes to read
 *  @return Number of bytes read
 *  @note aNumber - bits 0..2 = Address, bit 7 = Direction
 */
U32 USB_EpRead(U32 aNumber, U8 *pData, U32 aSize)
{
  /* Double Buffering is not yet supported */
  U32 num, cnt, *pv, n;
  U16 data;

  num = aNumber & USB_EP_NUM_MASK;

  cnt = (pEpBuffDscr + num)->COUNT_RX & USB_EP_COUNT_MASK;
  if (aSize < cnt)
  {
    return 0; /* The Buffer size is too small for received bytes */
  }

  pv  = (U32 *)(USB_PMAADDR + 2 * ((pEpBuffDscr + num)->ADDR_RX));

  for (n = 0; n < (cnt >> 1); n++)
  {
    *((__packed U16 *)pData) = *pv++;
    pData += 2;
  }
  if (1 == (cnt % 2))
  {
    *((__packed U16 *)&data) = *pv++;
    *pData = (U8)data;
    pData++;
  }
  usb_EpSetStatus(aNumber, USB_EP_RX_VALID);

  return (cnt);
}

//-----------------------------------------------------------------------------
/** @brief Writes USB Endpoint Data
 *  @param aNumber - Endpoint Number
 *  @param pData - Pointer to Data Buffer
 *  @param aSize - Number of bytes to write
 *  @return Number of bytes written
 *  @note aNumber - bits 0..2 = Address, bit 7 = Direction
 */
U32 USB_EpWrite(U32 aNumber, U8 *pData, U32 aSize)
{
  /* Double Buffering is not yet supported */
  U32 num, *pv, n;

  num = aNumber & USB_EP_NUM_MASK;

  if (aSize > USB_EpCfg[num].IMaxSize)
  {
    aSize = USB_EpCfg[num].IMaxSize;
  }

  pv  = (U32 *)(USB_PMAADDR + 2 * ((pEpBuffDscr + num)->ADDR_TX));
  for (n = 0; n < ((aSize + 1) >> 1); n++)
  {
    *pv++ = *((__packed U16 *)pData);
    pData += 2;
  }
  (pEpBuffDscr + num)->COUNT_TX = aSize;
  usb_EpSetStatus(aNumber, USB_EP_TX_VALID);

  return (aSize);
}

//-----------------------------------------------------------------------------
/** @brief Reads USB Endpoint Data
 *  @param aNumber - Endpoint Number
 *  @param pPutCb - Pointer to Callback that puts Byte to the Buffer
 *  @param aSize - Count of Bytes to be read
 *  @return Number of bytes read
 *  @note aNumber - bits 0..2 = Address, bit 7 = Direction
 */
U32 USB_EpReadToFifo(U32 aNumber, USB_CbEpPut pPutCb, U32 aSize)
{
  /* Double Buffering is not yet supported */
  U32 num, cnt, *pv, n;
  U8 data[2] = {0};

  num = aNumber & USB_EP_NUM_MASK;

  cnt = (pEpBuffDscr + num)->COUNT_RX & USB_EP_COUNT_MASK;
  if (aSize < cnt)
  {
    return 0; /* The Buffer size is too small for received bytes */
  }

  pv  = (U32 *)(USB_PMAADDR + 2 * ((pEpBuffDscr + num)->ADDR_RX));

  for (n = 0; n < (cnt >> 1); n++)
  {
    *((U16 *)data) = *pv++;
    pPutCb(&data[0]);
    pPutCb(&data[1]);
  }
  if (1 == (cnt % 2))
  {
    *((U16 *)data) = *pv++;
    pPutCb(&data[0]);
  }
  usb_EpSetStatus(aNumber, USB_EP_RX_VALID);

  return (cnt);
}

//-----------------------------------------------------------------------------
/** @brief Writes USB Endpoint Data
 *  @param aNumber - Endpoint Number
 *  @param pGetCb - Pointer to Callback that gets Byte from the Buffer
 *  @param aSize - Number of bytes to be written
 *  @return Number of bytes written
 *  @note aNumber - bits 0..2 = Address, bit 7 = Direction
 */
U32 USB_EpWriteFromFifo(U32 aNumber, USB_CbEpGet pGetCb, U32 aSize)
{
  /* Double Buffering is not yet supported */
  U32 num, *pv, n;
  U8 data[2] = {0};

  num = aNumber & USB_EP_NUM_MASK;

  if (aSize > USB_EpCfg[num].IMaxSize)
  {
    aSize = USB_EpCfg[num].IMaxSize;
  }

  pv  = (U32 *)(USB_PMAADDR + 2 * ((pEpBuffDscr + num)->ADDR_TX));
  for (n = 0; n < (aSize >> 1); n++)
  {
    pGetCb(&data[0]);
    pGetCb(&data[1]);
    *pv++ = *((U16 *)data);
  }
  if (1 == (aSize % 2))
  {
    pGetCb(&data[0]);
    data[1] = 0;
    *pv++ = *((U16 *)data);
  }
  (pEpBuffDscr + num)->COUNT_TX = aSize;
  usb_EpSetStatus(aNumber, USB_EP_TX_VALID);

  return (aSize);
}

//-----------------------------------------------------------------------------
/** @brief Gets USB Last Frame Number
 *  @param None
 *  @return Frame Number
 */
U32 USB_GetFrame(void)
{
  return (USB->FNR & USB_FNR_FN);
}

//-----------------------------------------------------------------------------
/** @brief USB Interrupt Service Routine
 *  @param None
 *  @return None
 */
void USB_IRQHandler(void)
{
  U32 istr, num, val;

  istr = USB->ISTR;

  /* USB Reset Request */
  if (istr & USB_ISTR_RESET)
  {
    USB_Reset();
    if (NULL != pUSB_CbReset) pUSB_CbReset();
    USB->ISTR = (U16)~(USB_ISTR_RESET);
  }

  /* USB Suspend Request */
  if (istr & USB_ISTR_SUSP)
  {
    USB_Suspend();
    if (NULL != pUSB_CbSuspend) pUSB_CbSuspend();
    USB->ISTR = (U16)~(USB_ISTR_SUSP);
  }

  /* USB Wakeup */
  if (istr & USB_ISTR_WKUP)
  {
    USB_WakeUp();
    if (NULL != pUSB_CbWakeUp) pUSB_CbWakeUp();
    USB->ISTR = (U16)~(USB_ISTR_WKUP);
  }

  /* Start of Frame */
  if (istr & USB_ISTR_SOF)
  {
    if (NULL != pUSB_CbSOF) pUSB_CbSOF();
    USB->ISTR = (U16)~(USB_ISTR_SOF);
  }

  /* PMA Over/underrun */
  if (istr & USB_ISTR_PMAOVR)
  {
    if (NULL != pUSB_CbError) pUSB_CbError(1);
    USB->ISTR = (U16)~(USB_ISTR_PMAOVR);
  }

  /* Error: No Answer, CRC Error, Bit Stuff Error, Frame Format Error */
  if (istr & USB_ISTR_ERR)
  {
    if (NULL != pUSB_CbError) pUSB_CbError(0);
    USB->ISTR = (U16)~(USB_ISTR_ERR);
  }

  /* Endpoint Interrupts */
  while ((istr = USB->ISTR) & USB_ISTR_CTR)
  {
    USB->ISTR = (U16)~(USB_ISTR_CTR);

    num = istr & USB_ISTR_EP_ID;

    val = EPREG(num);
    if (val & USB_EP_CTR_RX)
    {
      EPREG(num) = val & ~USB_EP_CTR_RX & USB_EPREG_MASK;
      if (NULL != USB_EpCfg[num].OCb)
      {
        if (val & USB_EP_SETUP)
        {
          USB_EpCfg[num].OCb(USB_EVNT_EP_SETUP);
        }
        else
        {
          USB_EpCfg[num].OCb(USB_EVNT_EP_OUT);
        }
      }
    }
    if (val & USB_EP_CTR_TX)
    {
      EPREG(num) = val & ~USB_EP_CTR_TX & USB_EPREG_MASK;
      if (NULL != USB_EpCfg[num].ICb)
      {
        USB_EpCfg[num].ICb(USB_EVNT_EP_IN);
      }
    }
  }
}
