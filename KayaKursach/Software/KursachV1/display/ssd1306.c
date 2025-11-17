#include "ssd1306.h"
#include "i2c_config.h"
#include <avr/pgmspace.h>
#include <util/delay.h>

const uint8_t font_5x8[] PROGMEM = {
    // Space (0x20)
    0x00, 0x00, 0x00, 0x00, 0x00,
    
    // Цифры 0-9 (0x30-0x39)
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
    0x00, 0x42, 0x7F, 0x40, 0x00, // 1
    0x42, 0x61, 0x51, 0x49, 0x46, // 2
    0x21, 0x41, 0x45, 0x4B, 0x31, // 3
    0x18, 0x14, 0x12, 0x7F, 0x10, // 4
    0x27, 0x45, 0x45, 0x45, 0x39, // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
    0x01, 0x71, 0x09, 0x05, 0x03, // 7
    0x36, 0x49, 0x49, 0x49, 0x36, // 8
    0x06, 0x49, 0x49, 0x29, 0x1E, // 9
    
    // A-Z (0x41-0x5A)
    0x7C, 0x12, 0x11, 0x12, 0x7C, // A
    0x7F, 0x49, 0x49, 0x49, 0x36, // B
    0x3E, 0x41, 0x41, 0x41, 0x22, // C
    0x7F, 0x41, 0x41, 0x22, 0x1C, // D
    0x7F, 0x49, 0x49, 0x49, 0x41, // E
    0x7F, 0x09, 0x09, 0x09, 0x01, // F
    0x3E, 0x41, 0x49, 0x49, 0x7A, // G
    0x7F, 0x08, 0x08, 0x08, 0x7F, // H
    0x00, 0x41, 0x7F, 0x41, 0x00, // I
    0x20, 0x40, 0x41, 0x3F, 0x01, // J
    0x7F, 0x08, 0x14, 0x22, 0x41, // K
    0x7F, 0x40, 0x40, 0x40, 0x40, // L
    0x7F, 0x02, 0x0C, 0x02, 0x7F, // M
    0x7F, 0x04, 0x08, 0x10, 0x7F, // N
    0x3E, 0x41, 0x41, 0x41, 0x3E, // O
    0x7F, 0x09, 0x09, 0x09, 0x06, // P
    0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
    0x7F, 0x09, 0x19, 0x29, 0x46, // R
    0x46, 0x49, 0x49, 0x49, 0x31, // S
    0x01, 0x01, 0x7F, 0x01, 0x01, // T
    0x3F, 0x40, 0x40, 0x40, 0x3F, // U
    0x1F, 0x20, 0x40, 0x20, 0x1F, // V
    0x3F, 0x40, 0x38, 0x40, 0x3F, // W
    0x63, 0x14, 0x08, 0x14, 0x63, // X
    0x07, 0x08, 0x70, 0x08, 0x07, // Y
    0x61, 0x51, 0x49, 0x45, 0x43,  // Z

    // Дополнительные символы для GSM ответов
    0x00, 0x00, 0x5F, 0x00, 0x00, // ! 
    0x00, 0x07, 0x00, 0x07, 0x00, // "
    0x14, 0x7F, 0x14, 0x7F, 0x14, // #
    0x24, 0x2A, 0x7F, 0x2A, 0x12, // $
    0x23, 0x13, 0x08, 0x64, 0x62, // %
    0x36, 0x49, 0x55, 0x22, 0x50, // &
    0x00, 0x05, 0x03, 0x00, 0x00, // '
    0x00, 0x1C, 0x22, 0x41, 0x00, // (
    0x00, 0x41, 0x22, 0x1C, 0x00, // )
    0x14, 0x08, 0x3E, 0x08, 0x14, // *
    0x08, 0x08, 0x3E, 0x08, 0x08, // +
    0x00, 0x50, 0x30, 0x00, 0x00, // ,
    0x08, 0x08, 0x08, 0x08, 0x08, // -
    0x00, 0x60, 0x60, 0x00, 0x00, // .
    0x20, 0x10, 0x08, 0x04, 0x02, // /
    // ... цифры 1-9 уже есть ...
    0x00, 0x36, 0x36, 0x00, 0x00, // :
    0x00, 0x56, 0x36, 0x00, 0x00, // ;
    0x08, 0x14, 0x22, 0x41, 0x00, // <
    0x14, 0x14, 0x14, 0x14, 0x14, // =
    0x00, 0x41, 0x22, 0x14, 0x08, // >
    0x02, 0x01, 0x51, 0x09, 0x06, // ?
    0x32, 0x49, 0x79, 0x41, 0x3E, // @
    // A-Z уже есть ...
    0x7F, 0x09, 0x19, 0x29, 0x46, // [ (используем R как замену)
    0x41, 0x22, 0x14, 0x08, 0x00, // 
    0x46, 0x29, 0x19, 0x09, 0x7F, // ] (используем R отраженный)
    0x04, 0x02, 0x01, 0x02, 0x04, // ^
    0x40, 0x40, 0x40, 0x40, 0x40, // _
    0x00, 0x01, 0x02, 0x04, 0x00, // `
    // a-z (будут конвертироваться в A-Z)
    // { | } ~
    0x08, 0x3E, 0x41, 0x41, 0x00, // {
    0x00, 0x00, 0x7F, 0x00, 0x00, // |
    0x00, 0x41, 0x41, 0x3E, 0x08, // }
    0x08, 0x04, 0x08, 0x10, 0x08 // ~
};

void ssd1306_cmd(uint8_t cmd) {
    i2c_start(SSD1306_ADDRESS);
    i2c_write(0x00);
    i2c_write(cmd);
    i2c_stop();
}

void ssd1306_init(void) {
    _delay_ms(100);
    
    ssd1306_cmd(SSD1306_DISPLAYOFF);
    
    ssd1306_cmd(0x20);
    ssd1306_cmd(0x00);

    ssd1306_cmd(0x21);
    ssd1306_cmd(0x02);  // 4 часа делал они делают с запасом 2 пикселя, а I2C 
    ssd1306_cmd(0x81);  // фигачит подряд теперь получается с 2-129

    ssd1306_cmd(0x22);
    ssd1306_cmd(0x00);  // Начальная страница = 0
    ssd1306_cmd(0x07);  // Конечная страница = 7

    // Сбрасываем смещение
    ssd1306_cmd(0xD3);
    ssd1306_cmd(0x00);  // NO offset

    // Начальная линия = 0
    ssd1306_cmd(0x40);
    ssd1306_cmd(0xA1);  // Segment re-map normal (колонка 0 -> SEG0)
    ssd1306_cmd(0xC8);  // COM output scan normal (страница 0 -> COM0)
    ssd1306_cmd(0xA8);
    ssd1306_cmd(0x3F);

    // COM pins configuration
    ssd1306_cmd(0xDA);
    ssd1306_cmd(0x12);  // Alternative COM pin configuration, disable COM left/right remap

    // Контраст
    ssd1306_cmd(0x81);
    ssd1306_cmd(0x7F);

    // Включаем отображение из RAM
    ssd1306_cmd(0xA4);
    
    // Нормальный (не инвертированный) дисплей
    ssd1306_cmd(0xA6);

    // Настройка тактовой частоты
    ssd1306_cmd(0xD5);
    ssd1306_cmd(0x80);

    // Включаем charge pump
    ssd1306_cmd(0x8D);
    ssd1306_cmd(0x14);

    // Pre-charge period
    ssd1306_cmd(0xD9);
    ssd1306_cmd(0x22);

    // VCOMH deselect level
    ssd1306_cmd(0xDB);
    ssd1306_cmd(0x20);
    ssd1306_cmd(SSD1306_DISPLAYON);
    _delay_ms(100);
}

void ssd1306_clear(void) {
    for (uint8_t page = 0; page < 8; page++) {
        ssd1306_set_cursor(page, 0);
        
        i2c_start(SSD1306_ADDRESS);
        i2c_write(0x40);
        for (uint8_t col = 0; col < 128; col++) {
            i2c_write(0x00);
        }
        
        i2c_stop();
    }
    ssd1306_set_cursor(0, 0);
}

void ssd1306_set_cursor(uint8_t page, uint8_t column) {
    if (page > 7) page = 7;
    if (column > 127) column = 127;
    column += 2; // костыль
    ssd1306_cmd(0xB0 + page);
    ssd1306_cmd((column & 0x0F) | 0x00);
    ssd1306_cmd((column >> 4) | 0x10);
}

void ssd1306_print_char(char c) {
    uint8_t char_index;
    
    // Конвертируем нижний регистр в верхний
    if (c >= 'a' && c <= 'z') {
        c = c - 'a' + 'A';
    }
    
    // Определяем индекс символа (без умножения на 5)
    if (c >= '0' && c <= '9') {
        char_index = c - '0' + 1;
    } else if (c >= 'A' && c <= 'Z') {
        char_index = c - 'A' + 11;
    } else if (c == ' ') {
        char_index = 0;
    } else if (c == '!') {
        char_index = 36;
    } else if (c == '"') {
        char_index = 37;
    } else if (c == '#') {
        char_index = 38;
    } else if (c == '$') {
        char_index = 39;
    } else if (c == '%') {
        char_index = 40;
    } else if (c == '&') {
        char_index = 41;
    } else if (c == '\'') {
        char_index = 42;
    } else if (c == '(') {
        char_index = 43;
    } else if (c == ')') {
        char_index = 44;
    } else if (c == '*') {
        char_index = 45;
    } else if (c == '+') {
        char_index = 46;
    } else if (c == ',') {
        char_index = 47;
    } else if (c == '-') {
        char_index = 48;
    } else if (c == '.') {
        char_index = 49;
    } else if (c == '/') {
        char_index = 50;
    } else if (c == ':') {
        char_index = 51;
    } else if (c == ';') {
        char_index = 52;
    } else if (c == '<') {
        char_index = 53;
    } else if (c == '=') {
        char_index = 54;
    } else if (c == '>') {
        char_index = 55;
    } else if (c == '?') {
        char_index = 56;
    } else if (c == '@') {
        char_index = 57;
    } else if (c == '[') {
        char_index = 58;
    } else if (c == '\\') {
        char_index = 59;
    } else if (c == ']') {
        char_index = 60;
    } else if (c == '^') {
        char_index = 61;
    } else if (c == '_') {
        char_index = 62;
    } else if (c == '`') {
        char_index = 63;
    } else if (c == '{') {
        char_index = 64;
    } else if (c == '|') {
        char_index = 65;
    } else if (c == '}') {
        char_index = 66;
    } else if (c == '~') {
        char_index = 67;
    } else {
        char_index = 1;  // Для неизвестных символов
    }
    
    // Умножаем на 5 только здесь
    uint16_t font_index = char_index * 5;
    
    i2c_start(SSD1306_ADDRESS);
    i2c_write(0x40);
    
    for (uint8_t i = 0; i < 5; i++) {
        i2c_write(pgm_read_byte(&font_5x8[font_index + i]));
    }
    
    i2c_write(0x00);
    i2c_stop();
}

void ssd1306_print_string(const char* str) {
    while (*str) {
        ssd1306_print_char(*str++);
    }
}

void ssd1306_test_chars(void) {
    ssd1306_clear();
    
    ssd1306_set_cursor(0, 0);
    ssd1306_print_string("0123456789");
    
    ssd1306_set_cursor(2, 0);
    ssd1306_print_string("ABCDEFGHIJKLM");
    
    ssd1306_set_cursor(4, 0);
    ssd1306_print_string("NOPQRSTUVWXYZ");
    
    ssd1306_set_cursor(6, 0);
    ssd1306_print_string("TEST WY");
}