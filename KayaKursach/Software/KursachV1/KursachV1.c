#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include "microSD/ff.h"
#include "microSD/diskio.h"
#include "reading.h"
#include <stdio.h>
#include "display/i2c_config.h"
#include "display/ssd1306.h"
#include "GSM/sms_reader.h"


void display ( const int row, char * string ) 
{
    ssd1306_clear();
    ssd1306_set_cursor(row, 10);
    ssd1306_print_string(string);
    return;
}

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
    }
    
    // Перемещаем указатель в конец файла (на всякий случай)
    f_lseek( & file, f_size( & file ) );
    
    // Записываем данные
    res = f_write( & file, text, strlen( text ), & bytes_written );
    if ( res != FR_OK || bytes_written != strlen(text) )
    {
        f_close( & file );
    }
    
    // Закрываем файл
    f_close( &file );

    f_mount(0, "", 0);

    return 0;  // Успех
}

// Функция для чтения конкретной строки из файла
int sd_read_line(const char *filename, char *result, int result_size, int line_number) {
    FRESULT res;
    
    res = disk_initialize(0);

    if (res != FR_OK) {
        display(2, "init_error");
        return 1;  // Ошибка открытия файла
    }
    FATFS fs;
    res = f_mount(&fs, "", 1);
    if (res != FR_OK) {
        display(6, "mount_Error");
        return 1;  // Ошибка открытия файла
    }
    FIL file;
    UINT bytes_read;
    int current_line = 0;
    int index = 0;
    char ch;
    
    // Открываем файл для чтения
    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) {
        f_mount(0, "", 0);
        display(6, "file_Error");
        return 1;  // Ошибка открытия файла
    }
    
    // Ищем нужную строку
    while (current_line < line_number) {
        res = f_read(&file, &ch, 1, &bytes_read);
        if (res != FR_OK || bytes_read == 0) {
            f_close(&file);
            f_mount(0, "", 0);
            display(6, "read_error");
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

void init()
{
    _delay_ms(100);
    init_buttons();
    _delay_ms(200);
    i2c_init();
    _delay_ms(200);
    ssd1306_init();
    _delay_ms(200);
    ssd1306_clear();
    _delay_ms(200);
    //SIM800_Init();
    return;
}

int main(void)
{
    init();


    // int page = 0;
    // char result[64];
    // char * filename = "TEST1.TXT";
    // sd_read_line(filename, result, 64, page);
    // display(4, result); 
    // while (1) {
    //     int prev_page = page;
    //     handle_buttons(&page);
    //     if (page < 0) 
    //         page = 0;
    //     if (sd_read_line(filename, result, 64, page) == 0)
    //     {
    //         if (result[0] == '\0')
    //         {
    //             page = prev_page;
    //             strcpy(result, "END OF FILE");
    //         }
    //     }
    //     else
    //     {
    //         page = prev_page;
    //         strcpy(result, "read_Error");
    //     }

    //     // Обновляем экран только если номер страницы изменился
    //     if (page != prev_page) {
    //         display(4, result);
    //     }

    //     _delay_ms(1);
    // }



    //     sd_append("test1.txt", text);
    
    // while (sd_read_line(filename, result, 64, page) == 0)
    // {
    //     if (result[0] =='\0')
    //         break;
        
    //     display(2, result);
    //     _delay_ms(1000);
    //     page++;
    // }
    // //int res = sd_read_line(filename, result, 64, 0);
    
    
    /*-----------------------------------------------------------------------*/
    /* this for SIM                                                          */
    /*-----------------------------------------------------------------------*/
    char received_command[50];
    while(1)
    {
        if (SIM800_ReadCommandMessage(received_command))
        {
            display(2, received_command);
        }
        _delay_ms(5000);
    }




    // int i = 1;
    // while(1)
    // {
    //     if(i)
    //     {
    //         ssd1306_clear();
    //         display(6, "END");
    //         i = 0;
    //     }
    //     _delay_ms(5000);
    // }

    return 0;
}