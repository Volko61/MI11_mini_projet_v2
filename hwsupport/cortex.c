/*
 * cortex.c
 *
 *  Created on: 8 avr. 2021
 *      Author: bonnetst
 */

#include <stdint.h>

#include "cortex.h"

/* Fonctions diverses */
void fpu_enable()
{
    SCB->cpacr |= (1 << 20U);
    _ISB();
    _DSB();
}

void pendsv_trigger()
{
    SCB->icsr |= 1 << 28U;
}

/* Fonctions d'accès NVIC */
void nvic_irq_enable(uint8_t irq, uint8_t prio)
{
    NVIC->iser[irq >> 5] = (1 << (irq & 0x1fU));
}

int nvic_irq_is_active(uint8_t irq)
{
    return (NVIC->iabr[irq >> 5] & (1 << (irq & 0x1fU))) != 0;
}

/* Fonctions Systick */
void systick_start(uint32_t ticks)
{
    SYSTICK->load = ticks - 1;
    SYSTICK->val = 0;
    SYSTICK->ctrl |= ((1 << 2U) | (1 << 0U));
}

void systick_irq_enable()
{
    SYSTICK->ctrl |= (1 << 1U);
}

void systick_wait()
{
    while ((SYSTICK->ctrl & (1 << 16U)) == 0) continue;
}

void systick_ack()
{
    uint32_t __attribute__((unused)) dummy = SYSTICK->ctrl & (1 << 16U);
}
