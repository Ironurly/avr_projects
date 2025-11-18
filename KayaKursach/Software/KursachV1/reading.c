/*-----------------------------------------------------------------------*/
/* this file is to show some information while programm is working       */
/*-----------------------------------------------------------------------*/
#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include "reading.h"

void init_buttons(void) {
    DDRD &= ~((1 << BTN_INC) | (1 << BTN_DEC));
    PORTD |= (1 << BTN_INC) | (1 << BTN_DEC);
}

void handle_buttons(int* counter) {
    static uint8_t last_inc = 0; 
    static uint8_t last_dec = 0;

    uint8_t current_inc = (PIND & (1 << BTN_INC)) == 0; // 1 если нажата
    uint8_t current_dec = (PIND & (1 << BTN_DEC)) == 0;

    if (current_inc && !last_inc) {
        (*counter)++;
    }

    if (current_dec && !last_dec) {
        (*counter)--;
    }

    last_inc = current_inc;
    last_dec = current_dec;
}
