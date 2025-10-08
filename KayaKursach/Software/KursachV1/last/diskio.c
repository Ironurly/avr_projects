#include "config.h"
#include "diskio.h"
#include <avr/io.h>
#include <util/delay.h>


#define MISO_PIN   4 // PB4
#define MOSI_PIN   3 // PB3  
#define SCK_PIN    5 // PB5
#define CS_PIN     2 // PB2
// Глобальные переменные для записи
static unsigned long current_sector = 0;
static unsigned int write_offset = 0;
static unsigned char write_buffer[512];

// Объявляем твои переменные и функции
unsigned char SD_data[512];

#define CS_LOW   PORTB &= ~(1 << CS_PIN)
#define CS_HIGH  PORTB |= (1 << CS_PIN)

#define SD_TIMEOUT_CNT      4095



/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/



// Функции SPI
void SPI_init(void) {
    // Настройка пинов SPI
    DDRB |= (1 << MOSI_PIN) | (1 << SCK_PIN) | (1 << CS_PIN);
    DDRB &= ~(1 << MISO_PIN);
    
    // Включение SPI, режим Master, частота F_CPU/128
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0); ///alarm dk why F_CPU/128
}

uint8_t SPI_put(uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1 << SPIF)));
    return SPDR;
}

// Результат выполнения команды
unsigned char SD_command(unsigned char cmd, unsigned long arg) {

    int i = 0;
    unsigned char status;
    unsigned char crc = 0xFF;

    // Перевести в активный режим
    CS_LOW;

    // Подождать ответ в течении определенного времени
    while (i < SD_TIMEOUT_CNT) { status = SPI_put(0xFF); if (status == 0xFF) break; i++; }

    // Ошибка
    if (i >= SD_TIMEOUT_CNT) { SD_error = SD_TimeoutError; return 0xff; }

    // Отсылка команды
    SPI_put(cmd | 0x40);

    // Отослать 32-х битную команду
    for (i = 24; i >= 0; i -= 8) SPI_put(arg >> i);

    // Отправка CRC
    if (cmd == SD_CMD0) crc = 0x95;  // CMD0 with arg 0
    if (cmd == SD_CMD8) crc = 0x87;  // CMD8 with arg 0x1AA

    SPI_put(crc);

    // Ожидать снятия флага BUSY
    for (i = 0; ((status = SPI_put(0xFF)) & 0x80) && i != 0xFF; i++);

    return status;
}

uint8_t SD_acmd(uint8_t cmd, uint32_t arg) {
    SD_command(SD_CMD55, 0);  // Всегда отправляем CMD55 перед ACMD
    return SD_command(cmd, arg);
}

DSTATUS disk_initialize (void) {
    uint8_t i, status = 0xFF;
    uint32_t arg;
    uint16_t timeout;

    // Инициализация переменных
    SD_error = SD_OK;
    SD_type = SD_CARD_TYPE_ERR;

    // Инициализация SPI
    SPI_init();
    
    // Отключить устройство
    CS_HIGH;

    // Подать 80 тактов для инициализации
    for (i = 0; i < 10; i++) SPI_put(0xFF);

    // Активировать устройство
    CS_LOW;

    // ШАГ 1: Сброс командой CMD0
    if (SD_command(SD_CMD0, 0) != R1_IDLE_STATE) {
        SD_error = SD_UnknownError; 
        CS_HIGH; 
        return RES_ERROR;
    }

    // ШАГ 2: Проверка версии карты командой CMD8
    if (SD_command(SD_CMD8, 0x1AA) & R1_ILLEGAL_COMMAND) {
        // Карта версии 1 (SD1)
        SD_type = SD_CARD_TYPE_SD1;
    } else {
        // ШАГ 3: Чтение ответа от CMD8
        for (i = 0; i < 4; i++) status = SPI_put(0xFF);

        if (status != 0xAA) {
            SD_error = SD_UnknownCard; 
            CS_HIGH; 
            return RES_ERROR;
        }
        // Карта версии 2 (SD2)
        SD_type = SD_CARD_TYPE_SD2;
    }

    // ШАГ 4: Инициализация карты
    arg = (SD_type == SD_CARD_TYPE_SD2) ? 0x40000000UL : 0;
    timeout = 0;
    
    do {
        status = SD_acmd(0x29, arg);
        timeout++;
        if (timeout > SD_TIMEOUT_CNT) {
            SD_error = SD_AcmdError; 
            CS_HIGH; 
            return RES_ERROR;
        }
        _delay_ms(2);
    } while (status != R1_READY_STATE);

    // ШАГ 5-7: Для SD2 карт проверяем поддержку SDHC
    if (SD_type == SD_CARD_TYPE_SD2) {
        // ШАГ 5: Отправка CMD58
        if (SD_command(SD_CMD58, 0)) {
            SD_error = SD_Unknown58CMD; 
            CS_HIGH; 
            return RES_ERROR;
        }

        // ШАГ 6: Проверка битов 6-7 для SDHC
        status = SPI_put(0xFF);
        if ((status & 0xC0) == 0xC0) {
            SD_type = SD_CARD_TYPE_SDHC;
        }

        // ШАГ 7: Игнорируем остальные 3 байта
        for (i = 0; i < 3; i++) SPI_put(0xFF);
    }

    // ШАГ 8: Деактивировать карту
    CS_HIGH;
    
    // Дополнительные такты
    SPI_put(0xFF);
    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Read Partial Sector                                                   */
/*-----------------------------------------------------------------------*/
// Писать блок 512 байт в память
// В функции SD_read замени:
char SD_read(unsigned long lba) {
    int i = 0;
    unsigned char status;

    // УБИРАЕМ проверку на SDHC - поддерживаем все типы
    // if (SD_type != SD_CARD_TYPE_SDHC)
    //    return SD_UnsupportYet;

    // Для SD1/SD2 карт используем адресацию в байтах, для SDHC - в секторах
    unsigned long address = (SD_type == SD_CARD_TYPE_SDHC) ? lba : lba * 512;

    // Отослать команду поиска блока
    if (SD_command(SD_CMD17, address)) {
        CS_HIGH; return SD_BlockSearchError;
    }

    // остальной код без изменений...
    while ((status = SPI_put(0xFF)) == 0xFF)
        if (i++ > SD_TIMEOUT_CNT) {
            CS_HIGH; return SD_TimeoutError;
        }

    if (status != 0xFE) {
        CS_HIGH; return SD_BlockSearchError;
    }

    for (i = 0; i < 512; i++) SD_data[i] = SPI_put(0xFF);

    CS_HIGH;
    return SD_OK;
}

DRESULT disk_readp (
    BYTE* buff,     /* Pointer to the destination object */
    DWORD sector,   /* Sector number (LBA) */
    UINT offset,    /* Offset in the sector */
    UINT count      /* Byte count (bit15:destination) */
) {
    unsigned int i;
    
    // Если buff == NULL, это запрос размера
    if (!buff) return RES_OK;
    
    // Читаем сектор с SD карты
    if (SD_read(sector) != 0) {
        return RES_ERROR;
    }
    
    // Копируем запрошенную часть данных
    for (i = 0; i < count; i++) {
        if (offset + i < 512) {
            buff[i] = SD_data[offset + i];
        }
    }
    
    return RES_OK;
}

/*-----------------------------------------------------------------------*/
/* Write Partial Sector                                                  */
/*-----------------------------------------------------------------------*/
// В функции SD_write тоже убираем проверку:
char SD_write(unsigned long lba) {
    int i = 0;
    unsigned char status;

    // УБИРАЕМ: if (SD_type != SD_CARD_TYPE_SDHC) return SD_UnsupportYet;

    // Для SD1/SD2 карт используем адресацию в байтах
    unsigned long address = (SD_type == SD_CARD_TYPE_SDHC) ? lba : lba * 512;

    if (SD_command(SD_CMD24, address)) {
        CS_HIGH; return SD_BlockSearchError;
    }

    // остальной код без изменений...
    SPI_put(0xFE);
    for (int i = 0; i < 512; i++) SPI_put(SD_data[i]);
    SPI_put(0xFF);
    SPI_put(0xFF);

    status = SPI_put(0xFF);
    if ((status & 0x1F) != 0x05) {
        CS_HIGH; return SD_WriteError;
    }

    while ((status = SPI_put(0xFF)) == 0xFF)
        if (i++ > SD_TIMEOUT_CNT) {
            CS_HIGH; return SD_TimeoutError;
        }

    if (SD_command(SD_CMD13, 0) || SPI_put(0xFF)) {
        CS_HIGH; return SD_WriteError;
    }

    CS_HIGH;
    return SD_OK;
}

DRESULT disk_writep (
    const BYTE* buff,   /* Pointer to the data to be written */
    DWORD sc            /* Sector number (LBA) or Number of bytes to send */
) {
    unsigned int i;
    
    if (!buff) {
        // Начало или конец операции записи
        if (sc) {
            // Начало записи - инициализируем
            current_sector = sc;
            write_offset = 0;
            for(i = 0; i < 512; i++) write_buffer[i] = 0;
        } else {
            // Конец записи - записываем оставшиеся данные
            if (write_offset > 0) {
                for(i = 0; i < 512; i++) SD_data[i] = write_buffer[i];
                if (SD_write(current_sector) != 0) {
                    return RES_ERROR;
                }
            }
        }
    } else {
        // Запись данных в буфер
        for (i = 0; i < sc; i++) {
            if (write_offset < 512) {
                write_buffer[write_offset++] = buff[i];
            }
        }
        
        // Если буфер заполнен, записываем на карту
        if (write_offset >= 512) {
            for(i = 0; i < 512; i++) SD_data[i] = write_buffer[i];
            if (SD_write(current_sector) != 0) {
                return RES_ERROR;
            }
            current_sector++;
            write_offset = 0;
            for(i = 0; i < 512; i++) write_buffer[i] = 0;
        }
    }
    
    return RES_OK;
}