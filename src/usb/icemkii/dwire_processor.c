#include <string.h>

#include "types.h"
#include "board.h"
#include "gpio.h"
#include "uart.h"
#include "debug.h"
#include "tim.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

/* -------------------------------------------------------------------------- */

#define DWIRE_DEBUG

#ifdef DWIRE_DEBUG
#  define DWIRE_LOG      DBG
#else
#  define DWIRE_LOG(...)
#endif

#define DWIRE_TX_COMPLETE              (1 << 0)
#define DWIRE_RX_COMPLETE              (1 << 1)
#define DWIRE_BAUD_COMPLETE            (1 << 2)
#define DWIRE_TIMEOUT                  (500)
#define DWIRE_BAUD_MEAS_COUNT          (6)
#define DWIRE_BAUD_MEAS_START_IDX      (2)

#define DWIRE_DISABLE                  (0x06)
#define DWIRE_RESET                    (0x07)
#define DWIRE_GO                       (0x20)
#define DWIRE_SS                       (0x21)
#define DWIRE_SS_INST                  (0x23)
#define DWIRE_RESUME                   (0x30)
#define DWIRE_RESUME_SS                (0x31)
#define DWIRE_FLAG_RUN                 (0x60)
#define DWIRE_FLAG_RUN_TO_CURSOR       (0x61)
#define DWIRE_FLAG_STEP_OUT            (0x63)
#define DWIRE_FLAG_STEP_IN             (0x79)
#define DWIRE_FLAG_SINGLE_STEP         (0x7A)
#define DWIRE_FLAG_MEMORY              (0x66)
#define DWIRE_FLAG_INST                (0x64)
#define DWIRE_FLAG_FLASH_INST          (0x44)
#define DWIRE_BAUD_128                 (0x83)
#define DWIRE_BAUD_64                  (0x82)
#define DWIRE_BAUD_32                  (0x81)
#define DWIRE_BAUD_16                  (0x80)
#define DWIRE_SET_PC_LOW               (0xC0)
#define DWIRE_SET_PC                   (0xD0)
#define DWIRE_SET_BP                   (0xD1)
#define DWIRE_SET_BP_LOW               (0xC1)
#define DWIRE_SET_IR                   (0xD2)
#define DWIRE_GET_PC                   (0xF0)
#define DWIRE_GET_BP                   (0xF1)
#define DWIRE_GET_IR                   (0xF2)
#define DWIRE_GET_SIG                  (0xF3)
#define DWIRE_RW_MODE                  (0xC2)
#define DWIRE_MODE_READ_SRAM           (0x00)
#define DWIRE_MODE_READ_REGS           (0x01)
#define DWIRE_MODE_READ_FLASH          (0x02)
#define DWIRE_MODE_WRITE_SRAM          (0x04)
#define DWIRE_MODE_WRITE_REGS          (0x05)

#define DWIRE_BUFFER_SIZE              (512 + 32)

#define REG_NUM_LO                     (0x00)
#define REG_NUM_HI                     (0x1F)
#define REG_IO_NUM_LO                  (0x20)
#define REG_IO_NUM_HI                  (0x5F)
#define REG_IO_EXT_NUM_LO              (0x60)
#define REG_IO_EXT_NUM_HI              (0xFF)

/* -------------------------------------------------------------------------- */

typedef enum
{
  Get = 0,
  Set,
} Xet_t;

typedef struct
{
  /* Sync parameters */
  U16                baudRateValue[DWIRE_BAUD_MEAS_COUNT];
  U8                 baudRateCnt;
  /* Rx/Tx parameters */
  struct
  {
    FW_BOOLEAN       txInProgress : 1;
    FW_BOOLEAN       txOk : 1;
  };
  U8                 txBuffer[DWIRE_BUFFER_SIZE];
  U32                txLen;
  U32                txCnt;
  U8                 rxBuffer[DWIRE_BUFFER_SIZE];
  U32                rxLen;
  /* RTOS objects */
  EventGroupHandle_t events;
  /* Device parameters */
  U8                 dwdr;
  U8                 spmcsr;
  U16                sramSize;
  U16                flashSize;
  U16                flashPageSize;
  U16                eepromSize;
  U16                basePC;
  /* dWire parameters */
  U16                pc;
  U16                z;
} DWire_t, * DWire_p;

/* -------------------------------------------------------------------------- */

static DWire_t gDWire = {0};

/* -------------------------------------------------------------------------- */

/** @brief Puts received Byte from UART to the Rx buffer
 *  @param pByte - Pointer to the container with Byte
 *  @return True
 */
static FW_BOOLEAN uart_RxByte(U8 * pByte);

/** @brief Receive complete callback
 *  @param pByte - Optional pointer to the latest received byte (def. NULL)
 *  @return True
 */
static FW_BOOLEAN uart_RxComplete(U8 * pByte);

/** @brief Gets Byte that need to be transmitted from the Tx buffer
 *  @param pByte - Pointer to the container for Byte
 *  @return True if the Tx buffer is not empty, else - False
 */
static FW_BOOLEAN uart_TxByte(U8 * pByte);

/** @brief Transmit complete callback
 *  @param pByte - Optional pointer to the latest transmitted byte (def. NULL)
 *  @return True
 */
static FW_BOOLEAN uart_TxComplete(U8 * pByte);

/** @brief Waits for dWire events
 *  @param eventMask - The set of events to wait
 *  @return True - if all the events were set, False - in case of timeout
 */
static FW_BOOLEAN uart_WaitForComplete(U32 eventMask);

/** @brief Performs the dWire read operation
 *  @param count - Count of bytes expected to be read
 *  @return True - if expected count of bytes were read
 */
static FW_BOOLEAN uart_Read(U32 count);

/** @brief Performs the dWire write operation
 *  @param None
 *  @return True - if all the bytes were written
 */
static FW_BOOLEAN uart_Write(void);

/** @brief Performs the dWire sequental write and read operations
 *  @param count - The count of bytes expected to be read
 *  @return True - if the expected count of bytes were read after writing
 */
static FW_BOOLEAN uart_WriteRead(U32 count);

/** @brief Timer input capture callback
 *  @param channel - Input capture channel
 *  @param value - Input captured value
 *  @return None
 */
static void dwire_BaudCaptComplete(TIM_CH_t channel, U16 value);

/** @brief Clears the dWire Rx/Tx buffers, resets the Rx/Tx counters
 *  @param None
 *  @return None
 */
static void dwire_Clear(void);

/** @brief Adds the set PC sequence to the dWire Tx buffer
 *  @param value - PC value
 *  @return None
 */
static void dwire_Append_SetPC(U16 value);

/** @brief Adds the set Break Point sequence to the dWire Tx buffer
 *  @param value - BP value
 *  @return None
 */
static void dwire_Append_SetBP(U16 value);

/** @brief Adds the byte to the dWire Tx buffer
 *  @param value - Byte's value
 *  @return None
 */
static void dwire_Append(U8 value);

/** @brief Adds the buffer to the dWire Tx buffer
 *  @param pRaw - Container of bytes
 *  @param count - Count of bytes in the container
 *  @return None
 */
static void dwire_Append_Raw(U8 * pRaw, U16 count);

/** @brief Adds the instruction sequence to the dWire Tx buffer
 *  @param value - Instruction
 *  @return None
 */
static void dwire_Append_Instr(U16 value);

/** @brief Adds the In register sequence to the dWire Tx buffer
 *  @param reg - Destination register
 *  @param ioreg - Source register
 *  @return None
 */
static void dwire_Append_I_Instr(U8 reg, U16 ioreg);

/** @brief Adds the Out register sequence to the dWire Tx buffer
 *  @param ioreg - Destination register
 *  @param reg - Source register
 *  @return None
 */
static void dwire_Append_O_Instr(U16 ioreg, U8 reg);

/** @brief Adds the In DWDR sequence to the dWire Tx buffer
 *  @param reg - Source register
 *  @return None
 */
static void dwire_Append_I_DWDR(U8 reg);

/** @brief Adds the Out DWDR sequence to the dWire Tx buffer
 *  @param reg - Destination register
 *  @return None
 */
static void dwire_Append_O_DWDR(U8 reg);

/** @brief Adds the Out SPM CSR sequence to the dWire Tx buffer
 *  @param reg - Destination register
 *  @return None
 */
static void dwire_Append_O_SPMCSR(U8 reg);

/** @brief Adds the LPM instruction to the dWire Tx buffer
 *  @param reg - Destination register
 *  @return None
 */
static void dwire_Append_LPM(U8 reg);

/** @brief Adds the SPM instruction to the dWire Tx buffer
 *  @param None
 *  @return None
 */
static void dwire_Append_SPM(void);

/** @brief Adds the SPM Z instruction to the dWire Tx buffer
 *  @param None
 *  @return None
 */
static void dwire_Append_SPM_Z(void);

/** @brief Adds the LDI instruction to the dWire Tx buffer
 *  @param reg - Register
 *  @param value - Value
 *  @return None
 */
static void dwire_Append_LDI(U8 reg, U8 value);

/** @brief Starts the operation with the SRAM (loads address)
 *  @param xet - The type of operation
 *  @param address - The address in the SRAM
 *  @return TRUE in case of success
 */
FW_BOOLEAN dwire_Start_SRAM(Xet_t xet, U16 address);

/** @brief Performs the next operation with the SRAM
 *  @param xet - The type of operation
 *  @param pValue - The container for the value used by the operation
 *  @return TRUE in case of success
 */
FW_BOOLEAN dwire_Next_SRAM(Xet_t xet, U8 * pValue);

/** @brief Performs the operation with the SRAM
 *  @param xet - The type of operation
 *  @param address - The address in the SRAM
 *  @param pRaw - The container for the data used by the operation
 *  @param length - The length of the data
 *  @return TRUE in case of success
 */
FW_BOOLEAN dwire_SRAM(Xet_t xet, U16 address, U8 * pRaw, U16 length);

/* -------------------------------------------------------------------------- */

void DWire_Init(void)
{
  /* Event Group for RTOS flow control */
  if (NULL == gDWire.events)
  {
    gDWire.events = xEventGroupCreate();
  }

  /* Init some fields */
  gDWire.txInProgress  = FW_FALSE;
  gDWire.txOk          = FW_FALSE;
  gDWire.txLen         = 0;
  gDWire.txCnt         = 0;
  gDWire.rxLen         = 0;
  gDWire.dwdr          = 0;
  gDWire.spmcsr        = 0;
  gDWire.sramSize      = 0;
  gDWire.flashSize     = 0;
  gDWire.flashPageSize = 0;
  gDWire.eepromSize    = 0;
  gDWire.basePC        = 0;
  gDWire.pc            = 0;

  /* Init the test pins (temporarily) */
  GPIO_Init(GPIOB, 5, GPIO_TYPE_OUT_PP_50MHZ, 0);
  GPIO_Init(GPIOB, 9, GPIO_TYPE_OUT_PP_50MHZ, 0);
  GPIO_Init(GPIOA, 8, GPIO_TYPE_OUT_PP_50MHZ, 0);

  /* Create the Debug Wire task */
  /* xTaskCreate
  (
    dwire_Task,
    "DWIRE",
    2 * configMINIMAL_STACK_SIZE,
    NULL,
    tskIDLE_PRIORITY + 1,
    NULL
  ); */
}

/* -------------------------------------------------------------------------- */

void DWire_SetParams(U8 dwdr, U8 spmcsr, U16 basePC)
{
  gDWire.dwdr   = dwdr;
  gDWire.spmcsr = spmcsr;
  gDWire.basePC = basePC;
}

/* -------------------------------------------------------------------------- */

void DWire_SetMemParams(U16 rSize, U16 fSize, U16 fPageSize, U16 eSize)
{
  gDWire.sramSize      = rSize;
  gDWire.flashSize     = fSize;
  gDWire.flashPageSize = fPageSize;
  gDWire.eepromSize    = eSize;
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN DWire_Sync(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U32 idx           = 0;
  U32 baudrate      = 0;
  U16 time          = 0;

  GPIO_Hi(GPIOA, 8);

  DWIRE_LOG("DWire: Sync\r\n");

  memset(gDWire.baudRateValue, 0, sizeof(gDWire.baudRateValue));
  gDWire.baudRateCnt = 0;

  /* Deinit the UART */
  UART_DeInit(UART2);

  /* Init timer */
  TIM2_InitInputCapture(TIM_CH3, dwire_BaudCaptComplete);

  /* Activate the Reset pin */
  GPIO_Init(TIM2_CH3_PORT, TIM2_CH3_PIN, GPIO_TYPE_OUT_OD_2MHZ, 0);
  vTaskDelay(5);

  /* Enable the timer */
  TIM2_Enable();

  /* Release the Reset pin */
  GPIO_Init(TIM2_CH3_PORT, TIM2_CH3_PIN, GPIO_TYPE_IN_FLOATING, 0);

  /* Wait for measurement is complete */
  result = uart_WaitForComplete(DWIRE_BAUD_COMPLETE);

  /* Disable the timer */
  TIM2_Disable();
  TIM2_DeInit();

  if (FW_TRUE == result)
  {
    /* Calculate the baudrate */
    baudrate = 0;
    for (idx = DWIRE_BAUD_MEAS_START_IDX; idx < DWIRE_BAUD_MEAS_COUNT; idx++)
    {
      time = gDWire.baudRateValue[idx] - gDWire.baudRateValue[idx - 1];
      DWIRE_LOG(" - Meas[%d]  = %d\r\n", idx - 2, time);

      baudrate += time;
    }
    baudrate /= (DWIRE_BAUD_MEAS_COUNT - DWIRE_BAUD_MEAS_START_IDX);
    baudrate = 2 * CPUClock / baudrate;
    DWIRE_LOG(" - BaudRate = %d\r\n", baudrate);

    /* Initialize the UART */
    UART_Init
    (
      UART2,
      baudrate,
      uart_RxByte,
      uart_RxComplete,
      uart_TxByte,
      uart_TxComplete
    );

    /* Enable the bidirectional mode */
    UART_BiDirModeEn(UART2);

    /* Activate the Reset pin */
    GPIO_Init(UART2_RTX_PORT, UART2_RTX_PIN, GPIO_TYPE_OUT_OD_2MHZ, 0);
    vTaskDelay(5);

    /* Release the Reset pin */
    GPIO_Init(UART2_RTX_PORT, UART2_RTX_PIN, GPIO_TYPE_ALT_OD_2MHZ, 1);

    /* Wait for the Sync byte from the target */
    idx = uart_Read(1);

    if ( (1 == idx) && (0x55 == gDWire.rxBuffer[0]) )
    {
      DWIRE_LOG("DWire: In sync!\r\n");
    }
    else
    {
      result = FW_FALSE;
      DWIRE_LOG("DWire: Sync error!\r\n");
    }
  }

  GPIO_Lo(GPIOA, 8);

  return result;
}

/* -------------------------------------------------------------------------- */

U16 DWire_GetSignature(void)
{
  U16 result = 0;

  DWIRE_LOG("DWire: Read Signature\r\n");

  dwire_Clear();
  dwire_Append(DWIRE_GET_SIG);
  if (FW_TRUE == uart_WriteRead(2))
  {
    result = gDWire.rxBuffer[1] + (gDWire.rxBuffer[0] << 8);
    DWIRE_LOG("DWire: Signature = %04X\r\n", result);
  }

  return result;
}

/* -------------------------------------------------------------------------- */

U16 DWire_GetPC(void)
{
  U16 result = 0;

  DWIRE_LOG("DWire: Read PC\r\n");

  dwire_Clear();
  dwire_Append(DWIRE_GET_PC);
  if (FW_TRUE == uart_WriteRead(2))
  {
    result = (gDWire.rxBuffer[1] + (gDWire.rxBuffer[0] << 8));
    result -= (gDWire.basePC + 1);
    DWIRE_LOG("DWire: PC = %04X\r\n", result);
  }

  return result;
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN DWire_Disable(void)
{
  FW_BOOLEAN result = FW_FALSE;

  DWIRE_LOG("DWire: Disable\r\n");

  dwire_Clear();
  dwire_Append(DWIRE_DISABLE);
  result = uart_Write();
  if (FW_TRUE == result)
  {
    DWIRE_LOG("DWire: Disabled!\r\n");
  }

  return result;
}

/* -------------------------------------------------------------------------- */

U8 DWire_GetReg(U8 reg)
{
  U8 result = 0;

  if (REG_NUM_HI < reg) return result;

  DWIRE_LOG("DWire: Read register %02X\r\n", reg);

  dwire_Clear();
  dwire_Append(DWIRE_FLAG_INST);
  dwire_Append_O_DWDR(reg);
  if (FW_TRUE == uart_WriteRead(1))
  {
    result = gDWire.rxBuffer[0];
  }

  return result;
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN DWire_SetReg(U8 reg, U8 value)
{
  if (REG_NUM_HI < reg) return FW_FALSE;

  DWIRE_LOG("DWire: Write register %02X - %02X\r\n", reg, value);

  dwire_Clear();
  dwire_Append(DWIRE_FLAG_INST);
  dwire_Append_I_DWDR(reg);
  dwire_Append(value);
  return uart_Write();
}

/* -------------------------------------------------------------------------- */

U8 DWire_GetIOReg(U8 reg)
{
  U8 result = 0;

  if ((REG_IO_NUM_LO > reg) || (REG_IO_NUM_HI < reg)) return result;

  DWIRE_LOG("DWire: Read IO register %02X\r\n", reg);

  dwire_Clear();
  /* in R0,reg */
  dwire_Append(DWIRE_FLAG_INST);
  dwire_Append_I_Instr(0, (reg - REG_IO_NUM_LO));
  /* out DWDR,R0 */
  dwire_Append(DWIRE_FLAG_INST);
  dwire_Append_O_DWDR(0);
  if (FW_TRUE == uart_WriteRead(1))
  {
    result = gDWire.rxBuffer[0];
  }

  return result;
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN DWire_SetIOReg(U8 reg, U8 value)
{
  FW_BOOLEAN result = FW_FALSE;

  if ((REG_IO_NUM_LO > reg) || (REG_IO_NUM_HI < reg)) return FW_FALSE;

  DWIRE_LOG("DWire: Write IO register %02X - %02X\r\n", reg, value);

  dwire_Clear();
  /* in R0,DWDR */
  dwire_Append(DWIRE_FLAG_INST);
  dwire_Append_I_DWDR(0);
  dwire_Append(value);
  result = uart_Write();

  if (FW_TRUE == result)
  {
    dwire_Clear();
    /* out reg,R0 */
    dwire_Append(DWIRE_FLAG_INST);
    dwire_Append_O_Instr((reg - REG_IO_NUM_LO), 0);
    result = uart_Write();
  }

  return result;
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN DWire_GetRegs(U8 first, U8 * pRaw, U8 count)
{
  FW_BOOLEAN result = FW_FALSE;

  if ( (REG_NUM_HI < first) || (REG_NUM_HI < (first + count - 1)) )
  {
    return result;
  }
  if ( (NULL == pRaw) || (0 == count) )
  {
    return result;
  }

  DWIRE_LOG("DWire: Read Regs\r\n");

  dwire_Clear();
  dwire_Append_SetPC(first);
  dwire_Append_SetBP(first + count);
  dwire_Append(DWIRE_FLAG_MEMORY);
  dwire_Append(DWIRE_RW_MODE);
  dwire_Append(DWIRE_MODE_READ_REGS);
  dwire_Append(DWIRE_GO);

  if (FW_TRUE == uart_WriteRead(count))
  {
    memcpy(pRaw, gDWire.rxBuffer, count);
    result = FW_TRUE;
  }

  return result;
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN DWire_SetRegs(U8 first, U8 * pRaw, U8 count)
{
  if ((REG_NUM_HI < first) || (REG_NUM_HI < (first + count - 1)))
  {
    return FW_FALSE;
  }
  if ( (NULL == pRaw) || (0 == count) )
  {
    return FW_FALSE;
  }

  DWIRE_LOG("DWire: Write Regs\r\n");

  dwire_Clear();
  dwire_Append_SetPC(first);
  dwire_Append_SetBP(first + count);
  dwire_Append(DWIRE_FLAG_MEMORY);
  dwire_Append(DWIRE_RW_MODE);
  dwire_Append(DWIRE_MODE_WRITE_REGS);
  dwire_Append(DWIRE_GO);
  dwire_Append_Raw(pRaw, count);
  return uart_Write();
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN DWire_GetSRAM(U16 address, U8 * pRaw, U16 length)
{
  DWIRE_LOG("DWire: Read SRAM\r\n");

  return dwire_SRAM(Get, address, pRaw, length);
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN DWire_SetSRAM(U16 address, U8 * pRaw, U16 length)
{
  DWIRE_LOG("DWire: Write SRAM\r\n");

  return dwire_SRAM(Set, address, pRaw, length);
}

/* -------------------------------------------------------------------------- */
















FW_BOOLEAN dwire_Set_Z(U16 address)
{
  return DWire_SetRegs(30, (U8 *)&address, 2); // BYTES(WORD_LE(addr)));
}

/* -------------------------------------------------------------------------- */


























//#define DWIRE_DEV_SIG 0x950f
//#define DWIRE_DEV_IO_SIZE 224
//#define DWIRE_DEV_SRAM_SIZE 2048
//#define DWIRE_DEV_EEPROM_SIZE 1024
//#define DWIRE_DEV_FLASH_SIZE 32768
//#define DWIRE_DEV_DWDR 0x31
//#define DWIRE_DEV_SPMCSR 0x37
//#define DWIRE_DEV_PAGESIZE 128
//#define DWIRE_DEV_BOOT 0x0000
//#define DWIRE_DEV_BOOTFLAGS 0
//#define DWIRE_DEV_EECR 0x1f
//#define DWIRE_DEV_EEARH 0x22
//static U8                 gTxBuffer[16]                          = {0};
//static U32                gTxLen                                 = 0;
//static U8                 gRxBuffer[16]                          = {0};
//static U32                gRxLen                                 = 0;
//static EventGroupHandle_t gEvents                                = NULL;
//static U16                gBaudRateValue[DWIRE_BAUD_MEAS_COUNT]  = {0};
//static U8                 gBaudRateCnt                           = 0;



/* -------------------------------------------------------------------------- */

static FW_BOOLEAN uart_RxByte(U8 * pByte)
{
  GPIO_Hi(GPIOB, 5);

  if (FW_TRUE == gDWire.txInProgress)
  {
    gDWire.txOk &= (FW_BOOLEAN)(gDWire.txBuffer[gDWire.rxLen] == *pByte);
    gDWire.rxLen++;
  }
  else if (sizeof(gDWire.rxBuffer) > gDWire.rxLen)
  {
    gDWire.rxBuffer[gDWire.rxLen] = *pByte;
    gDWire.rxLen++;
  }

  GPIO_Lo(GPIOB, 5);

  return FW_TRUE;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN uart_RxComplete(U8 * pByte)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  GPIO_Hi(GPIOB, 5);

  (void)xEventGroupSetBitsFromISR
        (
          gDWire.events,
          DWIRE_RX_COMPLETE,
          &xHigherPriorityTaskWoken
        );

  GPIO_Hi(GPIOA, 8);

  if (xHigherPriorityTaskWoken)
  {
    taskYIELD();
  }

  GPIO_Lo(GPIOB, 5);

  return FW_TRUE;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN uart_TxByte(U8 * pByte)
{
  GPIO_Hi(GPIOB, 9);
  GPIO_Lo(GPIOB, 9);

  if (gDWire.txCnt < gDWire.txLen)
  {
    *pByte = gDWire.txBuffer[gDWire.txCnt];
    gDWire.txCnt++;
    return FW_TRUE;
  }
  else
  {
    return FW_FALSE;
  }
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN uart_TxComplete(U8 * pByte)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  GPIO_Hi(GPIOB, 9);

  if (FW_TRUE == gDWire.txOk)
  {
    (void)xEventGroupSetBitsFromISR
          (
            gDWire.events,
            DWIRE_TX_COMPLETE,
            &xHigherPriorityTaskWoken
          );

    if (xHigherPriorityTaskWoken)
    {
      taskYIELD();
    }
  }
  gDWire.txInProgress = FW_FALSE;
  gDWire.rxLen = 0;

  GPIO_Lo(GPIOB, 9);

  return FW_TRUE;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN uart_WaitForComplete(U32 eventMask)
{
  FW_BOOLEAN result = FW_TRUE;
  EventBits_t events = 0;

  events = xEventGroupWaitBits
           (
             gDWire.events,
             eventMask,
             pdTRUE, /* Clear bits   */
             pdTRUE, /* Wait for all */
             DWIRE_TIMEOUT
           );

  if (eventMask != (events & eventMask))
  {
    result = FW_FALSE;
  }
  if (0 != (events & DWIRE_RX_COMPLETE)) GPIO_Lo(GPIOA, 8);

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN uart_Read(U32 count)
{
  FW_BOOLEAN result = FW_FALSE;
  U32 i = 0;

  gDWire.rxLen = 0;

  UART_RxStart(UART2);

  if (FW_TRUE == uart_WaitForComplete(DWIRE_RX_COMPLETE))
  {
    DWIRE_LOG("DWire <-- ");
    for (i = 0; i < gDWire.rxLen; i++)
    {
      DWIRE_LOG("%02X ", gDWire.rxBuffer[i]);
    }
    DWIRE_LOG("\r\n");

    result = (FW_BOOLEAN)(gDWire.rxLen == count);
  }

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN uart_Write(void)
{
  U32 i = 0;

  gDWire.rxLen = 0;
  gDWire.txCnt = 0;
  gDWire.txInProgress = FW_TRUE;
  gDWire.txOk = FW_TRUE;

  DWIRE_LOG("DWire --> ");
  for (i = 0; i < gDWire.txLen; i++)
  {
    DWIRE_LOG("%02X ", gDWire.txBuffer[i]);
  }
  DWIRE_LOG("\r\n");

  UART_TxStart(UART2);

  return uart_WaitForComplete(DWIRE_TX_COMPLETE | DWIRE_RX_COMPLETE);
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN uart_WriteRead(U32 count)
{
  FW_BOOLEAN result = FW_FALSE;
  U32 i = 0;

  gDWire.rxLen = 0;
  gDWire.txCnt = 0;
  gDWire.txInProgress = FW_TRUE;
  gDWire.txOk = FW_TRUE;

  DWIRE_LOG("DWire --> ");
  for (i = 0; i < gDWire.txLen; i++)
  {
    DWIRE_LOG("%02X ", gDWire.txBuffer[i]);
  }
  DWIRE_LOG("\r\n");

  UART_TxStart(UART2);

  result = uart_WaitForComplete(DWIRE_TX_COMPLETE | DWIRE_RX_COMPLETE);

  if ( (FW_TRUE == result) && (0 < gDWire.rxLen) )
  {
    DWIRE_LOG("DWire <-- ");
    for (i = 0; i < gDWire.rxLen; i++)
    {
      DWIRE_LOG("%02X ", gDWire.rxBuffer[i]);
    }
    DWIRE_LOG("\r\n");
  }

  result &= (FW_BOOLEAN)(gDWire.rxLen == count);

  return result;
}

/* -------------------------------------------------------------------------- */

static void dwire_BaudCaptComplete(TIM_CH_t channel, U16 value)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  GPIO_Hi(GPIOB, 5);
  gDWire.baudRateValue[gDWire.baudRateCnt++] = value;
  if (DWIRE_BAUD_MEAS_COUNT == gDWire.baudRateCnt)
  {
    (void)xEventGroupSetBitsFromISR
      (
        gDWire.events,
        DWIRE_BAUD_COMPLETE,
        &xHigherPriorityTaskWoken
      );

    if (xHigherPriorityTaskWoken)
    {
      taskYIELD();
    }
  }
  GPIO_Lo(GPIOB, 5);
}

/* -------------------------------------------------------------------------- */

static void dwire_Clear(void)
{
  memset(gDWire.txBuffer, 0, sizeof(gDWire.txBuffer));
  memset(gDWire.rxBuffer, 0, sizeof(gDWire.rxBuffer));
  gDWire.txLen = 0;
}

/* -------------------------------------------------------------------------- */

static void dwire_Append_SetPC(U16 value)
{
  value += gDWire.basePC;
  gDWire.txBuffer[gDWire.txLen++] = DWIRE_SET_PC;
  gDWire.txBuffer[gDWire.txLen++] = (value >> 8);
  gDWire.txBuffer[gDWire.txLen++] = (value & 0xFF);
}

/* -------------------------------------------------------------------------- */

static void dwire_Append_SetBP(U16 value)
{
  value += gDWire.basePC;
  gDWire.txBuffer[gDWire.txLen++] = DWIRE_SET_BP;
  gDWire.txBuffer[gDWire.txLen++] = (value >> 8);
  gDWire.txBuffer[gDWire.txLen++] = (value & 0xFF);
}

/* -------------------------------------------------------------------------- */

static void dwire_Append(U8 value)
{
  gDWire.txBuffer[gDWire.txLen++] = value;
}

/* -------------------------------------------------------------------------- */

static void dwire_Append_Raw(U8 * pRaw, U16 count)
{
  memcpy(&gDWire.txBuffer[gDWire.txLen], pRaw, count);
  gDWire.txLen += count;
}

/* -------------------------------------------------------------------------- */

static void dwire_Append_Instr(U16 value)
{
  gDWire.txBuffer[gDWire.txLen++] = DWIRE_SET_IR;
  gDWire.txBuffer[gDWire.txLen++] = (value >> 8);
  gDWire.txBuffer[gDWire.txLen++] = (value & 0xFF);
  gDWire.txBuffer[gDWire.txLen++] = DWIRE_SS_INST;
}

/* -------------------------------------------------------------------------- */

static void dwire_Append_I_Instr(U8 reg, U16 ioreg)
{
  U16 value = 0xB000;

  value |= ((ioreg << 5) & 0x600);
  value |= ((reg << 4) & 0x01F0);
  value |= (ioreg & 0x000F);

  /* DWire_Inst(dwire,
     0xb000 |
     ((ioreg << 5) & 0x600) |
     ((reg << 4) & 0x01F0) |
     (ioreg & 0x000F));
   */
  dwire_Append_Instr(value);
}

/* -------------------------------------------------------------------------- */

static void dwire_Append_O_Instr(U16 ioreg, U8 reg)
{
  U16 value = 0xB800;

  value |= ((ioreg << 5) & 0x600);
  value |= ((reg << 4) & 0x01F0);
  value |= (ioreg & 0x000F);

  /* DWire_Inst(dwire,
     0xb800 |
     ((ioreg << 5) & 0x600) |
     ((reg << 4) & 0x01F0) |
     (ioreg & 0x000F));
   */
  dwire_Append_Instr(value);
}

/* -------------------------------------------------------------------------- */

static void dwire_Append_I_DWDR(U8 reg)
{
  dwire_Append_I_Instr(reg, gDWire.dwdr);
}

/* -------------------------------------------------------------------------- */

static void dwire_Append_O_DWDR(U8 reg)
{
  dwire_Append_O_Instr(gDWire.dwdr, reg);
}

/* -------------------------------------------------------------------------- */

static void dwire_Append_O_SPMCSR(U8 reg)
{
  dwire_Append_O_Instr(gDWire.spmcsr, reg);
}

/* -------------------------------------------------------------------------- */

static void dwire_Append_LPM(U8 reg)
{
  dwire_Append_Instr(0x9004 | (reg << 4));
}

/* -------------------------------------------------------------------------- */

static void dwire_Append_SPM(void)
{
  dwire_Append_Instr(0x95E8);
}

/* -------------------------------------------------------------------------- */

static void dwire_Append_SPM_Z(void)
{
  dwire_Append_Instr(0x95F8);
}

/* -------------------------------------------------------------------------- */

static void dwire_Append_LDI(U8 reg, U8 value)
{
  U16 inst = 0xE000;

  inst |= ((reg << 4) & 0xF0);
  inst |= (value & 0xF);
  inst |= ((value << 4) & 0xF00);

  /* DWire_Inst(dwire,
     0xe000 |
     ((reg << 4) & 0xf0) |
     (val & 0xf) |
     ((val << 4) & 0xf00));
   */
  dwire_Append_Instr(inst);
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN dwire_Start_SRAM(Xet_t xet, U16 address)
{
  /* Set Z register to the address */
  if (FW_FALSE == DWire_SetRegs(30, (U8 *)&address, 2))
  {
    return FW_FALSE;
  }

  dwire_Clear();
  dwire_Append(DWIRE_FLAG_MEMORY);

  if (Set == xet)
  {
    dwire_Append_SetPC(1);
    dwire_Append_SetBP(3);
    dwire_Append(DWIRE_RW_MODE);
    dwire_Append(DWIRE_MODE_WRITE_SRAM);
  }
  else
  {
    dwire_Append_SetPC(0);
    dwire_Append_SetBP(2);
    dwire_Append(DWIRE_RW_MODE);
    dwire_Append(DWIRE_MODE_READ_SRAM);
  }

  return uart_Write();
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN dwire_Next_SRAM(Xet_t xet, U8 * pValue)
{
  FW_BOOLEAN result = FW_FALSE;

  dwire_Clear();
  dwire_Append(DWIRE_SET_PC_LOW);

  if (Set == xet)
  {
    dwire_Append(1);
    dwire_Append(DWIRE_GO);
    dwire_Append(*pValue);

    result = uart_Write();
  }
  else
  {
    dwire_Append(0);
    dwire_Append(DWIRE_GO);

    *pValue = 0;
    result = uart_WriteRead(1);
    if (FW_TRUE == result)
    {
      *pValue = gDWire.rxBuffer[0];
    }
  }

  return result;
}

/* -------------------------------------------------------------------------- */

FW_BOOLEAN dwire_SRAM(Xet_t xet, U16 address, U8 * pRaw, U16 length)
{
  FW_BOOLEAN result = FW_FALSE;
  U32 i = 0;

  if (REG_NUM_HI >= address)
  {
    return result;
  }

  /* Store Z register */
  if (FW_FALSE == DWire_GetRegs(30, (U8 *)&gDWire.z, 2))
  {
    return FW_FALSE;
  }

  if (FW_FALSE == dwire_Start_SRAM(xet, address))
  {
    return result;
  }

  for (i = 0; i < length; i++)
  {
    if ((address + i) == (gDWire.dwdr + REG_IO_NUM_LO))
    {
      /* Skip get/setting the DWDR register */
      /* Set Z register to the next SRAM address */
      result = dwire_Start_SRAM(xet, (address + i + 1));
      if (FW_FALSE == result) break;
    }
    else
    {
      result = dwire_Next_SRAM(xet, &pRaw[i]);
      if (FW_FALSE == result) break;
    }
  }

  if (FW_TRUE == result)
  {
    /* Retore Z register */
    result = DWire_SetRegs(30, (U8 *)&gDWire.z, 2);
  }

  return result;
}

/* -------------------------------------------------------------------------- */

































/* -------------------------------------------------------------------------- */

void DWire_CacheRegs(void)
{
  //DWire_Send(dwire, BYTES(DWIRE_GET_PC));
  //dwire->pc = (DWire_ReceiveWord(dwire) * 2 - 1) % DWIRE_DEV_FLASH_SIZE;
  dwire_Clear();

  //DWire_GetRegs(dwire, 28, &dwire->regs[28], 4);
}

/* -------------------------------------------------------------------------- */

void DWire_CacheAllRegs(void)
{
  //if (dwire->have_all_regs) return;
  //DWire_GetRegs(dwire, 0, dwire->regs, 28);
  //dwire->have_all_regs = true;
}

/* -------------------------------------------------------------------------- */

void DWire_FlushCacheRegs(void)
{
  //if (dwire->have_all_regs)
  //  DWire_SetRegs(dwire, 0, dwire->regs, 32);
  //else
  //  DWire_SetRegs(dwire, 28, &dwire->regs[28], 4);
  //DWire_SetPC(dwire, dwire->pc/2);
}

/* -------------------------------------------------------------------------- */

void DWire_Reset(void)
{
//  DWire_Send(dwire, BYTES(DWIRE_RESET));
//  DWire_Sync(dwire);
//  DWire_Reconnect(dwire);
}

/* -------------------------------------------------------------------------- */

void DWire_Step(void)
{
//  DWire_FlushCacheRegs(dwire);
//  DWire_Send(dwire, BYTES(DWIRE_FLAG_RUN, DWIRE_RESUME_SS));
//  DWire_Sync(dwire);
//  DWire_Reconnect(dwire);
//  dwire->have_all_regs = false;
}

/* -------------------------------------------------------------------------- */

void DWire_Run(void)
{
//  dwire->pc = 0;
//  dwire->breakpoint = -1;
//  DWire_Continue(dwire);
}

/* -------------------------------------------------------------------------- */

void DWire_Continue(void)
{
//  DWire_FlushCacheRegs(dwire);
//  if (dwire->breakpoint < 0) {
//    DWire_Send(dwire, BYTES(DWIRE_FLAG_RUN));
//  } else {
//    DWire_SetBP(dwire, dwire->breakpoint / 2);
//    DWire_Send(dwire, BYTES(DWIRE_FLAG_RUN_TO_CURSOR));
//  }
//  DWire_Send(dwire, BYTES(DWIRE_RESUME));
//  dwire->stopped = false;
//  dwire->have_all_regs = false;
}

/* -------------------------------------------------------------------------- */

void DWire_ReadFlash(U16 address, U8 * pBuffer, U16 count)
{
//  DWire_SetZ(dwire, addr);
//  DWire_SetBP(dwire, count * 2);
//  DWire_SetPC(dwire, 0);
//  DWire_Send(dwire, BYTES(DWIRE_FLAG_MEMORY, DWIRE_RW_MODE, DWIRE_MODE_READ_FLASH, DWIRE_GO));
//  DWire_Receive(dwire, buf, count);
}

/* -------------------------------------------------------------------------- */

void DWire_WriteFlashPage(U16 address, U8 * pBuffer, U16 count)
{
//  DWire_SetZ(dwire, addr);
//  DWire_SetPC(dwire, 0x3f00);
//  DWire_PreInst(dwire);
////  DWire_Inst(dwire, 0x01cf);  // movw r24,r30
//  DWire_LDI(dwire, 29, 0x03);
//  DWire_Out_SPMCSR(dwire, 29);
//  DWire_SPM(dwire);
//  osDelay(10); // TODO hack can we do without it?
//  DWire_BreakSync(dwire);
//  DWire_PreFlashInst(dwire);
//  DWire_LDI(dwire, 29, 0x01);
//  for (int i = 0; i < count; i += 2) {
//    DWire_SetPC(dwire, 0x3f00);
//    DWire_In_DWDR(dwire, 0);
//    DWire_SendByte(dwire, buf[i]);
//    DWire_In_DWDR(dwire, 1);
//    DWire_SendByte(dwire, buf[i+1]);
//    DWire_Out_SPMCSR(dwire, 29);
//    DWire_SPM(dwire);
//    DWire_Inst(dwire, 0x9632);
//  }
//  DWire_SetPC(dwire, 0x3f00);
//  DWire_LDI(dwire, 31, (addr >> 8) & 0xff);
//  DWire_LDI(dwire, 30, addr & 0xff);
//  DWire_LDI(dwire, 29, 0x05);
//  DWire_Out_SPMCSR(dwire, 29);
//  DWire_SPM(dwire);
//  osDelay(10); // TODO hack can we do without it?
//  DWire_BreakSync(dwire);
//  DWire_SetPC(dwire, 0x3f00);
//  DWire_LDI(dwire, 29, 0x11);
//  DWire_Out_SPMCSR(dwire, 29);
//  DWire_SPM(dwire);
//  DWire_BreakSync(dwire);
//  // TODO: restore regs r0/r1
}

/* -------------------------------------------------------------------------- */

U8 DWire_ReadFuseBits(U16 z)
{
//  DWire_SetZ(dwire, z);
//  //DWire_SetRegs(dwire, 29, BYTES(0x09, WORD_LE(z)));
//  //DWire_SetPC(dwire, 0x3f00);
//  DWire_PreInst(dwire);
//  DWire_LDI(dwire, 29, 0x09);
//  DWire_Out_SPMCSR(dwire, 29);
//  DWire_LPM(dwire, 29);
//  DWire_Out_DWDR(dwire, 29);
//  return DWire_ReceiveByte(dwire);
  return 0;
}

/* -------------------------------------------------------------------------- */

U8 DWire_ReadFuseBitsLow(void)
{
  return 0; //DWire_ReadFuseBits(dwire, 0);
}

/* -------------------------------------------------------------------------- */

U8 DWire_ReadFuseBitsHigh(void)
{
  return 0; //DWire_ReadFuseBits(dwire, 3);
}

/* -------------------------------------------------------------------------- */

U8 DWire_ReadFuseBitsExt(void)
{
  return 0; //DWire_ReadFuseBits(dwire, 2);
}

/* -------------------------------------------------------------------------- */

U8 DWire_ReadLockBits(void)
{
  return 0; //DWire_ReadFuseBits(dwire, 1);
}

/* -------------------------------------------------------------------------- */










/* -------------------------------------------------------------------------- */

//static void uart_Open(void)
//{
//  /* UART3: PB10 - Tx, PB11 - Rx */
//  GPIO_Init(UART3_TX_PORT, UART3_TX_PIN, GPIO_TYPE_IN_PUP_PDN, 1);
//  GPIO_Init(UART3_RX_PORT, UART3_RX_PIN, GPIO_TYPE_IN_PUP_PDN, 1);
//
//  //UART_DeInit(UART2);
//  UART_Init
//  (
//    UART2,
//    86400,
//    uart_RxByte,
//    uart_RxComplete,
//    uart_TxByte,
//    uart_TxComplete
//  );
//
//  /* UART3: PB10 - Tx, PB11 - Rx */
//  GPIO_Init(UART3_TX_PORT, UART3_TX_PIN, GPIO_TYPE_ALT_PP_10MHZ, 1);
//  GPIO_Init(UART3_RX_PORT, UART3_RX_PIN, GPIO_TYPE_IN_PUP_PDN,   1);
//
//  UART_RxStart(UART3);
//}

/* -------------------------------------------------------------------------- */

void dwire_Task(void * pvParameters)
{
  DWIRE_LOG("Debug Wire Task Started...\r\n");

//  (void)dwire_Sync();

//  UART_Init
//  (
//    UART2,
//    86400,
//    uart_RxByte,
//    uart_RxComplete,
//    uart_TxByte,
//    uart_TxComplete
//  );
//
//  USART2->CR3 |= USART_CR3_HDSEL;
//
//  GPIO_Init(UART2_RTX_PORT, UART2_RTX_PIN, GPIO_TYPE_OUT_OD_2MHZ, 0);
//  //GPIO_Lo(UART2_RTX_PORT, UART2_RTX_PIN);
//  vTaskDelay(50);
//  //GPIO_Hi(UART2_RTX_PORT, UART2_RTX_PIN);
//  //vTaskDelay(2);
//
//  GPIO_Init(UART2_RTX_PORT, UART2_RTX_PIN, GPIO_TYPE_ALT_OD_2MHZ, 1);
//
//  (void)uart_Read(gRxBuffer, 1);
//  DBG("DWire: Received: L = %d, B[0] = %02X\r\n", gRxLen, gRxBuffer[0]);
//
//  vTaskDelay(50);
//
//  gTxBuffer[0] = 0x07;
//  gTxLen = 1;
//  DBG("DWire: Transmit: L = %d, B[0] = %02X\r\n", gTxLen, gTxBuffer[0]);
//  uart_Write(gTxBuffer, 1);
//
//  (void)uart_Read(gRxBuffer, 1);
//  DBG("DWire: Received: L = %d, B[0] = %02X\r\n", gRxLen, gRxBuffer[0]);

  while(1)
  {
    vTaskDelay(500);
  }
}

