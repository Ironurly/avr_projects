#include "sms_reader.h"
#include "../config.h"
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "../log.h"
#include "../display/ssd1306.h"

char uart_buffer[SMS_BUFFER_SIZE];

void simDisplay(const int row, const char * buffer)
{
    ssd1306_clear();
    _delay_ms(20);
    ssd1306_set_cursor(row, 10);
    ssd1306_print_string(buffer);
    _delay_ms(2000);
}

void UART_SendString(const char* str) {
    while (*str) {
        while (!(UCSR0A & (1 << UDRE0)));
        UDR0 = *str++;
    }
}

char UART_Receive(void) {
    simDisplay(2,"idk im stuck");
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

void SIM800_Init(void) {
    UBRR0H = 0;
    UBRR0L = 103;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    
    _delay_ms(5000); // Увеличил до 5 сек
    
    // Проверяем связь
    UART_SendString("AT\r\n");
    _delay_ms(2000);
    
    // Ждем регистрации в сети
    UART_SendString("AT+CREG?\r\n");
    _delay_ms(3000);
    
    // Проверяем уровень сигнала
    UART_SendString("AT+CSQ\r\n");
    _delay_ms(2000);
    
    UART_SendString("AT+CMGF=1\r\n");
    _delay_ms(1000);

    UART_SendString("AT+CBC\r\n");
    _delay_ms(3000);
}
bool SIM800_ReadCommandMessage(char* message) {
    memset(uart_buffer, 0, SMS_BUFFER_SIZE);

    uint16_t j = 0;
    while (j < SMS_BUFFER_SIZE - 1) {
        uart_buffer[j] = UART_Receive();
        
        simDisplay(2, "recieving");
        if (strstr(uart_buffer, "OK\r\n") != NULL){
            simDisplay(2, "ok");
            break;
        }
        simDisplay(2, uart_buffer);
        j++;
    }

    simDisplay(2,"start sending");
    UART_SendString("AT+CMGL=\"ALL\"\r\n");
    _delay_ms(3000);
    simDisplay(2,"start reading");
    // Читаем ответ от модуля
    uint16_t i = 0;
    while (i < SMS_BUFFER_SIZE - 1) {
        uart_buffer[i] = UART_Receive();
        
        simDisplay(2, "recieving");
        if (strstr(uart_buffer, "OK\r\n") != NULL){
            simDisplay(2, "not ok");
            break;
        }
        simDisplay(2, uart_buffer);
        i++;
    }
    uart_buffer[i] = '\0';
    simDisplay(2, "Fine");
    // Ищем сообщение формата #XX:YYY
    char* msg_start = uart_buffer;
    while ((msg_start = strstr(msg_start, "\r\n#")) != NULL) {
        msg_start += 2; // Пропускаем \r\n
        simDisplay(2, "1");
        char* msg_end = strchr(msg_start, '\r');
        if (msg_end) {
            simDisplay(2, "2");
            strncpy(message, msg_start, msg_end - msg_start);
            message[msg_end - msg_start] = '\0';
            
            if (strchr(message, ':') != NULL) {
                return true;
            }
        }
    }
    
    return false;
}