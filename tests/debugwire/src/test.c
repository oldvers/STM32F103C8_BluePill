#include <stdio.h>

#include "types.h"
#include "debug.h"
#include "board.h"
#include "gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "dwire_processor.h"

typedef FW_BOOLEAN (* TestFunction_t)(void);

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_Sync(void)
{
  FW_BOOLEAN result = FW_TRUE;

  DBG("*** dWire Test Sync ***\r\n");

  DWire_Init();
  /* ATmega48: DWDR = 0x31, SPMCSR = 0x37, BasePC = 0x1800 */
  DWire_SetParams(0x31, 0x37, 0x1800);
  /* ATmega48: rSize = 512, fSize = 4096, fPage = 64, eSize = 256 */
  DWire_SetMemParams(512, 4096, 64, 56);

  result = DWire_Sync();

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_ReadSignature(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U16 sign = 0;

  DBG("*** dWire Test Read Signature ***\r\n");

  sign = DWire_GetSignature();

  result = (FW_BOOLEAN)(0 != sign);

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_ReadPc(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U16 sign = 0;

  DBG("*** dWire Test Read PC ***\r\n");

  sign = DWire_GetPC();

  result = (FW_BOOLEAN)(0 != sign);

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_ReadReg(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U8 value = 0;

  DBG("*** dWire Test Read Reg ***\r\n");

  /* Read PORTC */
  value = DWire_GetReg(0x08);
  DBG(" - Value = %02X\r\n", value);

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_WriteReg(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U8 value = 0;

  DBG("*** dWire Test Write Reg ***\r\n");

  /* Write 1 to PORTC */
  result = DWire_SetReg(0x08, 0x01);
  /* Read PORTC */
  if (FW_TRUE == result)
  {
    value = DWire_GetReg(0x08);
    result = (FW_BOOLEAN)(0x01 == value);
    DBG(" - Value = %02X\r\n", value);
  }

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_WriteRegSequence(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U8 value = 0;
  U8 i = 0;

  DBG("*** dWire Test Write Reg Sequence ***\r\n");

  do
  {
    /* Write 1 to DDRC - set PC0..PC2 as output */
    result = DWire_SetIOReg(0x27, 0x07);
    if (FW_FALSE == result) break;

    /* Read DDRC */
    value = DWire_GetIOReg(0x27);
    result = (FW_BOOLEAN)(0x07 == value);
    if (FW_FALSE == result) break;

    /* Write 1 to PORTC - set PC0..PC2 to 1 - LED off */
    result = DWire_SetIOReg(0x28, 0x00);
    if (FW_FALSE == result) break;

    /* Read PORTC */
    value = DWire_GetIOReg(0x28);
    result = (FW_BOOLEAN)(0x00 == value);
    if (FW_FALSE == result) break;

    /* Blink LED */
    value = 0x00;
    for (i = 0; i < 10; i++)
    {
      /* Write X to PORTC - set PCx to X - LED on/off */
      result = DWire_SetIOReg(0x28, value);
      if (FW_FALSE == result) break;
      value ^= 0x01;
      vTaskDelay(500);
    }

    value = 0x00;
    for (i = 0; i < 10; i++)
    {
      /* Write X to PORTC - set PCx to X - LED on/off */
      result = DWire_SetIOReg(0x28, value);
      if (FW_FALSE == result) break;
      value ^= 0x02;
      vTaskDelay(500);
    }

    value = 0x00;
    for (i = 0; i < 9; i++)
    {
      /* Write X to PORTC - set PCx to X - LED on/off */
      result = DWire_SetIOReg(0x28, value);
      if (FW_FALSE == result) break;
      value ^= 0x04;
      vTaskDelay(500);
    }
  }
  while (FW_FALSE);

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_ReadRegs(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U8 raw[32] = {0};

  DBG("*** dWire Test Read Regs ***\r\n");

  result = DWire_GetRegs(0, raw, sizeof(raw));

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_WriteRegs(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U8 raw[32] = {0};
  U8 i = 0;

  DBG("*** dWire Test Write Regs ***\r\n");

  for (i = 0; i < sizeof(raw); i++) raw[i] = 0x37;
  result = DWire_SetRegs(0, raw, sizeof(raw));

  if (FW_TRUE == result)
  {
    result = DWire_GetRegs(0, raw, sizeof(raw));
    for (i = 0; i < sizeof(raw); i++) result &= (FW_BOOLEAN)(raw[i] == 0x37);
  }

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_ReadSram(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U8 raw[32] = {0};

  DBG("*** dWire Test Read SRAM ***\r\n");

  result = DWire_GetSRAM(0x0100, raw, sizeof(raw));

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_WriteSram(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U8 raw[32] = {0};
  U8 i = 0;

  DBG("*** dWire Test Write SRAM ***\r\n");

  for (i = 0; i < sizeof(raw); i++) raw[i] = i;
  result = DWire_SetSRAM(0x0100, raw, sizeof(raw));

  if (FW_TRUE == result)
  {
    result = DWire_GetSRAM(0x0100, raw, sizeof(raw));
    for (i = 0; i < sizeof(raw); i++) result &= (FW_BOOLEAN)(raw[i] == i);
  }

  return result;
}

/* --- Test Start Up Function (mandatory, called before RTOS starts) -------- */

void vTestStartUpFunction(void)
{
  DBG_ClearScreen();
  DBG("*** Start Up Test ***\r\n");
}

/* --- Test Prepare Function (mandatory, called before RTOS starts) --------- */

void vTestPrepareFunction (void)
{
  DBG("*** Prepare Test ***\r\n");
}

/* --- Helper Task Main Function (mandatory) -------------------------------- */

void vTestHelpTaskFunction(void * pvParameters)
{
  //DBG("LED Task Started..\r\n");
  GPIO_Init(LED_PORT, LED_PIN, GPIO_TYPE_OUT_OD_2MHZ, 0);

  while(1)
  {
    GPIO_Lo(LED_PORT, LED_PIN);
    //DBG_SetTextColorGreen();
    //printf("LED ON\r\n");
    vTaskDelay(500);
    GPIO_Hi(LED_PORT, LED_PIN);
    //DBG_SetTextColorRed();
    //printf("LED OFF\r\n");
    vTaskDelay(500);
  }
  //vTaskDelete(NULL);
}

/* --- Test Cases List (mandatory) ------------------------------------------ */

const TestFunction_t gTests[] =
{
  Test_Sync,
  Test_ReadSignature,
  Test_ReadPc,
  Test_ReadReg,
  Test_WriteReg,
  Test_WriteRegSequence,
  Test_ReadRegs,
  Test_WriteRegs,
  Test_ReadSram,
  Test_WriteSram,
};

U32 uiTestsGetCount(void)
{
  return (sizeof(gTests) / sizeof(TestFunction_t));
}

/* --- Error callback function (mandatory) ---------------------------------- */

void on_error(S32 parameter)
{
  while (FW_TRUE) {};
}

/* -------------------------------------------------------------------------- */
