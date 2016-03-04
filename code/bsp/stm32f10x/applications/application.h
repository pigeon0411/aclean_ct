
#ifndef  __APPLACATION__H
#define  __APPLACATION__H

extern void airclean_power_onoff(u8 mode);
extern u8 set_device_work_mode(u8 type,u8 data);

extern DEVICE_WORK_TYPE device_work_data_bak;


#define FlashSize_KB    (256)

#if  FlashSize_KB == 256

#define PAGE_SIZE  (0x800)
#define FlashTotalAddrEnd   (0x0807FFFF)
#define FlashStartAddr      (0x08000000)
#define ApplicationAddress  (0x08003800)
#define EraseFlashPageEnd   ((FlashStartAddr + FlashTotalSize) - 1 )
#define FlashTotalSize      (0x40000/1)
#define EraseFlashPageNum   ((ApplicationAddress - FlashStartAddr) / PAGE_SIZE)


#endif

#if  FlashSize_KB == 128

#define PAGE_SIZE  (0x400)
#define FlashTotalAddrEnd   (0x0807FFFF)
#define FlashStartAddr      (0x08000000)
#define ApplicationAddress  (0x08003000)
#define EraseFlashPageEnd   ((FlashStartAddr + FlashTotalSize) - 1 )
#define FlashTotalSize      (0x40000/2)
#define EraseFlashPageNum   ((ApplicationAddress - FlashStartAddr) / PAGE_SIZE)

#endif

#define FLASH_SIZE                        (FlashTotalSize) /* Flash¡ä¨®D? */

/* Compute the FLASH upload image size */  
#define FLASH_IMAGE_SIZE                   (u32) (FLASH_SIZE - (ApplicationAddress - 0x08000000) - 2048)


#define	FLASH_END_ADDR	(FlashStartAddr + FlashTotalSize - 1)

#define SYS_PARA_ADDR	(FLASH_END_ADDR - 128 + 1)

#define	__Read_Flash_Word(FlashAddress)	((u32)(*(vu32*)FlashAddress))

#endif


