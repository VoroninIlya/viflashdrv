# viflashdrv

This is a simple flash driver (at the moment targetet to STM32F429ZIT6).
Driver implements next funktions to connect with FatFs library:

1. To write section **'VIFLASH_Write'**
2. To read section **'VIFLASH_Read'**
3. To control disk **'VIFLASH_Ioctl'**

4. To initialize the driver a funktion **'VIFLASH_InitDriver'** is provided 

The purpose of the driver:
1. Harmonization of internal flash with FatFs functionality
2. Optimization of working process with internal flash of controller to avoid superfluous erase/read/write operations

### Additional information:

The flash memory has a couple of inconvenient specificities: 
1. It is allowed write a byte only one time after erasing, if you need to rewrite already written byte you need first erase and then write new value.
2. Erasing is possible only by sectors, it means if you need rewrite only one byte in flash memory, you need first store the whole sector in RAM, 
then change one byte in RAM, erase whole old sector from Flash and at the end write that renewed sector again to Flash. 
(By the way STM32F429 sectors have variable size 16, 64 and 128 Kbytes)

This flash specificities bring some difficulties by development of write function. Plus, FatFs driver has its own sectors, that can by 512, 1024, 2048 or 4096 bytes,
so essential less than flash sectors. This is also to consider.
It is important to use for file system an integer of flash sectors, because it will content an integer of file system sectors.
(16kB/4kB=4, 16kB/2kB=8, 16kB/1kB=16, 16kB/0,5kB=32 etc.)

### VIFLASH_Write logic description:

#### VIFLASH_Write parameters
 + **_buff_** - pointer to array of bytes, that should be written
 + **_sector_** - start file system sector to write
 + **_count_** - number of file system sectors to write

Notes: 
  The file system driver doesn't know anything about flash sectors, it works with its own sectors. It means, the one of  main tasks of this driver to interpret file system sectors and convert it to physical flash addresses.

Next Animation schematically shows an write process as an allocate-prepare-erase-write sequence.

__Image__ __1:__ Schematically animates write process

![Image 1](https://github.com/VoroninIlya/viflashdrv/blob/develop/doc/img/anim1.gif)

#### Driver_t structure
The Driver_t is a main driver structur. After initialization it contents: 
 + **_startDiskAddress_** - physical meaning: start address of file system area (see Image 2)
 + **_endDiskAddress_** - physical meaning: end address of file system area (see Image 2)
 + **_ffSectorSize_** - file system sector size 

__Image__ __2:__ Describes **_startDiskAddress_**, **_endDiskAddress_** and schematically structure of flash

![Image 2](https://github.com/VoroninIlya/viflashdrv/blob/develop/doc/img/viflashdrv1.png)

Notes: 
1. **_startDiskAddress_** should be first address of an flash sector
2. **_endDiskAddress_** should be first address of an flash sector, as well but it doesn't belong to file system area, so next condition is met:

  ``` startDiskAddress <= fileSystemArea < endDiskAddress ```
  
#### WriteCtrl_t Structure
There is a WriteCtrl_t structur to control write process. It contents next fields:
 + **_stopFlashAddr_** - calculated flash address of the end of the last file system sector to be written
 + **_stopFlashSector_** - ordinal number of the flash sector to that belong **_stopFlashAddr_**
 + **_startFlashAddr_** - calculated flash address of the beginning of the first file system sector to be written
 + **_startFlashSector_** - ordinal number  of flash sector to that belong **_startFlashAddr_**
 + **_currentFlashAddrPtr_** - cursor in flash to current byte that is writing
 + **_currentBufferPtr_** - cursor in memory buffer to current byte that is writing
 + **_sectorBuffer_** - pointer to memory buffer, that has been allocated to store flash sector

__Image__ __3:__ Describes fields of **_WriteCtrl_t_** structure

![Image 3](https://github.com/VoroninIlya/viflashdrv/blob/develop/doc/img/viflashdrv3.png)