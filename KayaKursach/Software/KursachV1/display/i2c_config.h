#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include "../config.h"
#include <avr/io.h>
#include <util/twi.h>

void i2c_init(void);
uint8_t i2c_start(uint8_t address);
void i2c_stop(void);
uint8_t i2c_write(uint8_t data);

#endif