#ifndef __CDC_UART_H__
#define __CDC_UART_H__

void       CDC_UART_Init                 (void);
CDC_PORT * CDC_UART_GetPort              (void);
void       CDC_UART_OutStage             (void);
void       CDC_UART_InStage              (void);
void       CDC_UART_ProcessCollectedData (void);

#endif /* __CDC_UART_H__ */