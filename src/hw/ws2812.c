#include "types.h"
#include "hardware.h"
#include "gpio.h"

static U8 * gLedBuffer;
static U32  gLedBufferSize;

void WS2812_Init(U8 * pLedBuffer, U32 aLedBufferSize)
{
    gLedBuffer = pLedBuffer;
    gLedBufferSize = aLedBufferSize;

//  U32 i;

//  for (i = 0; i < LED_COUNT * 3 * 8; i++) LEDBUF[i] = 12;
//  for (i = LED_COUNT * 3 * 8; i < (LED_COUNT * 3 * 8 + 6); i++) LEDBUF[i] = 0;
//  WS2812_Convert();

  GPIO_Init(WS2812_PORT, WS2812_PIN, GPIO_TYPE_ALT_PP_50MHZ);

  /* NVIC */
  //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  //NVIC_SetPriorityGrouping(5);
  NVIC_SetPriority(IRQ_NUMBER_WS2812_TIM_DMA, IRQ_PRIORITY_WS2812_TIM_DMA);
  NVIC_EnableIRQ(IRQ_NUMBER_WS2812_TIM_DMA);

  /* DMA */
  RCC->WS2812_TIM_DMA_CLK_R |= WS2812_TIM_DMA_CLK_E;
  WS2812_TIM_DMA->IFCR |= ( WS2812_TIM_DMA_FTC | WS2812_TIM_DMA_FTE |
                            WS2812_TIM_DMA_FHT | WS2812_TIM_DMA_FGL );
  WS2812_TIM_DMA_CHANNEL->CPAR  = (U32)&WS2812_TIM->WS2812_TIM_CCR;
  WS2812_TIM_DMA_CHANNEL->CMAR  = (U32)gLedBuffer; //&LEDBUF[0];
  WS2812_TIM_DMA_CHANNEL->CCR |= ( DMA_CCR_PSIZE_0 | DMA_CCR_MINC |
                                   DMA_CCR_DIR | DMA_CCR_TCIE );

  /* TIM - 72 MHz (Lo - 12 (0.33 us), Hi - 28 (0.77 us)) */
  DBGMCU->CR |= WS2812_TIM_DBGMCU;
  RCC->WS2812_TIM_CLK_R |= WS2812_TIM_CLK_E;
  WS2812_TIM->PSC = 2 - 1;
  WS2812_TIM->ARR = 45 - 1;
  WS2812_TIM->CNT = 30;
  //Initialize TIM CH4 (Output, PWM Mode 1, Inverted, 10.67 us)
  WS2812_TIM->CCER &= ~(TIM_CCER_CC4E | TIM_CCER_CC4P);
  WS2812_TIM->CCMR2 &= ~(TIM_CCMR2_OC4CE | TIM_CCMR2_OC4M | TIM_CCMR2_OC4PE | TIM_CCMR2_OC4FE | TIM_CCMR2_CC4S);
  WS2812_TIM->CCMR2 |= TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4PE;
  WS2812_TIM->WS2812_TIM_CCR = 0;
  WS2812_TIM->CCER |= TIM_CCER_CC4E;
  //Enable TIM
  WS2812_TIM->DIER |= TIM_DIER_UDE;
}

//void WS2812_Start(void)
//{
//  WS2812_TIM_DMA_CHANNEL->CNDTR = LED_COUNT * 3 * 8 + 6;
//  WS2812_TIM_DMA_CHANNEL->CCR |= DMA_CCR_EN;
//  WS2812_TIM->CNT = 30;
//  WS2812_TIM->CR1 |= TIM_CR1_CEN;
//}

//void DMA1_Channel7_IRQHandler(void)
//{
//  WS2812_TIM_DMA->IFCR |= (DMA_ISR_GIF7 | DMA_ISR_TCIF7 | DMA_ISR_HTIF7 | DMA_ISR_TEIF7);
//  WS2812_TIM_DMA_CHANNEL->CCR &= ~(DMA_CCR_EN);
//  WS2812_TIM->CR1 &= ~(TIM_CR1_CEN);
//}
