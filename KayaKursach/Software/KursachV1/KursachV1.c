#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include "microSD/ff.h"
#include "microSD/diskio.h"
#include "log.h"
#include <stdio.h>
#include "display/i2c_config.h"
#include "display/ssd1306.h"

int sd_append( const char * filename, const char * text )
{
    disk_initialize(0);
    FATFS fs;
    f_mount(&fs, "", 1);

    FIL file;
    FRESULT res;
    UINT bytes_written;
    
    // Открываем файл для дозаписи (создаем если не существует)
    res = f_open( & file, filename, FA_WRITE | FA_OPEN_APPEND );
    if ( res != FR_OK ) 
    {
        blink(2, 1);
    }
    
    // Перемещаем указатель в конец файла (на всякий случай)
    f_lseek( & file, f_size( & file ) );
    
    // Записываем данные
    res = f_write( & file, text, strlen( text ), & bytes_written );
    if ( res != FR_OK || bytes_written != strlen(text) )
    {
        f_close( & file );
        blink(2, 2);
    }
    
    // Закрываем файл
    f_close( &file );

    f_mount(0, "", 0);

    return 0;  // Успех
}

int sd_write( const char * file_name, const char * text_input )
{
    disk_initialize(0);

    FATFS fs;
    f_mount(&fs, "", 1);

    FIL fil;
    UINT bw;

    FRESULT res = f_open(&fil, file_name, FA_WRITE | FA_CREATE_ALWAYS);
    
    if (res != FR_OK) 
    {
        f_mount(0, "", 0);
        return 1;
    }

    res = f_write(&fil, text_input, strlen(text_input), &bw);
    if (res != FR_OK || bw != strlen(text_input)) 
    {
        f_close(&fil);
        f_mount(0, "", 0);
        return 2;
    }
    
    // Закрываем файл
    f_close(&fil);
    
    // Размонтируем файловую систему
    f_mount(0, "", 0);

    return 0;
}

// Функция для чтения строки из файла
int sd_read(const char *filename, char *result, int result_size) {
    
    disk_initialize(0);

    FATFS fs;
    f_mount(&fs, "", 1);

    FIL file;
    FRESULT res;
    UINT bytes_read;
    
    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) {
        f_mount(0, "", 0);
        return 1;
    }
    
    res = f_read(&file, result, result_size - 1, &bytes_read);
    if (res != FR_OK) {
        f_close(&file);
        f_mount(0, "", 0);
        return 2;
    }
    
    f_close(&file);
    
    result[bytes_read] = '\0';
    f_mount(0, "", 0);
    return 0;
}
/*
int main(void) {
    init_leds();  
    const char* text = "5\n";
    blink(1, 1);  // Файл открыт
    sd_append("test1.txt", text);
    blink(1, 2);  // Файл открыт   
    // Успех - бесконечное быстрое мигание
    while(1) {
        blink(0, 3);
    }  
    return 0;
}
*/
void display () {// перенести весь мусор с main можно реализовать buffer для текста
    return;
};
int main(void) {
    _delay_ms(500);
    i2c_init();
    _delay_ms(100);
    ssd1306_init();
    _delay_ms(100);
    
    // Сначала заполним экран "мусором" для теста
    for (uint8_t page = 0; page < 8; page++) {
        ssd1306_set_cursor(page, 0);
        i2c_start(SSD1306_ADDRESS);
        i2c_write(0x40);
        for (uint8_t col = 0; col < 128; col++) {
            i2c_write(0xAA); // Паттерн 10101010
        }
        i2c_stop();
    }
    _delay_ms(2000);
    
    // Теперь очищаем
    ssd1306_clear();
    _delay_ms(1000);
    
    // Пишем текст на ВСЕХ страницах
    ssd1306_set_cursor(0, 0);
    ssd1306_test_chars();
    
    while(1) {
        _delay_ms(1000);
    }
    
    return 0;
}