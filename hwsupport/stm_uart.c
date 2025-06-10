/*
 * stm_uart.c
 *
 *  Created on: 1 avr. 2021
 *      Author: bonnetst
 */

#include "stm32h7xx.h"

typedef struct _tagUSART {
    volatile uint32_t cr1;
    volatile uint32_t cr2;
    volatile uint32_t cr3;
    volatile uint32_t brr;
    volatile uint32_t gtpr;
    volatile uint32_t rtor;
    volatile uint32_t rqr;
    volatile uint32_t isr;
    volatile uint32_t icr;
    volatile uint32_t rdr;
    volatile uint32_t tdr;
    volatile uint32_t presc;
} usart_t;

#define USART1 ((usart_t *) 0x40011000)

void usart_init(uint32_t baudrate)
{
    /* Passer les broches PA9 et PA10 en AF7 */
    gpio_enable(GPIO_A);
    gpio_setup(GPIO_A, 9, GPIO_MODE_AF);
    gpio_setup(GPIO_A, 10, GPIO_MODE_AF);
    gpio_set_alt(GPIO_A, 9, GPIO_AF7);
    gpio_set_alt(GPIO_A, 10, GPIO_AF7);

    /* Activer l'UART1 */
    RCC->apb2enr |= (1 << 4);
    while (!(RCC->apb2enr & (1 << 4))) continue;

    uint32_t __attribute__((unused)) dummy = USART1->cr1;

    USART1->brr = SYSPR_CLK / baudrate;
    USART1->cr2 = 0;
    USART1->cr3 = 0;

    USART1->cr1 |= ((1 << 3) | (1 << 2) | (1 << 0));
}

void usart_write(char c)
{
    while (!(USART1->isr & (1 << 7))) continue;
    USART1->tdr = c;
}

int usart_read()
{
    while (!(USART1->isr & (1 << 5))) continue;
    return USART1->rdr;
}
