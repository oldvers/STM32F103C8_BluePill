#include <stdio.h>
#include <string.h>

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

static U16 gPC = 0;

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_Sync(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U32 i = 0;

  DBG("*** dWire Test Sync ***\r\n");

  DWire_Init();
  /* ATmega48: DWDR = 0x31, SPMCSR = 0x37, BasePC = 0x1800 */
  DWire_SetParams(0x31, 0x37, 0x1800);
  /* ATmega48: rSize = 512, fSize = 4096, fPage = 64, eSize = 256 */
  DWire_SetMemParams(512, 4096, 64, 56);

  DBG(" - Sync\r\n");
  result = DWire_Sync();
  if (FW_FALSE == result) return FW_FALSE;

  DBG(" - Break\r\n");
  DWire_Break();

  DBG(" - Wait for Break event\r\n");
  for (i = 0; i < 100; i++)
  {
    result = DWire_CheckForBreak();
    if (FW_TRUE == result) break;
    vTaskDelay(5);
  }
  if (FW_TRUE == result)
  {
    DBG(" - Break received after %d ms\r\n", (i + 1) * 5);

    gPC = DWire_GetPC();
    DBG(" - PC = 0x%04X\r\n", gPC);
  }

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

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_ReadIoRegs(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U8 raw[64] = {0};

  DBG("*** dWire Test Read IO Registers ***\r\n");

  result = DWire_GetSRAM(0x0020, raw, sizeof(raw));

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_WriteIoRegs(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U8 sRaw[64] = {0}, rRaw[64] = {0};
  U16 sZ = 0, rZ = 0;

  DBG("*** dWire Test Write IO Registers ***\r\n");

  /* Set Z register to some magic value */
  sZ = 0x1313;
  DBG("--- Set Z to %04X\r\n", sZ);
  result = DWire_SetRegs(30, (U8 *)&sZ, 2);
  if (FW_FALSE == result) return FW_FALSE;

  result = DWire_GetRegs(30, (U8 *)&rZ, 2);
  if ((FW_FALSE == result) || (rZ != sZ)) return FW_FALSE;

  /* Read the I/O registers */
  DBG("--- Read I/O registers\r\n");
  result = DWire_GetSRAM(0x0020, sRaw, sizeof(sRaw));
  if (FW_FALSE == result) return FW_FALSE;

  /* Set the PORTC bit 0 to turn  the LED on. Write the I/O registers */
  DBG("--- Write I/O registers\r\n");
  sRaw[8] |= 1;
  result = DWire_SetSRAM(0x0028, &sRaw[8], 1);
  if (FW_FALSE == result) return FW_FALSE;

  /* Read back the I/O registers and compare */
  DBG("--- Read I/O registers\r\n");
  result = DWire_GetSRAM(0x0020, rRaw, sizeof(rRaw));
  if (FW_FALSE == result) return FW_FALSE;
  if (rRaw[8] != sRaw[8]) return FW_FALSE;

  /* Get Z register to check if it was restored correctly  */
  result = DWire_GetRegs(30, (U8 *)&rZ, 2);
  if (FW_FALSE == result) return FW_FALSE;

  DBG("--- Get Z = %04X\r\n", rZ);
  result = (FW_BOOLEAN)(rZ == sZ);

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_ReadFlash(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U8 raw[64] = {0};

  DBG("*** dWire Test Read Flash ***\r\n");

  result = DWire_GetFlash(0x0000, raw, sizeof(raw));

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_WriteFlash(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U8 raw[64] = {0}, regs[32] = {0};
  //U16 sZ = 0, rZ = 0;
  U16 i = 0;

  DBG("*** dWire Test Write Flash ***\r\n");

  /* Set registers to some magic value */
  for (i = 0; i < sizeof(regs); i++) regs[i] = 0x13;
  DBG("--- Set registers to %02X\r\n", regs[0]);
  result = DWire_SetRegs(0, regs, sizeof(regs));
  if (FW_FALSE == result) return FW_FALSE;

  /* Write the Flash */
  DBG("--- Write the Flash\r\n");
  for (i = 0; i < sizeof(raw); i++) raw[i] = (sizeof(raw) - i - 1);
  result = DWire_SetFlash(0x0080, raw, sizeof(raw));
  if (FW_FALSE == result) return FW_FALSE;

  /* Read back the registers and compare */
  DBG("--- Read the registers\r\n");
  result = DWire_GetRegs(0, raw, sizeof(regs));
  if (FW_FALSE == result) return FW_FALSE;
  if (0 != memcmp(raw, regs, sizeof(regs))) return FW_FALSE;

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_WriteBlinkProgramToFlash(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U8 blinky[64] =
  {
    0x0F, 0xEF, 0x0D, 0xBF, 0x02, 0xE0, 0x0E, 0xBF,
    0x01, 0xE0, 0x07, 0xB9, 0x40, 0x9A, 0x07, 0xD0,
    0x06, 0xD0, 0x05, 0xD0, 0x40, 0x98, 0x03, 0xD0,
    0x02, 0xD0, 0x01, 0xD0, 0xF7, 0xCF, 0xEE, 0x27,
    0xFF, 0x27, 0x31, 0x97, 0xF1, 0xF7, 0x08, 0x95,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  };

  DBG("*** dWire Test Write Blink Program to Flash ***\r\n");

  /* Write the Flash */
  DBG("--- Write the Flash\r\n");
  result = DWire_SetFlash(0x0000, blinky, sizeof(blinky));
  if (FW_FALSE == result) return FW_FALSE;

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_Reset(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U32 i = 0;

  DBG("*** dWire Test Reset ***\r\n");

  DBG(" - Send the Reset command\r\n");
  result = DWire_Reset();
  if (FW_FALSE == result) return FW_FALSE;

  DBG(" - Wait for Break event\r\n");
  for (i = 0; i < 100; i++)
  {
    vTaskDelay(5);
    result = DWire_CheckForBreak();
    if (FW_TRUE == result) break;
  }
  if (FW_TRUE == result)
  {
    DBG(" - Break received after %d ms\r\n", (i + 1) * 5);

    gPC = DWire_GetPC();
    DBG(" - PC = 0x%04X\r\n", gPC);

    result = (FW_BOOLEAN)(0 == gPC);
  }

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_Run(void)
{
  FW_BOOLEAN result = FW_TRUE;

  DBG("*** dWire Test Run ***\r\n");

  result = DWire_Run();

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_Stop(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U32 i = 0, t = 0;

  DBG("*** dWire Test Stop ***\r\n");

  /* Let the blinky program run about 5 seconds */
  vTaskDelay(5000);

  /* Stop the program */
  DBG("--- Stop the program\r\n");
  t = xTaskGetTickCount();
  DWire_Break();
  for (i = 0; i < 100; i++)
  {
    vTaskDelay(5);
    result = DWire_CheckForBreak();
    if (FW_TRUE == result) break;
  }
  if (FW_FALSE == result) return FW_FALSE;
  DBG(" - Stopped after %d ms\r\n", (xTaskGetTickCount() - t));

  /* Read the PC */
  gPC = DWire_GetPC();
  DBG(" - PC = 0x%04X\r\n", gPC);

//  /* Run to the address before the LED on instruction */
//  DBG("--- Run to cursor\r\n");
//  result = DWire_RunToCursor(16, 10000);
//  if (FW_FALSE == result) return FW_FALSE;
//
//  /* Read the PC */
//  pc = DWire_GetPC();
//  DBG(" -  PC = 0x%04X\r\n", pc);
//
//  vTaskDelay(2000);
//
//  /* Make the step to for LED on */
//  DBG("--- Do the step\r\n");
//  result = DWire_Step();
//  if (FW_FALSE == result) return FW_FALSE;
//
//  /* Read the PC */
//  pc = DWire_GetPC();
//  DBG(" -  PC = 0x%04X\r\n", pc);
//
//  vTaskDelay(2000);
//
//  /* Run the program */
//  DBG("--- Run the program\r\n");
//  result = DWire_Run();

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_StepInto(void)
{
  FW_BOOLEAN result = FW_TRUE;
  U32 i = 0, time = 0, step = 0;

  time = xTaskGetTickCount();
  for (step = 0; step < 8; step++)
  {
    DBG("--- Do the step %d into\r\n", step);
    result = DWire_StepInto();
    if (FW_FALSE == result) return FW_FALSE;

    for (i = 0; i < 100; i++)
    {
      result = DWire_CheckForBreak();
      if (FW_TRUE == result) break;
      vTaskDelay(1);
    }
    if (FW_FALSE == result) return FW_FALSE;
    DBG(" - Stopped after %d ms\r\n", (xTaskGetTickCount() - time));

    gPC = DWire_GetPC();
    DBG(" - PC = 0x%04X\r\n", gPC);
  }

  result &= (FW_BOOLEAN)(0x000F == gPC);

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_StepOver(void)
{
  FW_BOOLEAN result = FW_TRUE;

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_StepOut(void)
{
  FW_BOOLEAN result = FW_TRUE;

  return result;
}

/* -------------------------------------------------------------------------- */

static FW_BOOLEAN Test_RunToCursor(void)
{
  FW_BOOLEAN result = FW_TRUE;

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
  Test_ReadIoRegs,
  Test_WriteIoRegs,
  Test_ReadFlash,
  Test_WriteFlash,
  Test_WriteBlinkProgramToFlash,
  Test_Reset,
  Test_Run,
  Test_Stop,
  Test_Reset,
  Test_StepInto,
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
