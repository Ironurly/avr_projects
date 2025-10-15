#ifndef SSD1306_H
#define SSD1306_H

#include <avr/io.h>

// Адрес I2C
#define SSD1306_ADDRESS 0x78

#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

void ssd1306_init(void);
void ssd1306_cmd(uint8_t cmd);
void ssd1306_clear(void);
void ssd1306_set_cursor(uint8_t page, uint8_t column);
void ssd1306_print_char(char c);
void ssd1306_print_string(const char* str);

void ssd1306_test_chars(void);

#endif