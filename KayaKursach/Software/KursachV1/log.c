/*-----------------------------------------------------------------------*/
/* this file is to show some information while programm is working       */
/*-----------------------------------------------------------------------*/
#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"


void init_leds(void) {
    DDRD |= (1 << LED_Stat) | (1 << LED_Err) | (1 << LED_OK);
    PORTD &= ~((1 << LED_Stat) | (1 << LED_Err) | (1 << LED_OK));
}

void blink(int type, int amount){
    switch (type)
    {
    case 0: // OK
        for (int i = 0; i < amount; i++){
            PORTD |= (1 << LED_OK);
            _delay_ms(200);
            PORTD &= ~(1 << LED_OK);
            _delay_ms(200);
        }
        _delay_ms(1000);
        break;
    case 1: // STAT
        for (int i = 0; i < amount; i++){
            PORTD |= (1 << LED_Stat);
            _delay_ms(200);
            PORTD &= ~(1 << LED_Stat);
            _delay_ms(200);
            
        }
        _delay_ms(1000);
        break;
    case 2: // ERROR
        for (int i = 0; i < amount; i++){
            PORTD |= (1 << LED_Err);
            _delay_ms(500);
            PORTD &= ~(1 << LED_Err);
            _delay_ms(500);
        }
        _delay_ms(1000);
        break;
    default:
        break;
    }
}