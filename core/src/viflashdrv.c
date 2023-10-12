/**
 * @file viflashdrv.c
 * @author Ilia Voronin (www.linkedin.com/in/ilia-voronin-7a169122a)
 * @brief Source file of flash driver
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

#include "viflashdrv_private.h"
#include <stdlib.h>

static Driver_t driver = {
  NULL, /*programCb*/ NULL, /*unlockCb*/ NULL, /*lockCb*/
  NULL, /*eraseSecCb*/ NULL, /*sectorToAddrCb*/ NULL, /*addrToSectorCb*/
  NULL, /*sectorSizeCb*/ 0, /*startDiskAddress*/ 0, /*endDiskAddress*/
  0, /*ffSectorSize*/ false, /*initialized*/ false /*writeProtected*/,
  {0 /*stopFlashAddr*/, 0 /*stopFlashSector*/, 0 /*startFlashAddr*/, 0 /*startFlashSector*/, 
   NULL /*currentFlashAddrPtr*/, NULL /*currentBufferPtr*/,NULL /*sectorBuffer*/},
  NULL /*printfCb*/, 0 /*debugLvl*/
};

bool VIFLASH_InitDriver(VIFLASH_Program_t programCb,
  VIFLASH_Unlock_t unlockCb, VIFLASH_Lock_t lockCb, VIFLASH_EraseSector_t eraseSecCb, 
  VIFLASH_SectorToAddress_t sectorToAddrCb, VIFLASH_AddressToSector_t addrToSectorCb, 
  VIFLASH_SectorSize_t sectorSizeCb,
  size_t startDiskAddress, size_t endDiskAddress, uint32_t ffSectorSize) {

  driver.initialized = false;
  driver.printfCb = NULL;
  driver.programCb = NULL;
  driver.unlockCb = NULL;
  driver.lockCb = NULL;
  driver.eraseSecCb = NULL;
  driver.sectorToAddrCb = NULL;
  driver.addrToSectorCb = NULL;
  driver.sectorSizeCb = NULL;
  driver.startDiskAddress = 0;
  driver.endDiskAddress = 0;
  driver.ffSectorSize = 0;
  
  if((NULL == programCb) || (NULL == unlockCb) ||
     (NULL == lockCb) || (NULL == eraseSecCb || 
     (NULL == sectorToAddrCb) || (NULL == addrToSectorCb) ||
     (NULL == sectorSizeCb) || (0 == startDiskAddress) || 
     (0 == endDiskAddress) || 0 == ffSectorSize))
    return false;
  
  driver.programCb = programCb;
  driver.unlockCb = unlockCb;
  driver.lockCb = lockCb;
  driver.eraseSecCb = eraseSecCb;
  driver.sectorToAddrCb = sectorToAddrCb;
  driver.addrToSectorCb = addrToSectorCb;
  driver.sectorSizeCb = sectorSizeCb;

  driver.startDiskAddress = startDiskAddress;
  driver.endDiskAddress = endDiskAddress;
  driver.ffSectorSize = ffSectorSize;
  driver.writeProtected = false;
  driver.initialized = true;

  return true;
}

VIFLASH_Result_t VIFLASH_Write(const uint8_t *buff, 
  uint32_t sector, uint32_t count) {
  if(!driver.initialized) {
    if(VIFLASH_DEBUG_ERROR <= driver.debugLvl && NULL != driver.printfCb)
      driver.printfCb("ERROR: Driver not initialized\r\n");
    return VIFLASH_RESULT_NOTRDY;
  }
  if(driver.writeProtected) {
    if(VIFLASH_DEBUG_ERROR <= driver.debugLvl && NULL != driver.printfCb)
      driver.printfCb("ERROR: Write protected\r\n");
    return VIFLASH_RESULT_WRPRT;
  }
  driver.wrtCtrl.stopFlashAddr = driver.startDiskAddress + (sector+count) * driver.ffSectorSize - 1;
  driver.wrtCtrl.stopFlashSector = driver.addrToSectorCb(driver.wrtCtrl.stopFlashAddr);

  if((NULL == buff) || (0 == count) || 
     (driver.endDiskAddress <= driver.sectorToAddrCb(driver.wrtCtrl.stopFlashSector))) {
    if(VIFLASH_DEBUG_ERROR <= driver.debugLvl && NULL != driver.printfCb)
      driver.printfCb("ERROR: Wrong parameters\r\n");
    return VIFLASH_RESULT_PARERR;
  }
  driver.wrtCtrl.startFlashAddr = driver.startDiskAddress + sector * driver.ffSectorSize;
  driver.wrtCtrl.startFlashSector = driver.addrToSectorCb(driver.wrtCtrl.startFlashAddr);

  driver.writeProtected = true;
  int8_t diskSectors = driver.wrtCtrl.stopFlashSector - driver.wrtCtrl.startFlashSector + 1;
  if(0 >= diskSectors)
    return VIFLASH_RESULT_ERROR;

  driver.wrtCtrl.sectorBuffer = NULL;
  uint32_t bytesWritten = 0;
  driver.wrtCtrl.currentFlashAddrPtr = (uint8_t*)driver.sectorToAddrCb(driver.wrtCtrl.startFlashSector);
  driver.wrtCtrl.currentBufferPtr = NULL;
  
  bool success = true;
  // iterate trough each flash sector
  for(int32_t i = 0; i < diskSectors; i++) {
    int32_t currentSector = driver.wrtCtrl.startFlashSector + i;

    //allocate buffer for current sector
    uint32_t sectorSize = driver.sectorSizeCb(currentSector);
    if(VIFLASH_DEBUG_LVL1 <=  driver.debugLvl && NULL != driver.printfCb)
      driver.printfCb("Alloc memory for sector: %d; size: %d [B]\r\n", currentSector, sectorSize);
    driver.wrtCtrl.sectorBuffer = (uint8_t*)malloc(sectorSize);
    if(NULL == driver.wrtCtrl.sectorBuffer) {
      if(VIFLASH_DEBUG_DISABLED < driver.debugLvl && NULL != driver.printfCb)
        driver.printfCb("ERROR: malloc(%d) \r\n", sectorSize);
      success = false;
      break;
    }
    if(VIFLASH_DEBUG_LVL1 <= driver.debugLvl && 
       NULL != driver.printfCb)
      driver.printfCb("Memory allocated: 0x%08lX;\r\n", driver.wrtCtrl.sectorBuffer);

    driver.wrtCtrl.currentBufferPtr = driver.wrtCtrl.sectorBuffer;
    bool enableEraseSector = false;
    bool enableWriteSector = false;
    // prepare data in buffer
    if(VIFLASH_DEBUG_LVL2 <= driver.debugLvl && NULL != driver.printfCb) {
      driver.printfCb("Prepare data in buffer:\r\n");
      driver.printfCb("  ");
    }
      
    for(uint32_t j = 0; j < sectorSize; j++) {
      // if cursor out of start address to write, copy conten of flash to buffer
      if ((driver.wrtCtrl.currentFlashAddrPtr < (uint8_t*)driver.wrtCtrl.startFlashAddr) || 
          (driver.wrtCtrl.currentFlashAddrPtr > (uint8_t*)driver.wrtCtrl.stopFlashAddr)) {
        if(VIFLASH_DEBUG_LVL2 <= driver.debugLvl && NULL != driver.printfCb)
          driver.printfCb("%02X ", *(driver.wrtCtrl.currentFlashAddrPtr));
        *(driver.wrtCtrl.currentBufferPtr++) = *(driver.wrtCtrl.currentFlashAddrPtr);
      } else // else copy from incomming buffer
      if ((driver.wrtCtrl.currentFlashAddrPtr >= (uint8_t*)driver.wrtCtrl.startFlashAddr) && 
          (driver.wrtCtrl.currentFlashAddrPtr <= (uint8_t*)driver.wrtCtrl.stopFlashAddr)) {
        if((!enableEraseSector) && 
           (0xFF != *driver.wrtCtrl.currentFlashAddrPtr) && 
           (*(buff + bytesWritten) != *driver.wrtCtrl.currentFlashAddrPtr)) {
            enableEraseSector = true;
        }
        if(!enableWriteSector)
          enableWriteSector = true;
        if(VIFLASH_DEBUG_LVL2 <= driver.debugLvl && NULL != driver.printfCb)
          driver.printfCb("%02X ", *(buff + bytesWritten));
        *(driver.wrtCtrl.currentBufferPtr++) = *(buff + bytesWritten++);
      }
      driver.wrtCtrl.currentFlashAddrPtr++;
    }
    if(VIFLASH_DEBUG_LVL2 < driver.debugLvl && NULL != driver.printfCb)
      driver.printfCb("\r\n");

    Status_t stat;
    stat = driver.unlockCb();
    if(STATUS_OK != stat) {
      if(VIFLASH_DEBUG_ERROR <= driver.debugLvl && NULL != driver.printfCb)
        driver.printfCb("ERROR: Unlock");
      success = false;
    }

    if(success && enableEraseSector) {
      // erase sectors on disk
      VIFLASH_EraseInit_t eraseInit = {
        /*TypeErase*/    TYPEERASE_SECTORS, 
        /*Banks*/        FLASH_BANK_BOTH,
        /*Sector*/       currentSector,
        /*NbSectors*/    1,
        /*VoltageRange*/ VOLTAGE_RANGE_3
      };
      uint32_t sectorError = 0;
      if(VIFLASH_DEBUG_INFO <= driver.debugLvl && NULL != driver.printfCb)
        driver.printfCb("Erase sector %d\r\n", currentSector);
      do {
        stat = driver.eraseSecCb(&eraseInit, &sectorError);
      } while(STATUS_BUSY == stat);
      
      if(STATUS_OK != stat || 0xFFFFFFFFU != sectorError) {
        if(VIFLASH_DEBUG_ERROR <= driver.debugLvl && NULL != driver.printfCb)
          driver.printfCb("ERROR: Erase sector %d\r\n", currentSector);
        success = false;
      }
    }

    if(success && enableWriteSector) {
      size_t startSectorAddr = driver.sectorToAddrCb(currentSector);
      uint32_t* currentBufferPtr = (uint32_t*)driver.wrtCtrl.sectorBuffer;

      if(VIFLASH_DEBUG_INFO <= driver.debugLvl && NULL != driver.printfCb) {
        if(VIFLASH_DEBUG_LVL1 <= driver.debugLvl)
          driver.printfCb("Write sector %d; Start sector address 0x%08lX; \
Start write address 0x%08lX\r\n", currentSector, startSectorAddr, driver.wrtCtrl.startFlashAddr);
        else if(VIFLASH_DEBUG_LVL1 > driver.debugLvl)
          driver.printfCb("Write sector %d;\r\n", currentSector);
      }

      for(size_t word = startSectorAddr; word < startSectorAddr + sectorSize; word+=4) {
        char written = 's';
        if((0xFFFFFFFFU != *currentBufferPtr) && (*(uint32_t*)(word) != *currentBufferPtr)) {
          do {
            stat = driver.programCb(TYPEPROGRAM_WORD, word, *currentBufferPtr);
          } while(STATUS_BUSY == stat);
          if(STATUS_OK != stat) {
            if(VIFLASH_DEBUG_ERROR < driver.debugLvl && NULL != driver.printfCb)
              driver.printfCb("ERROR: Word write error at address 0x%08lX\r\n", word);
            success = false;
            break;
          }
          written = 'w';
        }
        if(VIFLASH_DEBUG_LVL2 < driver.debugLvl && NULL != driver.printfCb)
          driver.printfCb("0x%08lX : 0x%08lX [%c]\r\n", word, (*currentBufferPtr), written);
        currentBufferPtr++;
      }
    }

    driver.lockCb();

    free(driver.wrtCtrl.sectorBuffer);
    driver.wrtCtrl.sectorBuffer = NULL;

    if(!success)
      break;
  }

  driver.writeProtected = false;
  if(!success)
    return VIFLASH_RESULT_ERROR;
  return VIFLASH_RESULT_OK;
}

VIFLASH_Result_t VIFLASH_Read(uint8_t *buff, 
  uint32_t sector, uint32_t count) {
  if(!driver.initialized) {
    if(VIFLASH_DEBUG_ERROR <= driver.debugLvl && NULL != driver.printfCb)
      driver.printfCb("ERROR: Driver not initialized\r\n");
    return VIFLASH_RESULT_NOTRDY;
  }
  if(driver.writeProtected) {
    if(VIFLASH_DEBUG_ERROR <= driver.debugLvl && NULL != driver.printfCb)
      driver.printfCb("ERROR: Write protected\r\n");
    return VIFLASH_RESULT_NOTRDY;
  }

  size_t stopAddress = driver.startDiskAddress + (sector+count) * driver.ffSectorSize;

  if((NULL == buff) || (0 == count) || (driver.endDiskAddress <= stopAddress))
    return VIFLASH_RESULT_PARERR;

  driver.writeProtected = true;
  size_t startAddress = driver.startDiskAddress + sector * driver.ffSectorSize;

  size_t* currentBuffAddr = (size_t*)buff;
  if(VIFLASH_DEBUG_INFO <= driver.debugLvl && NULL != driver.printfCb)
      driver.printfCb("Start read from 0x%08lX, %ld bytes.\r\n", startAddress, stopAddress-startAddress);

  while(startAddress < stopAddress) {
    if(VIFLASH_DEBUG_LVL2 <= driver.debugLvl && NULL != driver.printfCb)
      driver.printfCb("0x%08lX : 0x%08lX\r\n", startAddress, *(size_t*)(startAddress));
    *(currentBuffAddr++) = *(size_t*)(startAddress);
    startAddress+=4;
  }

  driver.writeProtected = false;
  return VIFLASH_RESULT_OK;
}

VIFLASH_Result_t VIFLASH_Ioctl(uint8_t cmd, void *buff) {
  if(!driver.initialized)
    return VIFLASH_RESULT_NOTRDY;
  
  switch(cmd){
    case VIFLASH_CTRL_SYNC: {
      return VIFLASH_RESULT_OK;
      break;
    }
    case VIFLASH_GET_SECTOR_COUNT: {
      if(NULL == buff)
        return VIFLASH_RESULT_PARERR;
      uint32_t diskSizeBytes = driver.endDiskAddress - driver.startDiskAddress;
      uint32_t diskSizeSectors = diskSizeBytes / driver.ffSectorSize;
      if(VIFLASH_DEBUG_INFO <= driver.debugLvl && NULL != driver.printfCb)
          driver.printfCb("Disk size %ld [B]; FF-Sectors %ld\r\n", diskSizeBytes, diskSizeSectors);
      *(uint32_t*)buff = diskSizeSectors;
      return VIFLASH_RESULT_OK;
      break;
    }
    case VIFLASH_GET_SECTOR_SIZE: {
      if(NULL == buff)
        return VIFLASH_RESULT_PARERR;
      if(VIFLASH_DEBUG_INFO <= driver.debugLvl && NULL != driver.printfCb)
          driver.printfCb("FF-Sector size %ld\r\n", driver.ffSectorSize);
      *(uint32_t*)buff = driver.ffSectorSize;
      return VIFLASH_RESULT_OK;
      break;
    }
    case VIFLASH_GET_BLOCK_SIZE: {
      if(NULL == buff)
        return VIFLASH_RESULT_PARERR;
      if(VIFLASH_DEBUG_INFO <= driver.debugLvl && NULL != driver.printfCb)
          driver.printfCb("FF-Block size %ld\r\n", driver.sectorSizeCb(0) / driver.ffSectorSize);
      *(uint32_t*)buff = driver.sectorSizeCb(0) / driver.ffSectorSize;
      return VIFLASH_RESULT_OK;
      break;
    }
    case VIFLASH_CTRL_TRIM: {
      return VIFLASH_RESULT_OK;
      break;
    }
  }
  return VIFLASH_RESULT_PARERR;
}

bool VIFLASH_IsWriteProtected(void) {
  return driver.writeProtected;
}

void VIFLASH_SetPrintfCb(VIFLASH_Printf_t printfCb) {
  driver.printfCb = printfCb;
}

void VIFLASH_SetDebugLvl(VIFLASH_DebugLvl_t lvl) {
  driver.debugLvl = lvl;
}