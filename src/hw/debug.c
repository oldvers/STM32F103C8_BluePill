#include <string.h>
#include <stdio.h>
#include "types.h"
#include "hardware.h"

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

#if defined(__ICCARM__)
size_t __write(int handle, const unsigned char * buffer, size_t size)
{
  /* PB3 (JTDO/TRACESWO) is used for debug output */
  for (U32 i = 0; i< size; i++) ITM_SendChar(*buffer++);
  return size;
}
#endif
