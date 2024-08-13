/**
 * @file viflashdrv_private.h
 * @author Ilia Voronin (www.linkedin.com/in/ilia-voronin-7a169122a)
 * @brief Private header file of flash driver
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

#ifndef VIFLASHDRV_PRIVATE_H
#define VIFLASHDRV_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "viflashdrv.h"

/**
 * @brief Defines inherited from stm32f4xx used by erase sector
 * 
 */
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

/**
 * @brief Erase status type
 *    exacte copy of HAL_StatusTypeDef
 */
typedef enum 
{
  STATUS_OK       = 0x00U,
  STATUS_ERROR    = 0x01U,
  STATUS_BUSY     = 0x02U,
  STATUS_TIMEOUT  = 0x03U
} Status_t;

/**
 * @brief Write process control type
 * 
 */
typedef struct {
  size_t stopFlashAddr;         /*!< Stop flash address for current write action */
  uint32_t stopFlashSector;     /*!< Stop flash sector for current write action */
  size_t startFlashAddr;        /*!< Start flash address for current write action */
  uint32_t startFlashSector;    /*!< Start flash sector for current write action */

  uint8_t* currentFlashAddrPtr; /*!< Cursor to current flash address - destination adress*/
  uint8_t* currentBufferPtr;    /*!< Cursor to current buffer address - source address*/

  void* sectorBuffer;           /*!< Pointer to allocated buffer for current flash sector*/
}WriteCtrl_t;

/**
 * @brief Main driver structur
 * 
 */
typedef struct {
  VIFLASH_Program_t programCb;              /*!< Callback funktion to flash programming */
  VIFLASH_Unlock_t unlockCb;                /*!< Callback funktion to unlock flash */
  VIFLASH_Lock_t lockCb;                    /*!< Callback funktion to lock flash */
  VIFLASH_EraseSector_t eraseSecCb;         /*!< Callback funktion to erase flash sector */
  VIFLASH_SectorToAddress_t sectorToAddrCb; /*!< Callback funktion to convert sector nummber to start address */
  VIFLASH_AddressToSector_t addrToSectorCb; /*!< Callback funktion to convert address to sector number */
  VIFLASH_SectorSize_t sectorSizeCb;        /*!< Callback funktion to get flash sector size */

  size_t startDiskAddress;                  /*!< Flash address where starts disk area */
  size_t endDiskAddress;                    /*!< Flash address where ends disk area */
  uint32_t ffSectorSize;                    /*!< File system sector size */
  bool initialized;                         /*!< Driver initialization flag */
  bool writeProtected;                      /*!< Flash protection flag */

  WriteCtrl_t wrtCtrl;                      /*!< Write control structure */

  VIFLASH_Printf_t printfCb;                /*!< Printf callback */
  VIFLASH_DebugLvl_t debugLvl;              /*!< Debug level */
}Driver_t;

#ifdef __cplusplusq
}
#endif

#endif // VIFLASHDRV_PRIVATE_H