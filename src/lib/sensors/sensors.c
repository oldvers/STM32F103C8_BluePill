#include "system.h"
#include "board.h"
#include "gpio.h"
#include "i2c.h"
#include "sensors.h"
#include "debug.h"

//#define ADXL345                      (0x1D)
#define ADXL345                        (0x53)
#define ADXL345_R_DEVID                (0x00)
#define ADXL345_DEVID                  (0xE5)
#define ADXL345_R_BW_RATE              (0x2C)
#define ADXL345_BW_RATE_0_10HZ         (0x00)
#define ADXL345_BW_RATE_0_20HZ         (0x01)
#define ADXL345_BW_RATE_0_39HZ         (0x02)
#define ADXL345_BW_RATE_0_78HZ         (0x03)
#define ADXL345_BW_RATE_1_56HZ         (0x04)
#define ADXL345_BW_RATE_3_13HZ         (0x05)
#define ADXL345_BW_RATE_6_25HZ         (0x06)
#define ADXL345_BW_RATE_12_5HZ         (0x07)
#define ADXL345_BW_RATE_25HZ           (0x08)
#define ADXL345_BW_RATE_50HZ           (0x09)
#define ADXL345_BW_RATE_100HZ          (0x0A)
#define ADXL345_BW_RATE_200HZ          (0x0B)
#define ADXL345_BW_RATE_400HZ          (0x0C)
#define ADXL345_BW_RATE_800HZ          (0x0D)
#define ADXL345_BW_RATE_1600HZ         (0x0E)
#define ADXL345_BW_RATE_3200HZ         (0x0F)
#define ADXL345_R_DATA_FORMAT          (0x31)
#define ADXL345_DATA_FORMAT_RANGE_2G   (0x00)
#define ADXL345_DATA_FORMAT_RANGE_4G   (0x01)
#define ADXL345_DATA_FORMAT_RANGE_8G   (0x02)
#define ADXL345_DATA_FORMAT_RANGE_16G  (0x03)
#define ADXL345_DATA_FORMAT_FULL_RES   (0x08)
#define ADXL345_R_POWER_CTL            (0x2D)
#define ADXL345_POWER_CTL_MEASURE      (0x08)
#define ADXL345_R_DATAX0               (0x32)
#define ADXL345_R_DATAX1               (0x33)
#define ADXL345_R_DATAY0               (0x34)
#define ADXL345_R_DATAY1               (0x35)
#define ADXL345_R_DATAZ0               (0x36)
#define ADXL345_R_DATAZ1               (0x37)

typedef __packed struct _ADXL345_DATA
{
  U16 X;
  U16 Y;
  U16 Z;
} ADXL345_DATA;

/** @brief Initializes sensors unit
  * @param None
  * @return None
  */
void Sensors_Init(void)
{
  U8 buffer[2];

  GPIO_Init(SENS_I2C_SCL_PORT, SENS_I2C_SCL_PIN, GPIO_TYPE_ALT_OD_10MHZ, 1);
  GPIO_Init(SENS_I2C_SDA_PORT, SENS_I2C_SDA_PIN, GPIO_TYPE_ALT_OD_10MHZ, 1);

  I2C_Init(SENS_I2C);

  do
  {
    /* Set Device ID register address */
    buffer[0] = ADXL345_R_DEVID;
    if (1 != I2C_MasterWrite(SENS_I2C, ADXL345, buffer, 1)) break;

    /* Read Device ID */
    if (1 != I2C_MasterRead(SENS_I2C, ADXL345, buffer, 1)) break;

    /* Check if Device ID is correct */
    if (ADXL345_DEVID != buffer[0]) break;
    DBG("I2C: ADXL Dev ID = 0x%02X\r\n", buffer[0]);

    /* Setup Bandwidth and Data Rate */
    buffer[0] = ADXL345_R_BW_RATE;
    buffer[1] = ADXL345_BW_RATE_1600HZ;
    if (2 != I2C_MasterWrite(SENS_I2C, ADXL345, buffer, 2)) break;

    /* Setup data format and range */
    buffer[0] = ADXL345_R_DATA_FORMAT;
    buffer[1] = ADXL345_DATA_FORMAT_RANGE_4G | ADXL345_DATA_FORMAT_FULL_RES;
    if (2 != I2C_MasterWrite(SENS_I2C, ADXL345, buffer, 2)) break;

    /* Enable measurements */
    buffer[0] = ADXL345_R_POWER_CTL;
    buffer[1] = ADXL345_POWER_CTL_MEASURE;
    if (2 != I2C_MasterWrite(SENS_I2C, ADXL345, buffer, 2)) break;
  }
  while (FW_FALSE);
}

/** @brief Gets measurements from the sensors
  * @param Pointer to structure where measurements will be placed
  * @return None
  */
void Sensors_Measure(SENS_DATA * pData)
{
  U8 buffer[8];

  do
  {
    /* Set Data register address */
    buffer[0] = ADXL345_R_DATAX0;
    if (1 != I2C_MasterWrite(SENS_I2C, ADXL345, buffer, 1)) break;

    /* Read measurements */
    if (6 != I2C_MasterRead(SENS_I2C, ADXL345, buffer, 6)) break;

    DBG("*****\r\n");
    DBG("I2C: ADXL X = 0x%04X\r\n", ((ADXL345_DATA *)buffer)->X);
    DBG("I2C: ADXL Y = 0x%04X\r\n", ((ADXL345_DATA *)buffer)->Y);
    DBG("I2C: ADXL Z = 0x%04X\r\n", ((ADXL345_DATA *)buffer)->Z);

    pData->AX = ((ADXL345_DATA *)buffer)->X;
    pData->AY = ((ADXL345_DATA *)buffer)->Y;
    pData->AZ = ((ADXL345_DATA *)buffer)->Z;
  }
  while (FW_FALSE);
}
