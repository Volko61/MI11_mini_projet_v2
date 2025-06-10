/*
 * systick.h
 *
 *  Created on: 8 avr. 2021
 *      Author: bonnetst
 */

#ifndef CORTEX_H_
#define CORTEX_H_

#include <stdint.h>

#define SCS_BASE (0xE000E000UL)
#define SYSTICK_BASE (SCS_BASE + 0x10UL)
#define NVIC_BASE (SCS_BASE + 0x100UL)
#define SCB_BASE (SCS_BASE + 0xD00UL)

#define SCB ((scb_t *) SCB_BASE)
#define SYSTICK ((systick_t *) SYSTICK_BASE)
#define NVIC ((nvic_t *) NVIC_BASE)

#define _ISB() \
    asm volatile("isb\n" \
                 "nop\n")

#define _DSB() \
    asm volatile("dsb\n" \
                 "nop\n")

typedef struct _tagSCB {
    volatile uint32_t cpuid;
    volatile uint32_t icsr;
    volatile uint32_t vtor;
    volatile uint32_t aircr;
    volatile uint32_t scr;
    volatile uint32_t ccr;
    volatile uint8_t shp[12];
    volatile uint32_t shcsr;
    volatile uint32_t cfsr;
    volatile uint32_t hfsr;
    volatile uint32_t dfsr;
    volatile uint32_t mmfar;
    volatile uint32_t bfar;
    volatile uint32_t pfr[2];
    volatile uint32_t dfr;
    volatile uint32_t afr;
    volatile uint32_t mmfr[4];
    volatile uint32_t isar[5];
    uint32_t reserved[5];
    volatile uint32_t cpacr;
} scb_t;

typedef struct _tagSYSTICK {
    volatile uint32_t ctrl;
    volatile uint32_t load;
    volatile uint32_t val;
    volatile uint32_t calib;

} systick_t;

typedef struct _tagNVIC {
    volatile uint32_t iser[8];
    uint32_t reserved0[24];
    volatile uint32_t icer[8];
    uint32_t reserved1[24];
    volatile uint32_t ispr[8];
    uint32_t reserved2[24];
    volatile uint32_t icpr[8];
    uint32_t reserved3[24];
    volatile uint32_t iabr[8];
    uint32_t reserved4[56];
    volatile uint8_t ip[240];
    uint32_t reserved5[644];
    volatile uint32_t stir;
} nvic_t;

/* Fonctions diverses */
void fpu_enable();
void pendsv_trigger();

/* Fonctions d'accès NVIC */
void nvic_irq_enable(uint8_t irq, uint8_t prio);
int nvic_irq_is_active(uint8_t irq);

/* Fonctions Systick */
void systick_start(uint32_t ticks);
void systick_irq_enable();
void systick_wait();
void systick_ack();

#endif /* CORTEX_H_ */
