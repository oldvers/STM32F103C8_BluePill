#ifndef __TIM_H__
#define __TIM_H__

#include "types.h"

typedef enum
{
  TIM_CH1 = 0,
  TIM_CH2,
  TIM_CH3,
  TIM_CH4,
  TIM_CH_COUNT,
} TIM_CH_t;

/* Callback Function Declarations */
typedef void (* TIM_CbComplete_t)(TIM_CH_t aChannel, U16 aValue);

/* Function Declarations */
void TIM2_IrqHandler       (void);
void TIM2_Enable           (void);
void TIM2_Disable          (void);
void TIM2_InitInputCapture (U8 aChannel, TIM_CbComplete_t pCbComplete);
void TIM2_DeInit           (void);

#endif /* __TIM_H__ */
