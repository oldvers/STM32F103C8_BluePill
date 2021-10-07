#ifndef __CDC_I2C_H__
#define __CDC_I2C_H__

void       CDC_I2C_Init                 (void);
CDC_PORT * CDC_I2C_GetPort              (void);
void       CDC_I2C_OutStage             (void);
void       CDC_I2C_InStage              (void);
void       CDC_I2C_ProcessCollectedData (void);

#endif /* __CDC_I2C_H__ */