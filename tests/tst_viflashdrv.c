#include "unity.h"
#include "unity_fixture.h"
#include "viflashdrv.h"
#include "stdio.h"
#include <pthread.h>

static void* thread1Entry(void *arg);
static void* thread2Entry(void *arg);

static uint32_t calledProgramCounter = 0;
static VIFLASH_Result_t programReturn = VIFLASH_RESULT_OK;
static uint8_t FAKE_Program(uint32_t TypeProgram, size_t Address, uint64_t Data);
static uint32_t calledUnlockCounter = 0;
static VIFLASH_Result_t unlockReturn = VIFLASH_RESULT_OK;
static uint8_t FAKE_Unlock(void);
static uint32_t calledLockCounter = 0;
static uint8_t FAKE_Lock(void);
static uint32_t calledEraseCounter = 0;
static VIFLASH_Result_t eraseReturn = VIFLASH_RESULT_OK;
static uint8_t FAKE_EraseSector(VIFLASH_EraseInit_t* Sector, uint32_t *SectorError);
static size_t FAKE_SectorToAddress(uint8_t Sector);
static int8_t FAKE_AddressToSector(size_t Address);
static int32_t FAKE_SectorSize(uint8_t Sector);

TEST_GROUP(TST_VIFLASHDRV);

TEST_GROUP_RUNNER(TST_VIFLASHDRV) {
  RUN_TEST_CASE(TST_VIFLASHDRV, VIFLASH_Ioctl);
  RUN_TEST_CASE(TST_VIFLASHDRV, VIFLASH_Write);
  RUN_TEST_CASE(TST_VIFLASHDRV, VIFLASH_IsWriteProtected);
}

#define DISK_SIZE (128)
#define DISK_SECTOR_SIZE (32)
#define FFSECTOR_SIZE (16)

static uint8_t testDisk[DISK_SIZE] = {255};

static uint8_t testBuff[DISK_SIZE] = {0};

TEST_SETUP(TST_VIFLASHDRV) {
  calledProgramCounter = 0;
  calledUnlockCounter = 0;
  calledLockCounter = 0;
  calledEraseCounter = 0;

  for(uint32_t i = 0; i < DISK_SIZE; i++) {
    testDisk[i] = 0xFF;
  }
}

TEST_TEAR_DOWN(TST_VIFLASHDRV) {
  VIFLASH_InitDriver(NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, 0, 0, 0);
}

// ===================================================================================
// Test VIFLASH_Ioctl ================================================================
TEST(TST_VIFLASHDRV, VIFLASH_Ioctl)
{
  VIFLASH_SetDebugLvl(VIFLASH_DEBUG_LVL2);
  VIFLASH_SetPrintfCb(printf);
  // Test 1: driver not initialized
  {
    VIFLASH_SetDebugLvl(VIFLASH_DEBUG_LVL2);
    VIFLASH_SetPrintfCb(printf);

    TEST_ASSERT_TRUE(VIFLASH_RESULT_NOTRDY == 
      VIFLASH_Ioctl(VIFLASH_CTRL_SYNC, (void*)testBuff));
  }
  // Initialize driver
  {
    size_t startDiskAddress = (size_t)testDisk;
    size_t endDiskAddress = (size_t)testDisk+DISK_SIZE;
    uint32_t ffSectorSize = FFSECTOR_SIZE;

    TEST_ASSERT_TRUE(VIFLASH_InitDriver(
      FAKE_Program, FAKE_Unlock, FAKE_Lock, FAKE_EraseSector, 
      FAKE_SectorToAddress, FAKE_AddressToSector, FAKE_SectorSize,
       startDiskAddress, endDiskAddress, ffSectorSize));

    VIFLASH_SetDebugLvl(VIFLASH_DEBUG_LVL2);
    VIFLASH_SetPrintfCb(printf);
  }
  // Test 2: buffer pointer equal to null
  {
    TEST_ASSERT_TRUE(VIFLASH_RESULT_OK == 
      VIFLASH_Ioctl(VIFLASH_CTRL_SYNC, NULL));
  }
  // Test 3: unknown cmd
  {
    TEST_ASSERT_TRUE(VIFLASH_RESULT_PARERR == 
      VIFLASH_Ioctl(0xFF, (void*)testBuff));
  }
  // Test 4: VIFLASH_GET_SECTOR_COUNT
  {
    TEST_ASSERT_TRUE(VIFLASH_RESULT_OK == 
      VIFLASH_Ioctl(VIFLASH_GET_SECTOR_COUNT, (void*)testBuff));
    TEST_ASSERT_EQUAL_UINT32(DISK_SIZE/FFSECTOR_SIZE, (uint32_t)(*testBuff));
  }
  // Test 5: VIFLASH_GET_SECTOR_SIZE
  {
    TEST_ASSERT_TRUE(VIFLASH_RESULT_OK == 
      VIFLASH_Ioctl(VIFLASH_GET_SECTOR_SIZE, (void*)testBuff));
    TEST_ASSERT_EQUAL_UINT32(FFSECTOR_SIZE, (uint32_t)(*testBuff));
  }
  // Test 6: VIFLASH_GET_BLOCK_SIZE
  {
    TEST_ASSERT_TRUE(VIFLASH_RESULT_OK == 
      VIFLASH_Ioctl(VIFLASH_GET_BLOCK_SIZE, (void*)testBuff));
    TEST_ASSERT_EQUAL_UINT32(DISK_SECTOR_SIZE/FFSECTOR_SIZE, (uint32_t)(*testBuff));
  }
  // Test 7: VIFLASH_CTRL_SYNC
  {
    TEST_ASSERT_TRUE(VIFLASH_RESULT_OK == 
      VIFLASH_Ioctl(VIFLASH_CTRL_SYNC, (void*)testBuff));
  }
  // Test 8: VIFLASH_CTRL_TRIM
  {
    TEST_ASSERT_TRUE(VIFLASH_RESULT_OK == 
      VIFLASH_Ioctl(VIFLASH_CTRL_TRIM, (void*)testBuff));
  }
}

// ===================================================================================
// Test VIFLASH_Write ================================================================
TEST(TST_VIFLASHDRV, VIFLASH_Write) {
  // Test 1: driver not initialized
  {
    VIFLASH_SetDebugLvl(VIFLASH_DEBUG_LVL2);
    VIFLASH_SetPrintfCb(printf);
    TEST_ASSERT_TRUE(VIFLASH_RESULT_NOTRDY == 
      VIFLASH_Write(testBuff, 0, 1));
  }
  // Initialize driver
  {
    size_t startDiskAddress = (size_t)testDisk;
    size_t endDiskAddress = (size_t)testDisk+DISK_SIZE;
    uint32_t ffSectorSize = FFSECTOR_SIZE;

    TEST_ASSERT_TRUE(VIFLASH_InitDriver(
      FAKE_Program, FAKE_Unlock, FAKE_Lock, FAKE_EraseSector, 
      FAKE_SectorToAddress, FAKE_AddressToSector, FAKE_SectorSize,
      startDiskAddress, endDiskAddress, ffSectorSize));

    VIFLASH_SetPrintfCb(printf);
    VIFLASH_SetDebugLvl(VIFLASH_DEBUG_LVL2);
  }
  // Test 2: wrong sector number
  {
    TEST_ASSERT_TRUE(VIFLASH_RESULT_PARERR == 
      VIFLASH_Write(testBuff, DISK_SIZE/FFSECTOR_SIZE, 1));
  }
  // Test 3: write full disc one ffsector at a iteration
  {
    for(uint32_t i = 0; i < DISK_SIZE/FFSECTOR_SIZE; i++) {
      for(uint32_t j = 0; j < FFSECTOR_SIZE; j++) {
        testBuff[j] = i*10+j;
      }
      TEST_ASSERT_TRUE(VIFLASH_RESULT_OK == 
        VIFLASH_Write(testBuff, i, 1));
      for(uint32_t j = 0; j < FFSECTOR_SIZE; j++) {
        TEST_ASSERT_EQUAL_UINT32(i*10+j, testDisk[i*FFSECTOR_SIZE + j]);
      }
    }
    TEST_ASSERT_EQUAL_UINT32(32, calledProgramCounter);
    TEST_ASSERT_EQUAL_UINT32(8, calledUnlockCounter);
    TEST_ASSERT_EQUAL_UINT32(8, calledLockCounter);
    TEST_ASSERT_EQUAL_UINT32(0, calledEraseCounter);
    calledProgramCounter = 0;
    calledUnlockCounter = 0;
    calledLockCounter = 0;
    calledEraseCounter = 0;
  }
  // Test 4: rewrite full disc two ffsectors at a iteration
  {
    VIFLASH_SetDebugLvl(VIFLASH_DEBUG_INFO);
    for(uint32_t i = 0; i < DISK_SIZE/FFSECTOR_SIZE/2; i++) {
      for(uint32_t j = 0; j < FFSECTOR_SIZE*2; j++) {
        testBuff[j] = i*10+j;
      }
      TEST_ASSERT_TRUE(VIFLASH_RESULT_OK == 
        VIFLASH_Write(testBuff, i*2, 2));
      for(uint32_t j = 0; j < FFSECTOR_SIZE*2; j++) {
        TEST_ASSERT_EQUAL_UINT32(i*10+j, testDisk[i*FFSECTOR_SIZE*2 + j]);
      }
    }
    TEST_ASSERT_EQUAL_UINT32(32, calledProgramCounter);
    TEST_ASSERT_EQUAL_UINT32(4, calledUnlockCounter);
    TEST_ASSERT_EQUAL_UINT32(4, calledLockCounter);
    TEST_ASSERT_EQUAL_UINT32(4, calledEraseCounter);
    calledProgramCounter = 0;
    calledUnlockCounter = 0;
    calledLockCounter = 0;
    calledEraseCounter = 0;
  }
  // Test 5: rewrite full disc four ffsectors at a iteration
  {
    VIFLASH_SetDebugLvl(VIFLASH_DEBUG_LVL2);
    for(uint32_t i = 0; i < DISK_SIZE/FFSECTOR_SIZE/4; i++) {
      for(uint32_t j = 0; j < FFSECTOR_SIZE*4; j++) {
        testBuff[j] = i*10+j;
      }
      TEST_ASSERT_TRUE(VIFLASH_RESULT_OK == 
        VIFLASH_Write(testBuff, i*4, 4));
      for(uint32_t j = 0; j < FFSECTOR_SIZE*4; j++) {
        TEST_ASSERT_EQUAL_UINT32(i*10+j, testDisk[i*FFSECTOR_SIZE*4 + j]);
      }
    }
    TEST_ASSERT_EQUAL_UINT32(24, calledProgramCounter);
    TEST_ASSERT_EQUAL_UINT32(4, calledUnlockCounter);
    TEST_ASSERT_EQUAL_UINT32(4, calledLockCounter);
    TEST_ASSERT_EQUAL_UINT32(3, calledEraseCounter);
    calledProgramCounter = 0;
    calledUnlockCounter = 0;
    calledLockCounter = 0;
    calledEraseCounter = 0;
  }
  // Test 6: rewrite full disc at a iteration
  {
    for(uint32_t i = 0; i < DISK_SIZE/FFSECTOR_SIZE/8; i++) {
      for(uint32_t j = 0; j < FFSECTOR_SIZE*8; j++) {
        testBuff[j] = i*10+j;
      }
      TEST_ASSERT_TRUE(VIFLASH_RESULT_OK == 
        VIFLASH_Write(testBuff, i*8, 8));
      for(uint32_t j = 0; j < FFSECTOR_SIZE*8; j++) {
        TEST_ASSERT_EQUAL_UINT32(i*10+j, testDisk[i*FFSECTOR_SIZE*8 + j]);
      }
    }
    TEST_ASSERT_EQUAL_UINT32(16, calledProgramCounter);
    TEST_ASSERT_EQUAL_UINT32(4, calledUnlockCounter);
    TEST_ASSERT_EQUAL_UINT32(4, calledLockCounter);
    TEST_ASSERT_EQUAL_UINT32(2, calledEraseCounter);
    calledProgramCounter = 0;
    calledUnlockCounter = 0;
    calledLockCounter = 0;
    calledEraseCounter = 0;
  }
  // Test 7: two ff sectors on the border of disc sectors
  {
    for(uint32_t j = 0; j < FFSECTOR_SIZE*2; j++) {
      testBuff[j] = 10+j;
    }
    TEST_ASSERT_TRUE(VIFLASH_RESULT_OK == 
      VIFLASH_Write(testBuff, 1, 2));
    for(uint32_t j = 0; j < FFSECTOR_SIZE*2; j++) {
      TEST_ASSERT_EQUAL_UINT32(10+j, testDisk[FFSECTOR_SIZE + j]);
    }

    TEST_ASSERT_EQUAL_UINT32(16, calledProgramCounter);
    TEST_ASSERT_EQUAL_UINT32(2, calledUnlockCounter);
    TEST_ASSERT_EQUAL_UINT32(2, calledLockCounter);
    TEST_ASSERT_EQUAL_UINT32(2, calledEraseCounter);
    calledProgramCounter = 0;
    calledUnlockCounter = 0;
    calledLockCounter = 0;
    calledEraseCounter = 0;
  }
  // Test 8: if unlock return error
  {
    unlockReturn = VIFLASH_RESULT_ERROR;
    for(uint32_t j = 0; j < FFSECTOR_SIZE*2; j++) {
      testBuff[j] = j;
    }
    TEST_ASSERT_TRUE(VIFLASH_RESULT_ERROR == 
      VIFLASH_Write(testBuff, 3, 2));

    TEST_ASSERT_EQUAL_UINT32(0, calledProgramCounter);
    TEST_ASSERT_EQUAL_UINT32(1, calledUnlockCounter);
    TEST_ASSERT_EQUAL_UINT32(1, calledLockCounter);
    TEST_ASSERT_EQUAL_UINT32(0, calledEraseCounter);
    calledProgramCounter = 0;
    calledUnlockCounter = 0;
    calledLockCounter = 0;
    calledEraseCounter = 0;
    unlockReturn = VIFLASH_RESULT_OK;
  }
  // Test 9: if erase return error
  {
    eraseReturn = VIFLASH_RESULT_ERROR;
    TEST_ASSERT_TRUE(VIFLASH_RESULT_ERROR == 
      VIFLASH_Write(testBuff, 3, 2));
    TEST_ASSERT_EQUAL_UINT32(0, calledProgramCounter);
    TEST_ASSERT_EQUAL_UINT32(1, calledUnlockCounter);
    TEST_ASSERT_EQUAL_UINT32(1, calledLockCounter);
    TEST_ASSERT_EQUAL_UINT32(1, calledEraseCounter);
    calledProgramCounter = 0;
    calledUnlockCounter = 0;
    calledLockCounter = 0;
    calledEraseCounter = 0;
    eraseReturn = VIFLASH_RESULT_OK;
  }
  // Test 10: if program return error
  {
    programReturn = VIFLASH_RESULT_ERROR;
    TEST_ASSERT_TRUE(VIFLASH_RESULT_ERROR == 
      VIFLASH_Write(testBuff, 3, 2));
    TEST_ASSERT_EQUAL_UINT32(1, calledProgramCounter);
    TEST_ASSERT_EQUAL_UINT32(1, calledUnlockCounter);
    TEST_ASSERT_EQUAL_UINT32(1, calledLockCounter);
    TEST_ASSERT_EQUAL_UINT32(1, calledEraseCounter);
    calledProgramCounter = 0;
    calledUnlockCounter = 0;
    calledLockCounter = 0;
    calledEraseCounter = 0;
    programReturn = VIFLASH_RESULT_OK;
  }
}

// ===================================================================================
// Test VIFLASH_IsWriteProtected =====================================================
TEST(TST_VIFLASHDRV, VIFLASH_IsWriteProtected) {
  pthread_t thread1, thread2;
  
  // Initialize driver
  {
    size_t startDiskAddress = (size_t)testDisk;
    size_t endDiskAddress = (size_t)testDisk+DISK_SIZE;
    uint32_t ffSectorSize = FFSECTOR_SIZE;

    TEST_ASSERT_TRUE(VIFLASH_InitDriver(
      FAKE_Program, FAKE_Unlock, FAKE_Lock, FAKE_EraseSector, 
      FAKE_SectorToAddress, FAKE_AddressToSector, FAKE_SectorSize,
      startDiskAddress, endDiskAddress, ffSectorSize));

    VIFLASH_SetPrintfCb(printf);
    VIFLASH_SetDebugLvl(VIFLASH_DEBUG_LVL2);
  }

  for(uint32_t j = 0; j < FFSECTOR_SIZE*2; j++) {
    testBuff[j] = j;
  }

  TEST_ASSERT_EQUAL_UINT32(0, pthread_create(&thread1, NULL, &thread1Entry, NULL));
  TEST_ASSERT_EQUAL_UINT32(0, pthread_create(&thread2, NULL, &thread2Entry, NULL));

  TEST_ASSERT_EQUAL_UINT32(0, pthread_join(thread1, NULL));
  TEST_ASSERT_EQUAL_UINT32(0, pthread_join(thread2, NULL));

}

void* thread1Entry(__attribute__((unused)) void *arg) {

  VIFLASH_Result_t res = VIFLASH_RESULT_ERROR;
  do {
    res = VIFLASH_Write(testBuff, 2, 2);
    if(VIFLASH_IsWriteProtected()) {
      while(!VIFLASH_IsWriteProtected()){}
    }
  } while(res == VIFLASH_RESULT_WRPRT);

  TEST_ASSERT_TRUE(VIFLASH_RESULT_OK == res);
  pthread_exit(0);
  return 0;
}

void* thread2Entry(__attribute__((unused)) void *arg) {

  VIFLASH_Result_t res = VIFLASH_RESULT_ERROR;
  do {
    res = VIFLASH_Write(testBuff, 3, 2);
    if(VIFLASH_IsWriteProtected()) {
      while(!VIFLASH_IsWriteProtected()){}
    }
  } while(res == VIFLASH_RESULT_WRPRT);
 
  TEST_ASSERT_TRUE(VIFLASH_RESULT_OK == res);
  pthread_exit(0);
  return 0;
}

uint8_t FAKE_Program(uint32_t TypeProgram, size_t Address, uint64_t Data) {
  TEST_ASSERT_EQUAL_UINT32(2, TypeProgram);
  calledProgramCounter++;
  if(VIFLASH_RESULT_OK == programReturn)
    *(uint32_t*)(Address) = (uint32_t)Data;
  return programReturn;
}

uint8_t FAKE_Unlock(void) {
  calledUnlockCounter++;
  return unlockReturn;
}

uint8_t FAKE_Lock(void) {
  calledLockCounter++;
  return VIFLASH_RESULT_OK;
}

uint8_t FAKE_EraseSector(VIFLASH_EraseInit_t* Sector, uint32_t *SectorError) {
  TEST_ASSERT_EQUAL_UINT32(3, Sector->Banks);
  TEST_ASSERT_EQUAL_UINT32(0, Sector->TypeErase);
  TEST_ASSERT_EQUAL_UINT32(2, Sector->VoltageRange);
  calledEraseCounter++;
  if(VIFLASH_RESULT_OK == eraseReturn) {
    for(size_t i = 0; i < Sector->NbSectors*DISK_SECTOR_SIZE; i++) 
      testDisk[Sector->Sector*DISK_SECTOR_SIZE+i] = 0xFF;
    *SectorError = 0xFFFFFFFF;
  }
  return eraseReturn;
}

size_t FAKE_SectorToAddress(uint8_t Sector) {
  return (size_t)testDisk+Sector*DISK_SECTOR_SIZE;
}

int8_t FAKE_AddressToSector(size_t Address) {
  return (Address - (size_t)testDisk)/DISK_SECTOR_SIZE;
}

int32_t FAKE_SectorSize(__attribute__((unused)) uint8_t Sector) {
  return DISK_SECTOR_SIZE;
}