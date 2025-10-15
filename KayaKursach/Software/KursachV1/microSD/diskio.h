/*-----------------------------------------------------------------------/
/  Low level disk interface module include file   (C)ChaN, 2025          /
/-----------------------------------------------------------------------*/

#ifndef _DISKIO_DEFINED
#define _DISKIO_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#include "ff.h"

/* Status of Disk Functions */
typedef BYTE	DSTATUS;

/* SD Card Commands */
enum SD_Commands {
    SD_CMD0     = 0,    /* Reset */
    SD_CMD8     = 8,    /* Check voltage SD2 */
    SD_CMD13    = 13,   /* Check */
    SD_CMD17    = 17,   /* Read */
    SD_CMD24    = 24,   /* Write */
    SD_CMD55    = 55,   /* ACMD */
    SD_CMD58    = 58    /* Read OCR register */
};

/* Responses from SD */
enum SD_results {
    R1_READY_STATE      = 0x00,
    R1_IDLE_STATE       = 0x01,
    R1_ILLEGAL_COMMAND  = 0x04
};

/* SD Card Types */
enum SD_types {
    SD_CARD_TYPE_ERR    = 0,
    SD_CARD_TYPE_SD1    = 1,
    SD_CARD_TYPE_SD2    = 2,
    SD_CARD_TYPE_SDHC   = 3
};

/* Error types */
enum SD_errors {
    SD_OK               = 0,
    SD_UnknownError     = 1,
    SD_TimeoutError     = 2,
    SD_UnknownCard      = 3,
    SD_AcmdError        = 4,
    SD_Unknown58CMD     = 5,
    SD_BlockSearchError = 6,
    SD_UnsupportYet     = 7,
    SD_WriteError       = 8
};

/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;

/* Global variables - declare as extern in header, define in .c file */
extern uint8_t SD_error;
extern uint8_t SD_type;
extern unsigned char SD_data[512];

/*---------------------------------------*/
/* Prototypes for disk control functions */

DSTATUS disk_initialize (BYTE pdrv);
DSTATUS disk_status (BYTE pdrv);
DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count);
DRESULT disk_write (BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count);
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff);

/* Additional SD card specific functions */
void SPI_init(void);
uint8_t SPI_put(uint8_t data);
unsigned char SD_command(unsigned char cmd, unsigned long arg);
uint8_t SD_acmd(uint8_t cmd, uint32_t arg);
char SD_read(unsigned long lba);
char SD_write(unsigned long lba);

/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */


/* Command code for disk_ioctrl function */

/* Generic command (Used by FatFs) */
#define CTRL_SYNC			0	/* Complete pending write process (needed at FF_FS_READONLY == 0) */
#define GET_SECTOR_COUNT	1	/* Get media size (needed at FF_USE_MKFS == 1) */
#define GET_SECTOR_SIZE		2	/* Get sector size (needed at FF_MAX_SS != FF_MIN_SS) */
#define GET_BLOCK_SIZE		3	/* Get erase block size (needed at FF_USE_MKFS == 1) */
#define CTRL_TRIM			4	/* Inform device that the data on the block of sectors is no longer used (needed at FF_USE_TRIM == 1) */

/* Generic command (Not used by FatFs) */
#define CTRL_POWER			5	/* Get/Set power status */
#define CTRL_LOCK			6	/* Lock/Unlock media removal */
#define CTRL_EJECT			7	/* Eject media */
#define CTRL_FORMAT			8	/* Create physical format on the media */

/* MMC/SDC specific ioctl command (Not used by FatFs) */
#define MMC_GET_TYPE		10	/* Get card type */
#define MMC_GET_CSD			11	/* Get CSD */
#define MMC_GET_CID			12	/* Get CID */
#define MMC_GET_OCR			13	/* Get OCR */
#define MMC_GET_SDSTAT		14	/* Get SD status */
#define ISDIO_READ			55	/* Read data form SD iSDIO register */
#define ISDIO_WRITE			56	/* Write data to SD iSDIO register */
#define ISDIO_MRITE			57	/* Masked write data to SD iSDIO register */

/* ATA/CF specific ioctl command (Not used by FatFs) */
#define ATA_GET_REV			20	/* Get F/W revision */
#define ATA_GET_MODEL		21	/* Get model name */
#define ATA_GET_SN			22	/* Get serial number */

/* Pin definitions for SD card */
#define MISO_PIN   4 /* PB4 */
#define MOSI_PIN   3 /* PB3 */  
#define SCK_PIN    5 /* PB5 */
#define CS_PIN     2 /* PB2 */

#define CS_LOW     PORTB &= ~(1 << CS_PIN)
#define CS_HIGH    PORTB |= (1 << CS_PIN)

#define SD_TIMEOUT_CNT      4095

#ifdef __cplusplus
}
#endif

#endif /* _DISKIO_DEFINED */