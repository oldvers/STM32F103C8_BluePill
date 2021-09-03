#include <stdio.h>

#include "stm32f1xx.h"
#include "types.h"
#include "gpio.h"
#include "uniquedevid.h"
#include "system.h"
#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"






#define BITBAND_SRAM_REF   0x20000000
#define BITBAND_SRAM_BASE  0x22000000
// Convert SRAM address
#define BITBAND_SRAM(a,b)  ((BITBAND_SRAM_BASE + (((U32)a)-BITBAND_SRAM_REF)*32 + (b*4)))

#define BITBAND_PERI_REF   0x40000000
#define BITBAND_PERI_BASE  0x42000000
// Convert PERI address
#define BITBAND_PERIPH(reg,bit)  ((BITBAND_PERI_BASE + (((U32)reg)-BITBAND_PERI_REF)*32 + (bit*4)))
#define BITBAND_REG(reg,bit)     (*((volatile U32 *)(BITBAND_PERIPH(reg,bit))))

#define LED_COUNT 64
U8 LED[LED_COUNT * 3] = {50, 30, 34, 56, 67, 58, 99, 12, 4};
U8 LEDBUF[LED_COUNT * 3 * 8 + 6];
U32 LEDCNT;

void WS2812_Convert(void)
{
  U32 i;
  U32 * pLed, * pPwmBit;

  pLed = (U32*)BITBAND_SRAM(&LED[0], 0);
  pPwmBit = (U32*)BITBAND_SRAM(&LEDBUF[0], 4);
  for (i = 0; i < LED_COUNT * 3 * 8; i++)
  {
    *pPwmBit = *pLed;
    pLed += 1;
    pPwmBit += 8;
  }
}

/* RGB (00000000 RRRRRRRR GGGGGGGG BBBBBBBB) */
void WS2812_SetColor(U16 aLed, U8 aR, U8 aG, U8 aB)
{
  U32 pos = aLed * 3;
  U32 * pLed = (U32*)BITBAND_SRAM(&LED[pos], 0);
  //U32 * pPwmBit = (U32*)BITBAND_SRAM(&LEDBUF[pos << 3], 4);
  U32 * pPwmBit = (U32*)BITBAND_SRAM(&LEDBUF[(pos + 3) << 3], 4);
  pPwmBit -= 8;
  U32 i;

  if (LED_COUNT <= aLed) return;
  //LED[pos++] = aG;
  //LED[pos++] = aR;
  //LED[pos++] = aB;
  LED[pos++] = aB;
  LED[pos++] = aR;
  LED[pos++] = aG;

  for (i = 0; i < 3 * 8; i++)
  {
    *pPwmBit = *pLed;
    pLed += 1;
    //pPwmBit += 8;
    pPwmBit -= 8;
  }
}

void WS2812_StartE(void)
{
  DMA1_Channel7->CNDTR = LED_COUNT * 3 * 8 + 6;
  DMA1_Channel7->CCR |= DMA_CCR_EN;
  TIM4->CNT = 30;
  TIM4->CR1 |= TIM_CR1_CEN;
}

void WS2812_InitE()
{
  U32 i;

  for (i = 0; i < LED_COUNT * 3 * 8; i++) LEDBUF[i] = 12;
  for (i = LED_COUNT * 3 * 8; i < (LED_COUNT * 3 * 8 + 6); i++) LEDBUF[i] = 0;
  WS2812_Convert();

  GPIO_Init(GPIOB, 9, GPIO_TYPE_ALT_PP_50MHZ, 0);

  /* NVIC */
  //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  NVIC_SetPriorityGrouping(5);
  NVIC_SetPriority(DMA1_Channel7_IRQn, 0x02);
  NVIC_EnableIRQ(DMA1_Channel7_IRQn);

  /* DMA */
  RCC->AHBENR |= RCC_AHBENR_DMA1EN;
  DMA1->IFCR |= (DMA_ISR_GIF7 | DMA_ISR_TCIF7 | DMA_ISR_HTIF7 | DMA_ISR_TEIF7);
  DMA1_Channel7->CPAR  = (U32)&TIM4->CCR4;
  DMA1_Channel7->CMAR  = (U32)&LEDBUF[0];
  DMA1_Channel7->CCR |= //DMA_CCR_MSIZE_1 |
                        DMA_CCR_PSIZE_0 |
                        DMA_CCR_MINC |
                        //DMA_CCR_CIRC |
                        DMA_CCR_DIR |
                        DMA_CCR_TCIE;

  //* TIM * 72MHz (Lo - 12 (0.33 us), Hi - 28 (0.77 us))
  DBGMCU->CR |= DBGMCU_CR_DBG_TIM4_STOP;
  RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
  TIM4->PSC = 2 - 1;
  TIM4->ARR = 45 - 1;
  TIM4->CNT = 30;
  //Initialize TIM CH4 (Output, PWM Mode 1, Inverted, 10.67 us)
  TIM4->CCER &= ~(TIM_CCER_CC4E | TIM_CCER_CC4P);
  TIM4->CCMR2 &= ~(TIM_CCMR2_OC4CE | TIM_CCMR2_OC4M | TIM_CCMR2_OC4PE | TIM_CCMR2_OC4FE | TIM_CCMR2_CC4S);
  TIM4->CCMR2 |= TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4PE;
  TIM4->CCR4 = 0;
  TIM4->CCER |= TIM_CCER_CC4E;
  //Enable TIM
  TIM4->DIER |= TIM_DIER_UDE;
}

void DMA1_Channel7_IRQHandler(void)
{
  DMA1->IFCR |= (DMA_ISR_GIF7 | DMA_ISR_TCIF7 | DMA_ISR_HTIF7 | DMA_ISR_TEIF7);
  DMA1_Channel7->CCR &= ~(DMA_CCR_EN);
  TIM4->CR1 &= ~(TIM_CR1_CEN);
}

U32 HSV_to_RGB(U32 hue, U32 saturation, U32 value, U8 * rgb)
{
  double r, g, b;

  double h = (double)hue/360.0;
  double s = (double)saturation/255.0;
  double v = (double)value/255.0;

  int i = (int)(h * 6);
  double f = h * 6 - i;
  double p = v * (1 - s);
  double q = v * (1 - f * s);
  double t = v * (1 - (1 - f) * s);

  switch(i % 6)
  {
     case 0: r = v, g = t, b = p; break;
     case 1: r = q, g = v, b = p; break;
     case 2: r = p, g = v, b = t; break;
     case 3: r = p, g = q, b = v; break;
     case 4: r = t, g = p, b = v; break;
     case 5: r = v, g = p, b = q; break;
  }

  rgb[0] = (U8)(r * 255);
  rgb[1] = (U8)(g * 255);
  rgb[2] = (U8)(b * 255);
  return ((rgb[0] & 0xFF)<<16) | ((rgb[1] & 0xFF)<<8) | (rgb[2] & 0xFF);
}

void Fill_Rainbow(U32 sat, U32 val)
{
  U32 i;
  U32 hue;
  U8 rgb[3];

  for (i = 0; i < LED_COUNT; i++)
  {
    hue = ((360 * i) / LED_COUNT);
    HSV_to_RGB(hue, sat, val, rgb);
    WS2812_SetColor(i, rgb[0], rgb[1], rgb[2]);
  }
}




void Rainbow(void)
{
  U32 /*i,*/ loop;

  loop = 0;
  while (255 > loop)
  {
    Fill_Rainbow(loop, loop);
    WS2812_StartE();
    vTaskDelay(100);

    loop += 3;
  }

//  loop = 0;
//  while (5 > loop)
//  {
//    Fill_Rainbow(200, 100);
//    WS2812_StartE();
//    vTaskDelay(100);
//
//    for (i = 0; i < LED_COUNT; i++)
//    {
//      WS2812_SetColor(i, 0, 0, 0);
//    }
//    WS2812_StartE();
//    vTaskDelay(100);
//
//    loop++;
//  }
}


void RunningPixel(void)
{
  U32 col = 1, led = 0, x[3], loop = 0;

  GPIO_Init(GPIOC, 13, GPIO_TYPE_OUT_OD_2MHZ, 1);

  x[0] = 255;
  x[1] = 0;
  x[2] = 0;
  while (15 * LED_COUNT > loop)
  {
    WS2812_SetColor(led, 0, 0, 0);
    led++;
    if (0 == (led %= LED_COUNT))
    {
      x[col++] = 0;
      col %= 3;
      x[col] = 255;
    }
    led %= LED_COUNT;
    WS2812_SetColor(led, x[0], x[1], x[2]);
    WS2812_StartE();

    GPIO_Lo(GPIOC, 13);
    vTaskDelay(30);
    GPIO_Hi(GPIOC, 13);
    vTaskDelay(10);

    loop++;
  }
}

void BrightnessR(void)
{
  U32 led = 0, loop = 0;

  GPIO_Init(GPIOC, 13, GPIO_TYPE_OUT_OD_2MHZ, 1);

  vTaskDelay(300);

  while (255 >= loop)
  {
    for (led = 0; led < LED_COUNT; led++)
    {
      WS2812_SetColor(led, loop, 0, 0);
    }
    WS2812_StartE();
    vTaskDelay(30);
    loop++;
  }

  vTaskDelay(300);

  loop = 255;
  while (0 != loop)
  {
    for (led = 0; led < LED_COUNT; led++)
    {
      WS2812_SetColor(led, loop, 0, 0);
    }
    WS2812_StartE();
    vTaskDelay(30);
    loop--;
  }

  vTaskDelay(300);
}

void BrightnessG(void)
{
  U32 led = 0, loop = 0;

  GPIO_Init(GPIOC, 13, GPIO_TYPE_OUT_OD_2MHZ, 1);

  vTaskDelay(300);

  while (255 >= loop)
  {
    for (led = 0; led < LED_COUNT; led++)
    {
      WS2812_SetColor(led, 0, loop, 0);
    }
    WS2812_StartE();
    vTaskDelay(30);
    loop++;
  }

  vTaskDelay(300);

  loop = 255;
  while (0 != loop)
  {
    for (led = 0; led < LED_COUNT; led++)
    {
      WS2812_SetColor(led, 0, loop, 0);
    }
    WS2812_StartE();
    vTaskDelay(30);
    loop--;
  }

  vTaskDelay(300);
}

void BrightnessB(void)
{
  U32 led = 0, loop = 0;

  GPIO_Init(GPIOC, 13, GPIO_TYPE_OUT_OD_2MHZ, 1);

  vTaskDelay(300);

  while (255 >= loop)
  {
    for (led = 0; led < LED_COUNT; led++)
    {
      WS2812_SetColor(led, 0, 0, loop);
    }
    WS2812_StartE();
    vTaskDelay(30);
    loop++;
  }

  vTaskDelay(300);

  loop = 255;
  while (0 != loop)
  {
    for (led = 0; led < LED_COUNT; led++)
    {
      WS2812_SetColor(led, 0, 0, loop);
    }
    WS2812_StartE();
    vTaskDelay(30);
    loop--;
  }

  vTaskDelay(300);
}

void vLEDTask(void * pvParameters)
{
//  GPIO_Init(GPIOC, 13, GPIO_TYPE_OUT_OD_2MHZ);
//
//  while(1)
//  {
//    GPIO_Lo(GPIOC, 13);
//    vTaskDelay(500);
//    GPIO_Hi(GPIOC, 13);
//    vTaskDelay(500);
//  }
//  //vTaskDelete(NULL);

//    WS2812_SetColor(0, col, 0, 0);
//    WS2812_SetColor(1, 0, col, 0);
//    WS2812_SetColor(2, 0, 0, col);
//    WS2812_SetColor(3, col, col, 0);
//    WS2812_SetColor(4, col, 0, col);
//    WS2812_SetColor(5, col, col, 0);
//    WS2812_SetColor(6, col, 0, 0);
//    WS2812_SetColor(7, 0, 0, col);
//    Fill_Rainbow(255, 120);


//  U8 col = 1, led = 0, x[3];

  WS2812_InitE();

//  x[0] = 100;
//  x[1] = 0;
//  x[2] = 0;
  while(1)
  {
      Rainbow();
//    WS2812_SetColor(led, 0, 0, 0);
//    led++;
//    if (0 == (led %= LED_COUNT))
//    {
//      x[col++] = 0;
//      col %= 3;
//      x[col] = 200;
//    }
//    led %= LED_COUNT;
//    WS2812_SetColor(led, x[0], x[1], x[2]);
//    WS2812_StartE();

      GPIO_Lo(GPIOC, 13);
      vTaskDelay(10);
      GPIO_Hi(GPIOC, 13);
      vTaskDelay(10);
      RunningPixel();
      BrightnessR();
      BrightnessG();
      BrightnessB();
  }
}

int main(void)
{
  DBG_Init();
  DBG_ClearScreen();
  DBG_SetDefaultColors();

  printf("STM32F103C8 Started!\r\n");
  printf("ID0         = 0x%04X\r\n", UDID_0);
  printf("ID1         = 0x%04X\r\n", UDID_1);
  printf("ID2         = 0x%08X\r\n", UDID_2);
  printf("ID2         = 0x%08X\r\n", UDID_3);
  printf("Memory Size = %d kB\r\n", FLASH_SIZE);
  printf("CPU clock   = %d Hz\r\n", CPUClock);
  printf("AHB clock   = %d Hz\r\n", AHBClock);
  printf("APB1 clock  = %d Hz\r\n", APB1Clock);
  printf("APB2 clock  = %d Hz\r\n", APB2Clock);

  xTaskCreate
  (
    vLEDTask,
    "LEDTask",
    configMINIMAL_STACK_SIZE,
    NULL,
    tskIDLE_PRIORITY + 1,
    NULL
  );

  vTaskStartScheduler();

  while (FW_TRUE) {};
}

void on_error(void)
{
  while (FW_TRUE) {};
}
