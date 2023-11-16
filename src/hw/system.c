#include "types.h"
#include "system.h"
#include "interrupts.h"

#define SYSTEM_STARTUP_TIMEOUT     (20000U)
#define RCC_CFGR_PLLSRC_HSI        (0U << RCC_CFGR_PLLSRC_Pos)
#define RCC_CFGR_PLLSRC_HSE        (1U << RCC_CFGR_PLLSRC_Pos)

static void SYS_ClockConfig( void );

/* -------------------------------------------------------------------------- */

__no_init U32 CPUClock;
__no_init U32 AHBClock;
__no_init U32 APB1Clock;
__no_init U32 APB2Clock;

static const U16 AHBDiv[16] =
{
  1,   1,   1,   1,   1,   1,   1,   1,
  2,   4,   8,  16,  64, 128, 256, 512,
};
static const U8 APBDiv[8] =
{
  1,   1,   1,   1,   2,   4,   8,  16,
};

/* -------------------------------------------------------------------------- */

void SYS_ApplicationInit( void )
{
  /* Setup interrupts priority grouping */
  IRQ_SetPriorityGrouping();

  /* First of all - Init the system */
  SystemInit();

  /* Initialize system clock */
  SYS_ClockConfig();
}

/* -------------------------------------------------------------------------- */

/* Configuration of System clock frequency, AHB/APBx prescalers and Flash WS
    - System Clock source - PLL(HSE)
    - SYSCLK              - 72000000 Hz
    - HCLK                - 72000000 Hz
    - AHB Prescaler       - 1
    - APB1 Prescaler      - 2
    - APB2 Prescaler      - 1
    - HSE Frequency       - 8000000 Hz
    - PLL MUL             - 9
    - VDD                 - 3.3 V
    - Flash Latency       - 2 WS                                              */

void SYS_ClockConfig( void )
{
  volatile U32 StartUpCounter = 0, HSEStatus = 0;

  /* Enable HSE */
  RCC->CR |= ((U32)RCC_CR_HSEON);

  /* Wait till HSE is ready and if Time out is reached exit */
  do
  {
    HSEStatus = RCC->CR & RCC_CR_HSERDY;
    StartUpCounter++;
  }
  while ((HSEStatus == 0) && (StartUpCounter != SYSTEM_STARTUP_TIMEOUT));

  if (0 != (RCC->CR & RCC_CR_HSERDY))
  {
    HSEStatus = 1U;
  }
  else
  {
    HSEStatus = 0U;
  }

  if (1U == HSEStatus)
  {
    /* Enable Prefetch Buffer */
    FLASH->ACR |= FLASH_ACR_PRFTBE;

    /* Flash 2 wait state */
    FLASH->ACR &= (U32)((U32)~FLASH_ACR_LATENCY);
    FLASH->ACR |= (U32)FLASH_ACR_LATENCY_1;

    /* HCLK = SYSCLK */
    RCC->CFGR |= (U32)RCC_CFGR_HPRE_DIV1;

    /* PCLK2 = HCLK */
    RCC->CFGR |= (U32)RCC_CFGR_PPRE2_DIV1;

    /* PCLK1 = HCLK */
    RCC->CFGR |= (U32)RCC_CFGR_PPRE1_DIV2;

    /*  PLL configuration: PLLCLK = HSE * 9 = 72 MHz */
    RCC->CFGR &= (U32)((U32)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE |
                              RCC_CFGR_PLLMULL));
    RCC->CFGR |= (U32)(RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMULL9);

    /* Enable PLL */
    RCC->CR |= RCC_CR_PLLON;

    /* Wait till PLL is ready */
    while (0U == (RCC->CR & RCC_CR_PLLRDY))
    {
        //
    }

    /* Select PLL as system clock source */
    RCC->CFGR &= (U32)((U32)~(RCC_CFGR_SW));
    RCC->CFGR |= (U32)RCC_CFGR_SW_PLL;

    /* Wait till PLL is used as system clock source */
    while (RCC_CFGR_SWS_PLL != (RCC->CFGR & (U32)RCC_CFGR_SWS))
    {
        //
    }
  }
  else
  {
    /* If HSE fails to start-up, the application will have wrong clock
    configuration. User can add here some code to deal with this error */
  }

  SystemCoreClockUpdate();

  CPUClock  = SystemCoreClock;
  AHBClock  = AHBDiv[(RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos];
  AHBClock  = CPUClock / AHBClock;
  APB1Clock = APBDiv[(RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos];
  APB1Clock = CPUClock / APB1Clock;
  APB2Clock = APBDiv[(RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos];
  APB2Clock = CPUClock / APB2Clock;
}

/* -------------------------------------------------------------------------- */
/** @brief Sets the APB2 prescaler
 *  @param value - Prescaler value
 *  @return True - in case of success
 */
FW_BOOLEAN SYS_SetAPB2Prescaler(U8 value)
{
  FW_BOOLEAN result = FW_FALSE;
  U32 prescaler = 0;

  if ((0 < value) && (16 >= value) && (0 == (value & (value - 1))))
  {
    prescaler = (34 - __CLZ(value));

    RCC->CFGR &= (U32)~(RCC_CFGR_PPRE2);
    RCC->CFGR |= ((prescaler << RCC_CFGR_PPRE2_Pos) & RCC_CFGR_PPRE2_Msk);

    APB2Clock = APBDiv[(RCC->CFGR & RCC_CFGR_PPRE2) >> RCC_CFGR_PPRE2_Pos];
    APB2Clock = CPUClock / APB2Clock;

    result = FW_TRUE;
  }

  return result;
}

/* -------------------------------------------------------------------------- */