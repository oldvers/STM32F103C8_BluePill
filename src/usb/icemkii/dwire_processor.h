#ifndef __DWIRE_PROCESSOR_H__
#define __DWIRE_PROCESSOR_H__

#include "types.h"

/** @brief Initializes the dWire
 *  @param None
 *  @return None
 */
void DWire_Init(void);

/** @brief Sets the dWire device parameters
 *  @param dwdr - Address of the DWDR register
 *  @param spmcsr - Address of the SPMCSR register
 *  @param basePC - Base address of the PC register
 *  @return None
 */
void DWire_SetParams(U8 dwdr, U8 spmcsr, U16 basePC);

/** @brief Sets the dWire device memory parameters
 *  @param rSize - SRAM size
 *  @param fSize - FLASH size
 *  @param rPageSize - FLASH page size
 *  @param eSize - EEPROM size
 *  @return None
 */
void DWire_SetMemParams(U16 rSize, U16 fSize, U16 fPageSize, U16 eSize);

/** @brief Performs the dWire synchronization procedure
 *  @param None
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_Sync(void);

/** @brief Reads the device signature
 *  @param None
 *  @return The signature, 0 - in case of error
 */
U16 DWire_ReadSignature(void);

/** @brief Reads the device program counter
 *  @param None
 *  @return The program counter
 */
U16 DWire_ReadPC(void);

/** @brief Disables the dWire
 *  @param None
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_Disable(void);

/** @brief Reads the device register
 *  @param reg - The address of the register
 *  @return Register's value
 */
U8 DWire_ReadReg(U16 reg);





/** @brief Reads the device registers
 *  @param first - The address of the first register
 *  @param pBuffer - The container for registers
 *  @param count - The count of registers to be read
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_ReadRegs(U16 first, U8 * pBuffer, U16 count);



/** @brief Writes the device register
 *  @param reg - The address of the register
 *  @param value - The register's value
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_WrireReg(U16 reg, U8 value);

/** @brief Writes the device registers
 *  @param first - The address of the first register
 *  @param pBuffer - The container with registers' values
 *  @param count - The count of registers to be written
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_WriteRegs(U16 first, U8 * pBuffer, U16 count);

/** @brief Sets the device Z register
 *  @param address - The value of Z register
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_SetZ(U16 address);


#endif /* __DWIRE_PROCESSOR_H__ */
