#include "types.h"
#include "stm32f1xx.h"
#include "system_stm32f1xx.h"
#include "system.h"

#define SYSTEM_STARTUP_TIMEOUT     (20000U)
#define RCC_CFGR_PLLSRC_HSI        (0U << RCC_CFGR_PLLSRC_Pos)
#define RCC_CFGR_PLLSRC_HSE        (1U << RCC_CFGR_PLLSRC_Pos)

static void SystemClockConfig( void );

void ApplicationInit( void )
{
  /* Setup interrupts priority grouping */
  NVIC_SetPriorityGrouping(5);
  
  /* First of all - Init the system */
  SystemInit();
  
  /* Initialize system clock */
  SystemClockConfig();
}

/* ---------------------------------------------------------------------------------------------- */

/* Configuration of System clock frequency, AHB/APBx prescalers and Flash settings.
    - System Clock source - PLL(HSE)
    - SYSCLK              - 72000000 Hz
    - HCLK                - 72000000 Hz
    - AHB Prescaler       - 1
    - APB1 Prescaler      - 1
    - APB2 Prescaler      - 1
    - HSE Frequency       - 8000000 Hz
    - PLL MUL             - 9
    - VDD                 - 3.3 V
    - Flash Latency       - 1 WS                                                                  */

void SystemClockConfig( void )
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
    FLASH->ACR |= (U32)FLASH_ACR_LATENCY_2;    
 
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
}
