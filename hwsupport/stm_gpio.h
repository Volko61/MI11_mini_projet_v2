/*
 * stm_gpio.h
 *
 *  Created on: 31 mars 2021
 *      Author: bonnetst
 */

#ifndef STM_GPIO_H_
#define STM_GPIO_H_

#include <stdint.h>

typedef struct _tagGPIO {
    volatile uint32_t moder;
    volatile uint32_t otyper;
    volatile uint32_t ospeedr;
    volatile uint32_t pupdr;
    volatile uint32_t idr;
    volatile uint32_t odr;
    volatile uint32_t bsrr;
    volatile uint32_t lckr;
    volatile uint32_t afrl;
    volatile uint32_t afrh;
    volatile uint32_t reserved[246];
} gpio_t;

#define GPIO ((gpio_t *) 0x58020000)
#define GPIO_A  (0)
#define GPIO_B  (1)
#define GPIO_C  (2)
#define GPIO_D  (3)
#define GPIO_E  (4)
#define GPIO_F  (5)
#define GPIO_G  (6)
#define GPIO_H  (7)
#define GPIO_I  (8)
#define GPIO_J  (9)
#define GPIO_K  (10)

#define GPIO_MODE_INPUT (0)
#define GPIO_MODE_OUTPUT (1)
#define GPIO_MODE_AF (2)
#define GPIO_MODE_AN (3)

#define GPIO_SPEED_LOW (0)
#define GPIO_SPEED_MED (1)
#define GPIO_SPEED_HI (2)
#define GPIO_SPEED_VHI (3)

#define GPIO_PULL_NONE (0)
#define GPIO_PULL_UP (1)
#define GPIO_PULL_DN (2)
#define GPIO_PULL_EXT (4)

#define GPIO_AF0 (0)
#define GPIO_AF1 (1)
#define GPIO_AF2 (2)
#define GPIO_AF3 (3)
#define GPIO_AF4 (4)
#define GPIO_AF5 (5)
#define GPIO_AF6 (6)
#define GPIO_AF7 (7)
#define GPIO_AF8 (8)
#define GPIO_AF9 (9)
#define GPIO_AF10 (10)
#define GPIO_AF11 (11)
#define GPIO_AF12 (12)
#define GPIO_AF13 (13)
#define GPIO_AF14 (14)
#define GPIO_AF15 (15)

void gpio_enable(int gpio);

void gpio_setup(int gpio, int pin, int mode);

void gpio_set_alt(int gpio, int pin, int function);

void gpio_set_pull(int gpio, int pin, int congig);

void gpio_write(int gpio, int pin, int value);

int gpio_read(int gpio, int pin);

#endif /* STM_GPIO_H_ */
