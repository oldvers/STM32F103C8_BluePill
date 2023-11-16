#ifndef __DWIRE_PROCESSOR_H__
#define __DWIRE_PROCESSOR_H__

#include "types.h"

/** @brief Initializes the dWire
 *  @param None
 *  @return None
 */
void DWire_Init(void);

/** @brief Sets the dWire target parameters
 *  @param dwdr - Address of the DWDR register
 *  @param spmcsr - Address of the SPMCSR register
 *  @param basePC - Base address of the PC register
 *  @return None
 */
void DWire_SetParams(U8 dwdr, U8 spmcsr, U16 basePC, U16 bootStart);

/** @brief Sets the dWire target memory parameters
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

/** @brief Reads the target signature
 *  @param None
 *  @return The signature, 0 - in case of error
 */
U16 DWire_GetSignature(void);

/** @brief Reads the target program counter
 *  @param None
 *  @return The program counter
 *  @note The PC is also stored internally into the global variable for using
 *        with step into/over/out functions
 */
U16 DWire_GetPC(void);

/** @brief Disables the dWire
 *  @param None
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_Disable(void);

/** @brief Reads the target register
 *  @param reg - Register's number (0x00..0x20)
 *  @return Register's value
 */
U8 DWire_GetReg(U8 reg);

/** @brief Writes the target register
 *  @param reg - Register's number (0x00..0x20)
 *  @param value - Register's value
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_SetReg(U8 reg, U8 value);

/** @brief Reads the target I/O register
 *  @param reg - Register's number (0x20..0x5F)
 *  @return Register's value
 */
U8 DWire_GetIOReg(U8 reg);

/** @brief Writes the target I/O register
 *  @param reg - Register's number (0x20..0x5F)
 *  @param value - Register's value
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_SetIOReg(U8 reg, U8 value);

/** @brief Reads the target registers
 *  @param first - The number of the first register
 *  @param pRaw - The container for registers' values
 *  @param count - The count of registers to be read
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_GetRegs(U8 first, U8 * pRaw, U8 count);

/** @brief Writes the target registers
 *  @param first - The number of the first register
 *  @param pRaw - The container with registers' values
 *  @param count - The count of registers to be written
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_SetRegs(U8 first, U8 * pRaw, U8 count);

/** @brief Reads the target's SRAM
 *  @param address - The address in SRAM
 *  @param pRaw - The container for SRAM data
 *  @param length - The count of bytes to be read
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_GetSRAM(U16 address, U8 * pRaw, U16 length);

/** @brief Writes the target's SRAM
 *  @param address - The address in SRAM
 *  @param pRaw - The container with SRAM content
 *  @param length - The count of bytes to be written
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_SetSRAM(U16 address, U8 * pRaw, U16 length);

/** @brief Reads the target's FLASH
 *  @param address - The address in FLASH
 *  @param pRaw - The container for FLASH data
 *  @param length - The count of bytes to be read
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_GetFlash(U16 address, U8 * pRaw, U16 length);

/** @brief Writes the target's FLASH
 *  @param address - The address in FLASH
 *  @param pRaw - The container with FLASH content
 *  @param length - The count of bytes to be written
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_SetFlash(U16 address, U8 * pRaw, U16 length);

/** @brief Resets the target
 *  @param None
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_Reset(void);

/** @brief Runs the target
 *  @param None
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_Run(void);

/** @brief Stops the target
 *  @param None
 *  @return None
 */
void DWire_Break(void);

/** @brief Checks if the target was stopped
 *  @param None
 *  @return True - in case of success
 */
FW_BOOLEAN DWire_CheckForBreak(void);

/** @brief Performs the step into operation on the target
 *  @param None
 *  @return True - in case of success
 *  @note The DWire_GetPC must be called before this function
 */
FW_BOOLEAN DWire_StepInto(void);

/** @brief Performs the step over operation on the target
 *  @param None
 *  @return True - in case of success
 *  @note The DWire_GetPC must be called before this function
 */
FW_BOOLEAN DWire_StepOver(void);

/** @brief Performs the step out operation on the target
 *  @param None
 *  @return True - in case of success
 *  @note The DWire_GetPC must be called before this function
 */
FW_BOOLEAN DWire_StepOut(void);

/** @brief Performs the run to cursor operation on the target
 *  @param address - The address of the cursor
 *  @return True - in case of success
 *  @note The DWire_GetPC must be called before this function
 */
FW_BOOLEAN DWire_RunToCursor(U16 address);

#endif /* __DWIRE_PROCESSOR_H__ */
