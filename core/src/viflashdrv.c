#include "viflashdrv_private.h"
#include <stdlib.h>

static Driver_t driver = {
  NULL, /*programCb*/ NULL, /*unlockCb*/ NULL, /*lockCb*/
  NULL, /*eraseSecCb*/ NULL, /*sectorToAddrCb*/ NULL, /*addrToSectorCb*/
  NULL, /*sectorSizeCb*/ 0, /*startDiskAddress*/ 0, /*endDiskAddress*/
  0, /*ffSectorSize*/ false, /*initialized*/ false /*writeProtected*/,
  {0 /*stopFlashAddr*/, 0 /*stopFlashSector*/, 0 /*startFlashAddr*/, 0 /*startFlashSector*/, 
   NULL /*currentFlashAddrPtr*/, NULL /*currentBufferPtr*/,NULL /*sectorBuffer*/}
  };

bool VIFLASH_InitDriver(VIFLASH_Program_t programCb,
  VIFLASH_Unlock_t unlockCb, VIFLASH_Lock_t lockCb,
  VIFLASH_EraseSector_t eraseSecCb, VIFLASH_SectorToAddress_t sectorToAddrCb,
  VIFLASH_AddressToSector_t addrToSectorCb, VIFLASH_SectorSize_t sectorSizeCb,
  uint32_t startDiskAddress, uint32_t endDiskAddress, uint32_t ffSectorSize) {

  driver.initialized = false;
  
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
  driver.initialized = true;
  driver.writeProtected = false;

  return true;
}

VIFLASH_Result_t VIFLASH_Write(const uint8_t *buff, 
  uint32_t sector, uint32_t count)
{
  if(!driver.initialized) {
    return VIFLASH_RESULT_NOTRDY;
  }
  if(driver.writeProtected) {
    return VIFLASH_RESULT_WRPRT;
  }
  driver.wrtCtrl.stopFlashAddr = driver.startDiskAddress + (sector+count) * driver.ffSectorSize;
  driver.wrtCtrl.stopFlashSector = driver.addrToSectorCb(driver.wrtCtrl.stopFlashAddr);

  if((NULL == buff) || (0 == count) || (driver.endDiskAddress <= driver.wrtCtrl.stopFlashSector)) {
    return VIFLASH_RESULT_PARERR;
  }
  driver.wrtCtrl.startFlashAddr = driver.startDiskAddress + sector * driver.ffSectorSize;
  driver.wrtCtrl.startFlashSector = driver.addrToSectorCb(driver.wrtCtrl.startFlashAddr);

  driver.writeProtected = true;
  //allocate pointers to buffers for all sectors
  int8_t diskSectors = driver.wrtCtrl.stopFlashSector - driver.wrtCtrl.startFlashSector + 1;
  if(0 >= diskSectors)
    return VIFLASH_RESULT_ERROR;

  driver.wrtCtrl.sectorBuffer = NULL;
  //uint32_t totalBytesToWrite = (count)*driver.ffSectorSize;
  uint32_t bytesWritten = 0;
  driver.wrtCtrl.currentFlashAddrPtr = (uint8_t*)driver.sectorToAddrCb(driver.wrtCtrl.startFlashSector);
  driver.wrtCtrl.currentBufferPtr = NULL;

  bool success = true;
  // iterate trough each flash sector
  for(int32_t i = 0; i < diskSectors; i++) {
    int32_t currentSector = driver.wrtCtrl.startFlashSector + i;

    //allocate buffer for current sector
    uint32_t sectorSize = driver.sectorSizeCb(currentSector);
    driver.wrtCtrl.sectorBuffer = (uint8_t*)malloc(sectorSize);
    if(NULL == driver.wrtCtrl.sectorBuffer) {
      break;
    }
    driver.wrtCtrl.currentBufferPtr = driver.wrtCtrl.sectorBuffer;
    bool enableEraseSector = false;
    bool enableWriteSector = false;
    // prepare data in buffer
    for(uint32_t j = 0; j < sectorSize; j++) {
      // if cursor out of start address to write, copy conten of flash to buffer
      if ((driver.wrtCtrl.currentFlashAddrPtr < (uint8_t*)driver.wrtCtrl.startFlashAddr) || 
          (driver.wrtCtrl.currentFlashAddrPtr >= (uint8_t*)driver.wrtCtrl.stopFlashAddr)) {
        *(driver.wrtCtrl.currentBufferPtr++) = *(driver.wrtCtrl.currentFlashAddrPtr);
      } else // else copy from incomming buffer
      if ((driver.wrtCtrl.currentFlashAddrPtr >= (uint8_t*)driver.wrtCtrl.startFlashAddr) && 
          (driver.wrtCtrl.currentFlashAddrPtr < (uint8_t*)driver.wrtCtrl.stopFlashAddr)) {
        if((!enableEraseSector) && 
           (0xFF != *driver.wrtCtrl.currentFlashAddrPtr) && 
           (*(buff + bytesWritten) != *driver.wrtCtrl.currentFlashAddrPtr)) {
            enableEraseSector = true;
        }
        if(!enableWriteSector)
          enableWriteSector = true;
        *(driver.wrtCtrl.currentBufferPtr++) = *(buff + bytesWritten++);
      }
      driver.wrtCtrl.currentFlashAddrPtr++;
    }
    driver.unlockCb();

    Status_t stat;
    if(enableEraseSector) {
      // erase sectors on disk
      VIFLASH_EraseInit_t eraseInit = {
        /*TypeErase*/    TYPEERASE_SECTORS, 
        /*Banks*/        FLASH_BANK_BOTH,
        /*Sector*/       currentSector,
        /*NbSectors*/    1,
        /*VoltageRange*/ VOLTAGE_RANGE_3
      };
      uint32_t sectorError = 0;
      do 
      {
        stat = driver.eraseSecCb(&eraseInit, &sectorError);
      }while(STATUS_BUSY == stat);
      
      if(STATUS_OK != stat || 0xFFFFFFFFU != sectorError) {
        success = false;
      }
    }
    if(success && enableWriteSector) {
      uint32_t startSectorAddr = driver.sectorToAddrCb(currentSector);
      uint32_t* currentBufferPtr = (uint32_t*)driver.wrtCtrl.sectorBuffer;
      for(uint32_t word = startSectorAddr; word < startSectorAddr + sectorSize; word+=4) {
        if((0xFFFFFFFFU != *currentBufferPtr) && (*(uint32_t*)(word) != *currentBufferPtr)) {
          do 
          {
            stat = driver.programCb(TYPEPROGRAM_WORD, word, *currentBufferPtr);
          }while(STATUS_BUSY == stat);
          if(STATUS_OK != stat) {
            success = false;
            break;
          }
        }
        currentBufferPtr++;
      }
    }

    driver.lockCb();

    free(driver.wrtCtrl.sectorBuffer);
    driver.wrtCtrl.sectorBuffer = NULL;
  }

  driver.writeProtected = false;
  if(false == success)
    return VIFLASH_RESULT_ERROR;
  return VIFLASH_RESULT_OK;
}

VIFLASH_Result_t VIFLASH_Read (uint8_t *buff, 
  uint32_t sector, uint32_t count)
{
  if(!driver.initialized)
    return VIFLASH_RESULT_NOTRDY;

  uint32_t stopAddress = driver.startDiskAddress + (sector+count) * driver.ffSectorSize;

  if((NULL == buff) || (0 == count) || (driver.endDiskAddress <= stopAddress))
    return VIFLASH_RESULT_PARERR;

  uint32_t startAddress = driver.startDiskAddress + sector * driver.ffSectorSize;

  uint32_t* currentBuffAddr = (uint32_t*)buff;
  while(startAddress < stopAddress) {
    *(currentBuffAddr++) = *(uint32_t*)(startAddress);
    startAddress+=4;
  }
  return VIFLASH_RESULT_OK;
}

VIFLASH_Result_t VIFLASH_Ioctl (uint8_t cmd, void *buff) {
  switch(cmd){
    case VIFLASH_CTRL_SYNC:{
      return VIFLASH_RESULT_OK;
      break;
    }
    case VIFLASH_GET_SECTOR_COUNT:{
      uint32_t diskSizeBytes = driver.endDiskAddress - driver.startDiskAddress;
      uint32_t diskSizeSectors = diskSizeBytes / driver.ffSectorSize;
      *(uint32_t*)buff = diskSizeSectors;
      return VIFLASH_RESULT_OK;
      break;
    }
    case VIFLASH_GET_SECTOR_SIZE:{
      *(uint32_t*)buff = driver.ffSectorSize;
      return VIFLASH_RESULT_OK;
      break;
    }
    case VIFLASH_GET_BLOCK_SIZE:{
      *(uint32_t*)buff = driver.sectorSizeCb(0) / driver.ffSectorSize;
      return VIFLASH_RESULT_OK;
      break;
    }
    case VIFLASH_CTRL_TRIM:{
      return VIFLASH_RESULT_OK;
      break;
    }
  }
  return VIFLASH_RESULT_PARERR;
}

bool VIFLASH_IsWriteProtected(void) {
  return driver.writeProtected;
}