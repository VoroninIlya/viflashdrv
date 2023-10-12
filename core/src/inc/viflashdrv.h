/**
 * @file viflashdrv.h
 * @author Ilia Voronin (www.linkedin.com/in/ilia-voronin-7a169122a)
 * @brief Header file of flash driver
 *
 * @copyright Copyright (c) 2023 Ilia Voronin
 * 
 * This software is licensed under GNU GENERAL PUBLIC LICENSE 
 * The terms can be found in the LICENSE file in
 * the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS,
 * Without warranty of any kind, express or implied, 
 * including but not limited to the warranties of merchantability, 
 * fitness for a particular purpose and noninfringement. 
 * In no event shall the authors or copyright holders be liable for any claim, 
 * damages or other liability, whether in an action of contract, tort or otherwise, 
 * arising from, out of or in connection with the software 
 * or the use or other dealings in the software.
 * 
 */

#ifndef VIFLASHDRV_H
#define VIFLASHDRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * @brief Result type
 * 
 */
typedef enum {
	VIFLASH_RESULT_OK = 0,  /*!< 0: Successful */
	VIFLASH_RESULT_ERROR,   /*!< 1: R/W Error */
	VIFLASH_RESULT_WRPRT,   /*!< 2: Write Protected */
	VIFLASH_RESULT_NOTRDY,  /*!< 3: Not Ready */
	VIFLASH_RESULT_PARERR   /*!< 4: Invalid Parameter */
} VIFLASH_Result_t;

/**
 * @brief Debug level
 * 
 */
typedef enum {
  VIFLASH_DEBUG_DISABLED = 0,  
  VIFLASH_DEBUG_INFO,
  VIFLASH_DEBUG_ERROR,
  VIFLASH_DEBUG_LVL1,
  VIFLASH_DEBUG_LVL2
} VIFLASH_DebugLvl_t;

/**
 * @brief Erase init structure type
 *    Copy of FLASH_EraseInitTypeDef from stm32f4xx_hal_flash_ex.h
 */
typedef struct
{
  uint32_t TypeErase;   /*!< Mass erase or sector Erase.
                             This parameter can be a value of @ref FLASHEx_Type_Erase */
  uint32_t Banks;       /*!< Select banks to erase when Mass erase is enabled.
                             This parameter must be a value of @ref FLASHEx_Banks */
  uint32_t Sector;      /*!< Initial FLASH sector to erase when Mass erase is disabled
                             This parameter must be a value of @ref FLASHEx_Sectors */
  uint32_t NbSectors;   /*!< Number of sectors to be erased.
                             This parameter must be a value between 1 and (max number of sectors - value of Initial sector)*/
  uint32_t VoltageRange;/*!< The device voltage range which defines the erase parallelism
                             This parameter must be a value of @ref FLASHEx_Voltage_Range */
} VIFLASH_EraseInit_t;

typedef uint8_t (*VIFLASH_Program_t)(uint32_t TypeProgram, size_t Address, uint64_t Data);
typedef uint8_t (*VIFLASH_Unlock_t)(void);
typedef uint8_t (*VIFLASH_Lock_t)(void);
typedef uint8_t (*VIFLASH_EraseSector_t)(VIFLASH_EraseInit_t* Sector, uint32_t *SectorError);
typedef size_t (*VIFLASH_SectorToAddress_t)(uint8_t Sector);
typedef int8_t (*VIFLASH_AddressToSector_t)(size_t Address);
typedef int32_t (*VIFLASH_SectorSize_t)(uint8_t Sector);
typedef int (*VIFLASH_Printf_t) (const char *__format, ...);

/**
 * @brief Initialization of driver
 * 
 * @param[in] programCb Callback funktion to flash programming
 * @param[in] unlockCb Callback funktion to unlock flash
 * @param[in] lockCb Callback funktion to lock flash
 * @param[in] eraseSecCb Callback funktion to erase flash sector
 * @param[in] sectorToAddrCb Callback funktion to convert sector nummber to start address
 * @param[in] addrToSectorCb Callback funktion to convert address to sector number
 * @param[in] sectorSizeCb Callback funktion to get flash sector size
 * @param[in] startDiskAddress Flash address where starts disk area
 * @param[in] endDiskAddress Flash address where ends disk area
 * @param[in] ffSectorSize  File system sector size
 * @return true if initialization is successful
 * @return false if any error occured
 */
bool VIFLASH_InitDriver(
  VIFLASH_Program_t programCb,
  VIFLASH_Unlock_t unlockCb, 
  VIFLASH_Lock_t lockCb,
  VIFLASH_EraseSector_t eraseSecCb, 
  VIFLASH_SectorToAddress_t sectorToAddrCb,
  VIFLASH_AddressToSector_t addrToSectorCb, 
  VIFLASH_SectorSize_t sectorSizeCb,
  size_t startDiskAddress, 
  size_t endDiskAddress, 
  uint32_t ffSectorSize);

/**
 * @brief Write sectors to disk.
 *   This funktion is developed for file system, so
 *   sectors mean file system sectors
 * @param[in] buff Pointer to data buffer
 * @param[in] sector Start sector to write
 * @param[in] count Number of sectors to write
 * @return VIFLASH_Result_t result code, VIFLASH_RESULT_OK if everithing ok
 */
VIFLASH_Result_t VIFLASH_Write(
  const uint8_t *buff, 
  uint32_t sector, 
  uint32_t count);

/**
 * @brief 
 * 
 * @param[out] buff Pointer to data buffer
 * @param[in] sector Start sector to read
 * @param[in] count Number of sectors to read
 * @return VIFLASH_Result_t result code, VIFLASH_RESULT_OK if everithing ok
 */
VIFLASH_Result_t VIFLASH_Read(
  uint8_t *buff, 
  uint32_t sector, 
  uint32_t count);

/**
 * @brief Generic Ioctl command codes
 *   (Used by FatFs)
 */
#define VIFLASH_CTRL_SYNC         0	/*!< Complete pending write process (needed at _FS_READONLY == 0) */
#define VIFLASH_GET_SECTOR_COUNT  1	/*!< Get media size (needed at _USE_MKFS == 1) */
#define VIFLASH_GET_SECTOR_SIZE   2	/*!< Get sector size (needed at _MAX_SS != _MIN_SS) */
#define VIFLASH_GET_BLOCK_SIZE    3	/*!< Get erase block size (needed at _USE_MKFS == 1) */
#define VIFLASH_CTRL_TRIM         4	/*!< Inform device that the data on the block of sectors is no longer used (needed at _USE_TRIM == 1) */

/**
 * @brief Get info about disk
 * 
 * @param[in] cmd Comand code
 * @param[out] buff Pointer for return value
 * @return VIFLASH_Result_t result code, VIFLASH_RESULT_OK if everithing ok
 */
VIFLASH_Result_t VIFLASH_Ioctl (uint8_t cmd, void *buff);

/**
 * @brief Check if disk is busy/protected
 * 
 * @return true if disk is busy or protected
 * @return false if disk ready to use
 */
bool VIFLASH_IsWriteProtected(void);

/**
 * @brief Set printf callback.
 *   This callback can be used to get debug info from driver
 * 
 * @param printfCb Callback of prontf fuktion
 */
void VIFLASH_SetPrintfCb(VIFLASH_Printf_t printfCb);

/**
 * @brief Set debug info level
 * 
 * @param lvl debug info level
 */
void VIFLASH_SetDebugLvl(VIFLASH_DebugLvl_t lvl);

#ifdef __cplusplus
}
#endif

#endif // VIFLASHDRV_H
