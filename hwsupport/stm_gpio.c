/*
 * stm_gpio.c
 *
 *  Created on: 1 avr. 2021
 *      Author: bonnetst
 */

#include "stm_gpio.h"
#include "stm_rcc.h"

void gpio_enable(int gpio)
{
    RCC->ahb4enr |= (1 << gpio);
    while (!(RCC->ahb4enr & (1 << gpio))) continue;
    volatile uint32_t __attribute__((unused)) dummy = GPIO[gpio].moder;
}

void gpio_setup(int gpio, int pin, int mode)
{
    GPIO[gpio].moder = (GPIO[gpio].moder & ~(3 << pin * 2)) | (mode << pin * 2);
    GPIO[gpio].otyper &= ~(1 << pin);
}

void gpio_set_alt(int gpio, int pin, int function)
{
    if (pin <= 7) {
        GPIO[gpio].afrl = (GPIO[gpio].afrl & ~(0xf << pin * 4)) | (function << pin * 4);
    } else {
        GPIO[gpio].afrh = (GPIO[gpio].afrh & ~(0xf << (pin - 8) * 4)) | (function << (pin - 8) * 4);
    }
}

void gpio_set_pull(int gpio, int pin, int config)
{
    if (config != GPIO_PULL_NONE) {
        GPIO[gpio].otyper |= (1 << pin);
    } else {
        GPIO[gpio].otyper &= ~(1 << pin);
    }

    GPIO[gpio].pupdr = (GPIO[gpio].pupdr & ~(3 << pin * 2)) | ((config & 3) << pin * 2);
}

void gpio_write(int gpio, int pin, int value)
{
    GPIO[gpio].odr = (GPIO[gpio].odr & ~(1 << pin)) | (value << pin);
}

int gpio_read(int gpio, int pin)
{
    return (GPIO[gpio].idr >> pin) & 1;
}
