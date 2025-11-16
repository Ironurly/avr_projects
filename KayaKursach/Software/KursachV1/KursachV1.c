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

// Функция для чтения конкретной строки из файла
int sd_read_line(const char *filename, char *result, int result_size, int line_number) {
    disk_initialize(0);
    FATFS fs;
    f_mount(&fs, "", 1);

    FIL file;
    FRESULT res;
    UINT bytes_read;
    int current_line = 0;
    int index = 0;
    char ch;
    
    // Открываем файл для чтения
    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) {
        f_mount(0, "", 0);
        return 1;  // Ошибка открытия файла
    }
    
    // Ищем нужную строку
    while (current_line < line_number) {
        res = f_read(&file, &ch, 1, &bytes_read);
        if (res != FR_OK || bytes_read == 0) {
            f_close(&file);
            f_mount(0, "", 0);
            return 2;  // Файл закончился до достижения нужной строки
        }
        
        if (ch == '\n') {
            current_line++;
        }
    }
    
    // Читаем строку до конца строки или предела буфера
    while (index < result_size - 1) {
        res = f_read(&file, &ch, 1, &bytes_read);
        if (res != FR_OK || bytes_read == 0) {
            break;  // Конец файла
        }
        
        if (ch == '\n' || ch == '\r') {
            break;  // Конец строки
        }
        
        result[index++] = ch;
    }
    
    // Завершаем строку
    result[index] = '\0';
    
    f_close(&file);
    f_mount(0, "", 0);
    return 0;  // Успех
}

void display ( char * string ) 
{
    ssd1306_set_cursor(2, 10);
    ssd1306_print_string(string);
    return;
}

void init()
{
    _delay_ms(100);
    init_leds();
    _delay_ms(20);
    i2c_init();
    _delay_ms(20);
    ssd1306_init();
    _delay_ms(20);
    ssd1306_clear();
    _delay_ms(20);
    return;
}

void exit_all()
{
    _delay_ms(20);
    while(1) 
    {
        blink(0, 3);
    }
    return;
}

int main(void)
{
    init();
    //     sd_append("test1.txt", text);
    int line = 0; // Потом доработать с кнопками аля +1 -1;
    char result[64]; // 64 от головы взял
    char * filename = "test1.txt"; // если rtc то потом надо головой думать
    
    blink(1, 1);
    while (sd_read_line(filename, result, 64, line) == 0)
    {
        if (result[0] =='\0')
            break;
        
        ssd1306_clear();
        display(result);
        _delay_ms(1000);
        line++;
    }
    //int res = sd_read_line(filename, result, 64, 0);

    blink(1, 2);

    exit_all();
    return 0;
}