#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
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

int main(void) {
    init_leds();
    _delay_ms(2000);
    
    blink(1, 1);  // Старт программы
    
    DSTATUS disk_init_status = disk_initialize(0);
    if (disk_init_status != 0) {
        blink(2, 1);
    }
    
    blink(1, 2);  // SD карта инициализирована
    
    // Монтирование файловой системы
    FATFS fs;
    FRESULT mount_status = f_mount(&fs, "", 1);
    if (mount_status != FR_OK) {
        blink(2, 2);
    }
    
    blink(1, 3);  // Файловая система смонтирована
    
    // Создание и запись в файл
    FIL fil;
    UINT bw;
    const char* text = "Hello from AVR FatFs!\n";
    
    // Открываем/создаем файл
    FRESULT res = f_open(&fil, "test.txt", FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        f_mount(0, "", 0);
        blink(2, 3);
    }
    
    blink(1, 4);  // Файл открыт
    
    // Записываем данные
    res = f_write(&fil, text, strlen(text), &bw);
    if (res != FR_OK || bw != strlen(text)) {
        f_close(&fil);
        f_mount(0, "", 0);
        blink(2, 4);
    }
    
    blink(1, 5);  // Данные записаны
    
    // Закрываем файл
    f_close(&fil);
    
    // Размонтируем файловую систему
    f_mount(0, "", 0);
    
    // Успех - бесконечное быстрое мигание
    while(1) {
        blink(0, 1);
    }
    
    return 0;
}
