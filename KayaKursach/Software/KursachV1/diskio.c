/*-----------------------------------------------------------------------*/
/* Low level disk I/O module for FatFs and SD card                       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Basic definitions of FatFs */
#include "diskio.h"		/* Declarations FatFs API */
#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#define DEV_MMC		0	/* Map SD card to physical drive 0 */
#define SD_TIMEOUT_CNT      4095

#define CS_LOW   PORTB &= ~(1 << CS_PIN)
#define CS_HIGH  PORTB |= (1 << CS_PIN)

/* SD Commands */
#define SD_CMD0     0   // Reset
#define SD_CMD8     8   // Check voltage
#define SD_CMD13    13  // Check
#define SD_CMD17    17  // Read
#define SD_CMD24    24  // Write
#define SD_CMD55    55  // ACMD
#define SD_CMD58    58  // Read OCR

/* SD Responses */
#define R1_READY_STATE     0x00
#define R1_IDLE_STATE      0x01
#define R1_ILLEGAL_COMMAND 0x04

/* SD Card Types */
#define SD_CARD_TYPE_ERR    0
#define SD_CARD_TYPE_SD1    1
#define SD_CARD_TYPE_SD2    2
#define SD_CARD_TYPE_SDHC   3

/* Global variables */
unsigned char SD_data[512];
uint8_t SD_type = SD_CARD_TYPE_ERR;

/* SPI Functions */
void SPI_init(void) {
    DDRB |= (1 << MOSI_PIN) | (1 << SCK_PIN) | (1 << CS_PIN);
    DDRB &= ~(1 << MISO_PIN);
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
}

uint8_t SPI_put(uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1 << SPIF)));
    return SPDR;
}

/* SD Card Functions */
unsigned char SD_command(unsigned char cmd, unsigned long arg) {
    int i = 0;
    unsigned char status;
    unsigned char crc = 0xFF;

    CS_LOW;

    while (i < SD_TIMEOUT_CNT) { 
        status = SPI_put(0xFF); 
        if (status == 0xFF) break; 
        i++; 
    }

    if (i >= SD_TIMEOUT_CNT) { 
        CS_HIGH; 
        return 0xFF; 
    }

    SPI_put(cmd | 0x40);
    for (i = 24; i >= 0; i -= 8) SPI_put(arg >> i);

    if (cmd == SD_CMD0) crc = 0x95;
    if (cmd == SD_CMD8) crc = 0x87;
    SPI_put(crc);

    for (i = 0; ((status = SPI_put(0xFF)) & 0x80) && i != 0xFF; i++);
    return status;
}

uint8_t SD_acmd(uint8_t cmd, uint32_t arg) {
    SD_command(SD_CMD55, 0);
    return SD_command(cmd, arg);
}

char SD_read(unsigned long lba) {
    int i = 0;
    unsigned char status;
    unsigned long address = (SD_type == SD_CARD_TYPE_SDHC) ? lba : lba * 512;

    if (SD_command(SD_CMD17, address)) {
        CS_HIGH; 
        return 1;
    }

    while ((status = SPI_put(0xFF)) == 0xFF) {
        if (i++ > SD_TIMEOUT_CNT) {
            CS_HIGH; 
            return 1;
        }
    }

    if (status != 0xFE) {
        CS_HIGH; 
        return 1;
    }

    for (i = 0; i < 512; i++) {
        SD_data[i] = SPI_put(0xFF);
    }

    CS_HIGH;
    return 0;
}

char SD_write(unsigned long lba) {
    int i = 0;
    unsigned char status;
    unsigned long address = (SD_type == SD_CARD_TYPE_SDHC) ? lba : lba * 512;

    if (SD_command(SD_CMD24, address)) {
        CS_HIGH; 
        return 1;
    }

    SPI_put(0xFE);
    for (i = 0; i < 512; i++) {
        SPI_put(SD_data[i]);
    }
    SPI_put(0xFF);
    SPI_put(0xFF);

    status = SPI_put(0xFF);
    if ((status & 0x1F) != 0x05) {
        CS_HIGH; 
        return 1;
    }

    while ((status = SPI_put(0xFF)) == 0xFF) {
        if (i++ > SD_TIMEOUT_CNT) {
            CS_HIGH; 
            return 1;
        }
    }

    if (SD_command(SD_CMD13, 0) || SPI_put(0xFF)) {
        CS_HIGH; 
        return 1;
    }

    CS_HIGH;
    return 0;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive number to identify the drive */
)
{
	if (pdrv != DEV_MMC) return STA_NOINIT;
	
	// Simple status check - you can enhance this
	return 0; // Assume drive is ready
}

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (BYTE pdrv)
{
    uint8_t i, status;
    uint32_t arg;
    uint16_t timeout;

    if (pdrv != 0) return STA_NOINIT;

    // 1. Инициализация SPI
    SPI_init();
    CS_HIGH;
    
    // 2. Подаем 80+ тактов для инициализации
    for (i = 0; i < 20; i++) SPI_put(0xFF);
    _delay_ms(10);  // Даем время на стабилизацию

    // 3. Активируем устройство
    CS_LOW;
    
    // 4. Ждем ответа от карты (должен быть 0xFF)
    timeout = 0;
    while ((status = SPI_put(0xFF)) != 0xFF) {
        if (timeout++ > 10000) {
            CS_HIGH;
            return STA_NOINIT;  // Карта не отвечает
        }
    }

    // 5. Отправляем CMD0 (reset)
    status = SD_command(SD_CMD0, 0);
    if (status != R1_IDLE_STATE) {
        CS_HIGH;
        return STA_NOINIT;  // CMD0 не сработал
    }

    // 6. Отправляем CMD8 (check voltage)
    status = SD_command(SD_CMD8, 0x1AA);
    if (status & R1_ILLEGAL_COMMAND) {
        SD_type = SD_CARD_TYPE_SD1;
    } else {
        // Читаем ответ CMD8
        for (i = 0; i < 4; i++) status = SPI_put(0xFF);
        if (status != 0xAA) {
            CS_HIGH;
            return STA_NOINIT;
        }
        SD_type = SD_CARD_TYPE_SD2;
    }

    // 7. Инициализация ACMD41
    arg = (SD_type == SD_CARD_TYPE_SD2) ? 0x40000000UL : 0;
    timeout = 0;
    do {
        status = SD_acmd(0x29, arg);  // ACMD41
        if (timeout++ > 500) {  // Увеличим таймаут
            CS_HIGH;
            return STA_NOINIT;
        }
        _delay_ms(10);
    } while (status != R1_READY_STATE);

    // 8. Для SD2 проверяем SDHC
    if (SD_type == SD_CARD_TYPE_SD2) {
        if (SD_command(SD_CMD58, 0) == 0) {
            status = SPI_put(0xFF);
            if ((status & 0xC0) == 0xC0) {
                SD_type = SD_CARD_TYPE_SDHC;
            }
            for (i = 0; i < 3; i++) SPI_put(0xFF);
        }
    }

    CS_HIGH;
    SPI_put(0xFF);
    
    return 0;  // Успех
}
/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive number to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	UINT i;
	
	if (pdrv != DEV_MMC) return RES_PARERR;
	if (!buff) return RES_PARERR;

	for (i = 0; i < count; i++) {
		if (SD_read(sector + i) != 0) {
			return RES_ERROR;
		}
		// Copy data from SD_data to buffer
		memcpy(buff + (i * 512), SD_data, 512);
	}

	return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive number to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	UINT i;
	
	if (pdrv != DEV_MMC) return RES_PARERR;
	if (!buff) return RES_PARERR;

	for (i = 0; i < count; i++) {
		// Copy data to SD_data
		memcpy(SD_data, buff + (i * 512), 512);
		if (SD_write(sector + i) != 0) {
			return RES_ERROR;
		}
	}

	return RES_OK;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive number (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	if (pdrv != DEV_MMC) return RES_PARERR;

	switch (cmd) {
		case CTRL_SYNC:
			// Make sure that no pending write process
			return RES_OK;
			
		case GET_SECTOR_COUNT:
			// Return maximum sector number - you may want to implement proper detection
			*(LBA_t*)buff = 1000000; // Example value
			return RES_OK;
			
		case GET_SECTOR_SIZE:
			// Return sector size
			*(WORD*)buff = 512;
			return RES_OK;
			
		case GET_BLOCK_SIZE:
			// Return erase block size
			*(DWORD*)buff = 1;
			return RES_OK;
			
		default:
			return RES_PARERR;
	}
}
DWORD get_fattime (void)
{
    /* Returns current time packed into a DWORD variable */
    /* Format: bit31:25 - Year from 1980 (0..127) */
    /*         bit24:21 - Month (1..12) */
    /*         bit20:16 - Day (1..31) */
    /*         bit15:11 - Hour (0..23) */
    /*         bit10:5  - Minute (0..59) */
    /*         bit4:0   - Second/2 (0..29) */
    
    return ((DWORD)(2025 - 1980) << 25)  /* Year 2025 */
         | ((DWORD)9 << 21)              /* Month September */
         | ((DWORD)30 << 16)             /* Day 30 */
         | ((DWORD)0 << 11)              /* Hour 0 */
         | ((DWORD)0 << 5)               /* Minute 0 */
         | ((DWORD)0 >> 1);              /* Second 0 */
}