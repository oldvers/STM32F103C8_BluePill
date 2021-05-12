#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "types.h"
#include "stm32f1xx.h"

#define SYS_BBHW_R(r)         (((U32)&r - PERIPH_BASE) * 32)
#define SYS_BBHW_B(b)         (b * 4)
#define SYS_BBHW_A(r,b)       (PERIPH_BB_BASE + SYS_BBHW_R(r) + SYS_BBHW_B(b))
#define SYS_BITBAND_HW(r,b)   (*((U32 *)SYS_BBHW_A(r,b)))

extern U32 CPUClock;
extern U32 AHBClock;
extern U32 APB1Clock;
extern U32 APB2Clock;

#endif /* __SYSTEM_H__ */
