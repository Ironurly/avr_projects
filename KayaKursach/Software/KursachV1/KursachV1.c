#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include "ff.h"
#include "diskio.h"
#include "log.h"

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
    
    // Открываем файл для чтения
    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) {
        f_mount(0, "", 0);
        return 1;  // Ошибка открытия файла
    }
    
    // Читаем данные из файла в переданный буфер
    res = f_read(&file, result, result_size - 1, &bytes_read);
    if (res != FR_OK) {
        f_close(&file);
        f_mount(0, "", 0);
        return 2;  // Ошибка чтения
    }
    
    // Закрываем файл
    f_close(&file);
    
    // Добавляем нуль-терминатор для создания строки
    result[bytes_read] = '\0';
    f_mount(0, "", 0);
    return 0;
}

int main(void) {
    init_leds();
    
    const char* text = "5\n";
    //char result[32];  // Используем массив вместо динамического выделения
    //int error_code;
    
    // Открываем/создаем файл
    blink(1, 1);  // Файл открыт
    sd_append("test1.txt", text);
    blink(1, 2);  // Файл открыт
    
    // Читаем из файла
    // error_code = read_string_from_file("test1.txt", result, sizeof(result));
    
    // if (error_code == 0) {
    //     // Преобразуем строку в число
    //     int amount = atoi(result);
    //     blink(2, amount);
    // } else {
    //     // Обработка ошибки
    //     blink(2, error_code);  // Мигаем кодом ошибки
    // }
    
    // Успех - бесконечное быстрое мигание
    while(1) {
        blink(0, 3);
    }
    
    return 0;
}
