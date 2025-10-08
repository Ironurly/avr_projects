#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include "ff.h"
#include "diskio.h"

#define LED_Err    5 // PD5
#define LED_OK     6 // PD6
#define LED_Stat   7 // PD7

void init_leds(void) {
    DDRD |= (1 << LED_Stat) | (1 << LED_Err) | (1 << LED_OK);
    PORTD &= ~((1 << LED_Stat) | (1 << LED_Err) | (1 << LED_OK));
}
void blink(int type, int amount){
    switch (type)
    {
    case 0:
        for (int i = 0; i < amount; i++){
            PORTD |= (1 << LED_OK);
            _delay_ms(1000);
            PORTD &= ~(1 << LED_OK);
            _delay_ms(1000);
        }
        break;
    case 1:
        for (int i = 0; i < amount; i++){
            PORTD |= (1 << LED_Stat);
            _delay_ms(200);
            PORTD &= ~(1 << LED_Stat);
            _delay_ms(200);
            
        }
        break;
    case 2:
        for (int i = 0; i < amount; i++){
            PORTD |= (1 << LED_Err);
            _delay_ms(500);
            PORTD &= ~(1 << LED_Err);
            _delay_ms(500);
            
        }
        break;
    default:
        break;
    }
}

int write_to_file( const char * file_name, const char * text_input )
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
int read_string_from_file(const char *filename, char *result, int result_size) {
    
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
    _delay_ms(2000);

    const char* text = "10\n";
    char result[32];  // Используем массив вместо динамического выделения
    int error_code;
    
    // Открываем/создаем файл
    blink(1, 1);  // Файл открыт
    write_to_file("test1.txt", text);
    blink(1, 3);  // Файл открыт
    
    // Читаем из файла
    error_code = read_string_from_file("test1.txt", result, sizeof(result));
    
    if (error_code == 0) {
        // Преобразуем строку в число
        int amount = atoi(result);
        blink(2, amount);
    } else {
        // Обработка ошибки
        blink(2, error_code);  // Мигаем кодом ошибки
    }
    
    // Успех - бесконечное быстрое мигание
    while(1) {
        blink(0, 1);
    }
    
    return 0;
}