#ifndef __SENSORS_H__
#define __SENSORS_H__

typedef __packed struct _SENS_DATA
{
  U16 TimeStamp;
  S16 AX;
  S16 AY;
  S16 AZ;
  U16 GX;
  U16 GY;
  U16 GZ;
  U16 CX;
  U16 CY;
  U16 CZ;
  U16 Temp;
  U32 Pres;
} SENS_DATA;

void Sensors_Init(void);
void Sensors_Measure(SENS_DATA * pData);

#endif /* __SENSORS_H__ */
