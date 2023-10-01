#ifndef VIFLASHDRV_H
#define VIFLASHDRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Results of Disk Functions 
typedef enum {
	VIFLASH_RESULT_OK = 0,  /* 0: Successful */
	VIFLASH_RESULT_ERROR,   /* 1: R/W Error */
	VIFLASH_RESULT_WRPRT,   /* 2: Write Protected */
	VIFLASH_RESULT_NOTRDY,  /* 3: Not Ready */
	VIFLASH_RESULT_PARERR   /* 4: Invalid Parameter */
} VIFLASH_Result_t;

typedef enum {
  VIFLASH_DEBUG_DISABLED = 0,  
  VIFLASH_DEBUG_INFO,
  VIFLASH_DEBUG_ERROR,
  VIFLASH_DEBUG_LVL1,
  VIFLASH_DEBUG_LVL2
} VIFLASH_DebugLvl_t;

/* Generic command (Used by FatFs) */
#define VIFLASH_CTRL_SYNC         0	/* Complete pending write process (needed at _FS_READONLY == 0) */
#define VIFLASH_GET_SECTOR_COUNT  1	/* Get media size (needed at _USE_MKFS == 1) */
#define VIFLASH_GET_SECTOR_SIZE   2	/* Get sector size (needed at _MAX_SS != _MIN_SS) */
#define VIFLASH_GET_BLOCK_SIZE    3	/* Get erase block size (needed at _USE_MKFS == 1) */
#define VIFLASH_CTRL_TRIM         4	/* Inform device that the data on the block of sectors is no longer used (needed at _USE_TRIM == 1) */

// Copy of FLASH_EraseInitTypeDef from stm32f4xx_hal_flash_ex.h
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

/*!
Driver initialization
\param[in] programCb - @TODO Description
\param[in] unlockCb - @TODO Description
\param[in] lockCb - @TODO Description
\param[in] eraseSecCb - @TODO 
\param[in] sectorToAddrCb - @TODO Description
\param[in] addrToSectorCb - @TODO Description
\param[in] sectorSizeCb - @TODO Description
\param[in] startDiskAddress - @TODO Description
\param[in] endDiskAddress - @TODO Description
\param[in] ffSectorSize - @TODO Description
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

/*!
Write sectors to flash
\param[in] buff - @TODO Description
\param[in] sector - @TODO Description
\param[in] count - @TODO Description
*/
VIFLASH_Result_t VIFLASH_Write(
  const uint8_t *buff, 
  uint32_t sector, 
  uint32_t count);

/*!
Read sectors from flash
\param[out] buff - @TODO Description
\param[in] sector - @TODO Description
\param[in] count - @TODO Description
*/
VIFLASH_Result_t VIFLASH_Read(
  uint8_t *buff, 
  uint32_t sector, 
  uint32_t count);

/*!
Control disk
\param[in] cmd - @TODO Description
\param[out] buff - @TODO Description
*/
VIFLASH_Result_t VIFLASH_Ioctl (uint8_t cmd, void *buff);

/*!
Check if flash is busy
*/
bool VIFLASH_IsWriteProtected(void);

void VIFLASH_SetPrintfCb(VIFLASH_Printf_t printfCb);
void VIFLASH_SetDebugLvl(VIFLASH_DebugLvl_t lvl);

#ifdef __cplusplus
}
#endif

#endif // VIFLASHDRV_H
