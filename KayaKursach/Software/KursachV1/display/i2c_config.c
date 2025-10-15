#include "i2c_config.h"
#include <util/delay.h>

void i2c_init(void) {
    TWSR = 0x00;
    TWBR = 0x0C;
    TWCR = (1 << TWEN);
}

uint8_t i2c_start(uint8_t address) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    
    if ((TWSR & 0xF8) != TW_START) return 1;
    
    TWDR = address;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    
    if ((TWSR & 0xF8) != TW_MT_SLA_ACK) return 1;
    
    return 0;
}

void i2c_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    while (TWCR & (1 << TWSTO));
}

uint8_t i2c_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    
    if ((TWSR & 0xF8) != TW_MT_DATA_ACK) return 1;
    
    return 0;
}