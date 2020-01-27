#include <string.h>
#include <stdio.h>
#include "types.h"
#include "debug.h"
#include "stm32f1xx.h"
#include "SEGGER_RTT.h"

/* ITM Stimulus Ports */
#define CPU_ITM_O_STIMPORT_00               (0x00000000)

/* ITM Trace Enable Register */
#define CPU_ITM_O_TER                       (0x00000E00)
#define CPU_ITM_TER_STIMENA_00              (0x00000001)

/* ITM Trace Privilege Register */
#define CPU_ITM_O_TPR                       (0x00000E40)
#define CPU_ITM_TPR_PORTS_00_07             (0x00000001)
#define CPU_ITM_TPR_PORTS_08_15             (0x00000002)
#define CPU_ITM_TPR_PORTS_16_23             (0x00000004)
#define CPU_ITM_TPR_PORTS_24_31             (0x00000008)

/* ITM Trace Control Register */
#define CPU_ITM_O_TCR                       (0x00000E80)
#define CPU_ITM_TCR_ATBID                   (0x7F << 16)
#define CPU_ITM_TCR_SWOENA                  (1 << 4)
#define CPU_ITM_TCR_SYNCENA                 (1 << 2)
#define CPU_ITM_TCR_ITMENA                  (1 << 0)

/* ITM Lock Access Register */
#define CPU_ITM_O_LAR                       (0x00000FB0)
#define CPU_ITM_LAR_KEY                     (0xC5ACCE55)

/* DWT Control Register */
#define CPU_DWT_O_CR                        (0x00000000)
#define CPU_DWT_CR_NUMCOMP                  (0x40000000)
#define CPU_DWT_CR_POSTPRESET               (0x0000001E)
#define CPU_DWT_CR_POSTCNT                  (0x000001E0)
#define CPU_DWT_CR_CYCTAP                   (0x00000200)

/* Async Clock Prescaler Register */
#define CPU_TPIU_O_ACPR                     (0x00000010)

/* Selected Pin Protocol Register */
#define CPU_TPIU_O_SPPR                     (0x000000F0)
#define CPU_TPIU_SPPR_PROTOCOL_TRACE_PORT   (0x00000000)
#define CPU_TPIU_SPPR_PROTOCOL_MANCHESTER   (0x00000001)
#define CPU_TPIU_SPPR_PROTOCOL_NRZ          (0x00000002)

/* Formatter and Flush Control Register */
#define CPU_TPIU_O_FFCR                     (0x00000304)
#define CPU_TPIU_FFCR_TRIG_IN               (0x00000100)

/*----------------------------------------------------------------------------*/

/* Initialize the SWO trace port for debug message printing */
static void SWO_Init( void )
{
  /* Default 8M baud rate */
  /* SWO Speed in Hz */
  /* Note that freq is expected to be match the CPU core clock */
  U32 SWOPrescaler = (SystemCoreClock / 8000000) - 1;

  /* Enable trace in core debug */
  //*((volatile unsigned *)(SCS_BASE + CPU_SCS_O_DEMCR)) |=
  //    CPU_SCS_DEMCR_TRCENA;
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

  /* "Selected PIN Protocol Register": Select which protocol to use for */
  /* trace output (2: SWO NRZ, 1: SWO Manchester encoding) */
  //*((volatile unsigned *)(CPU_TPIU_BASE + CPU_TPIU_O_SPPR)) =
  //    CPU_TPIU_SPPR_PROTOCOL_NRZ;
  TPI->SPPR = CPU_TPIU_SPPR_PROTOCOL_NRZ;

  /* "Async Clock Prescaler Register". */
  /* Scale the baud rate of the asynchronous output */
  //*((volatile unsigned *)(CPU_TPIU_BASE + CPU_TPIU_O_ACPR)) = SWOPrescaler;
  TPI->ACPR = SWOPrescaler;


  /* ITM Lock Access Register, C5ACCE55 enables more write access to */
  /* Control Register 0xE00 :: 0xFFC */
  //*((volatile unsigned *)(CPU_ITM_BASE + CPU_ITM_O_LAR)) = CPU_ITM_LAR_KEY;
  ITM->LAR = CPU_ITM_LAR_KEY;

  /* ITM Trace Control Register */
//    *((volatile unsigned *)(CPU_ITM_BASE + CPU_ITM_O_TCR)) =
//        CPU_ITM_TCR_ATBID | CPU_ITM_TCR_SWOENA | CPU_ITM_TCR_SYNCENA |
//        CPU_ITM_TCR_ITMENA;
  ITM->TCR = (ITM_TCR_TraceBusID_Msk | ITM_TCR_SWOENA_Msk |
              ITM_TCR_SYNCENA_Msk    | ITM_TCR_ITMENA_Msk);

  /* ITM Trace Privilege Register */
//    *((volatile unsigned *)(CPU_ITM_BASE + CPU_ITM_O_TPR)) =
//        CPU_ITM_TPR_PORTS_00_07;
  ITM->TPR = CPU_ITM_TPR_PORTS_00_07;

  /* ITM Trace Enable Register. Enabled tracing on stimulus ports. */
  /* One bit per stimulus port. */
//    *((volatile unsigned *)(CPU_ITM_BASE + CPU_ITM_O_TER)) =
//        CPU_ITM_TER_STIMENA_00;
  ITM->TER = CPU_ITM_TER_STIMENA_00;

  /* DWT_CTRL */
//    *((volatile unsigned *)(CPU_DWT_BASE + CPU_DWT_O_CR)) =
//        CPU_DWT_CR_NUMCOMP | CPU_DWT_CR_POSTPRESET |
//        CPU_DWT_CR_POSTCNT | CPU_DWT_CR_CYCTAP;
  DWT->CTRL = (CPU_DWT_CR_NUMCOMP | CPU_DWT_CR_POSTPRESET |
               CPU_DWT_CR_POSTCNT | CPU_DWT_CR_CYCTAP);

  /* Formatter and Flush Control Register */
//    *((volatile unsigned *)(CPU_TPIU_BASE + CPU_TPIU_O_FFCR)) =
//        CPU_TPIU_FFCR_TRIG_IN;
  TPI->FFCR = TPI_FFCR_TrigIn_Msk;
}

void Debug_Init(void)
{
#ifdef ENABLE_DEBUG
  SWO_Init();
#endif
}

/*----------------------------------------------------------------------------*/

#if defined(__ARMCC_VERSION)
#include <rt_misc.h>

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

int fputc(int c, FILE *f)
{
  /* PB3 (JTDO/TRACESWO) is used for debug output */
  ITM_SendChar(c);
  return 0;
}
#endif

#if defined(__ICCARM__) && !defined(SIMULATOR)
size_t __write(int handle, const unsigned char * buffer, size_t size)
{
  (void) handle;  /* Not used, avoid warning */
  SEGGER_RTT_Write(0, (const char*)buffer, size);
  return size;
}
#endif
