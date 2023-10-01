#ifndef VIFLASHDRV_PRIVATE_H
#define VIFLASHDRV_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "viflashdrv.h"

#define TYPEPROGRAM_BYTE        0x00000000U  /*!< Program byte (8-bit) at a specified address           */
#define TYPEPROGRAM_HALFWORD    0x00000001U  /*!< Program a half-word (16-bit) at a specified address   */
#define TYPEPROGRAM_WORD        0x00000002U  /*!< Program a word (32-bit) at a specified address        */
#define TYPEPROGRAM_DOUBLEWORD  0x00000003U  /*!< Program a double word (64-bit) at a specified address */


#define TYPEERASE_SECTORS       0x00000000U  /*!< Sectors erase only          */
#define TYPEERASE_MASSERASE     0x00000001U  /*!< Flash Mass erase activation */

#define VOLTAGE_RANGE_1         0x00000000U  /*!< Device operating range: 1.8V to 2.1V                */
#define VOLTAGE_RANGE_2         0x00000001U  /*!< Device operating range: 2.1V to 2.7V                */
#define VOLTAGE_RANGE_3         0x00000002U  /*!< Device operating range: 2.7V to 3.6V                */
#define VOLTAGE_RANGE_4         0x00000003U  /*!< Device operating range: 2.7V to 3.6V + External Vpp */

#define FLASH_BANK_1            1U /*!< Bank 1   */
#define FLASH_BANK_2            2U /*!< Bank 2   */
#define FLASH_BANK_BOTH         ((uint32_t)FLASH_BANK_1 | FLASH_BANK_2) /*!< Bank1 and Bank2  */

// exacte copy of HAL_StatusTypeDef
typedef enum 
{
  STATUS_OK       = 0x00U,
  STATUS_ERROR    = 0x01U,
  STATUS_BUSY     = 0x02U,
  STATUS_TIMEOUT  = 0x03U
} Status_t;

typedef struct {
  // write controll
  size_t stopFlashAddr;
  uint32_t stopFlashSector;
  size_t startFlashAddr;
  uint32_t startFlashSector;

  uint8_t* currentFlashAddrPtr;
  uint8_t* currentBufferPtr;

  void* sectorBuffer;
}WriteCtrl_t;

typedef struct {
  VIFLASH_Program_t programCb;
  VIFLASH_Unlock_t unlockCb;
  VIFLASH_Lock_t lockCb;
  VIFLASH_EraseSector_t eraseSecCb;
  VIFLASH_SectorToAddress_t sectorToAddrCb;
  VIFLASH_AddressToSector_t addrToSectorCb;
  VIFLASH_SectorSize_t sectorSizeCb;

  size_t startDiskAddress;
  size_t endDiskAddress;
  uint32_t ffSectorSize;
  bool initialized;
  bool writeProtected;

  WriteCtrl_t wrtCtrl;

  VIFLASH_Printf_t printfCb;
  VIFLASH_DebugLvl_t debugLvl;
}Driver_t;

#ifdef __cplusplusq
}
#endif

#endif // VIFLASHDRV_PRIVATE_H