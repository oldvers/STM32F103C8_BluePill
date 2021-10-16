#ifndef __CDC_SPI_H__
#define __CDC_SPI_H__

void       CDC_SPI_Init                 (void);
CDC_PORT * CDC_SPI_GetPort              (void);
void       CDC_SPI_OutStage             (void);
void       CDC_SPI_InStage              (void);
void       CDC_SPI_ProcessCollectedData (void);

#endif /* __CDC_SPI_H__ */