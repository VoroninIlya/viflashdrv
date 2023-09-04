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
