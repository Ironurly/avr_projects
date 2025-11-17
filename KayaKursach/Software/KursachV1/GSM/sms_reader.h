#ifndef SMS_READER_H
#define SMS_READER_H

#include <stdbool.h>

#define SMS_BUFFER_SIZE 160

void SIM800_Init(void);
bool SIM800_ReadCommandMessage(char* message);
void UART_SendString(const char* str);

#endif