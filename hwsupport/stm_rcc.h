/*
 * stm_rcc.h
 *
 *  Created on: 31 mars 2021
 *      Author: bonnetst
 */

#ifndef STM_RCC_H_
#define STM_RCC_H_

#include <stdint.h>

typedef struct _tagRCC {
    volatile uint32_t cr;
    volatile uint32_t hsicfgr;
    volatile uint32_t crrcr;
    volatile uint32_t csicfgr;
    volatile uint32_t cfgr;
    volatile uint32_t reserved1;
    volatile uint32_t d1cfgr;
    volatile uint32_t d2cfgr;
    volatile uint32_t d3cfgr;
    volatile uint32_t reserved2;
    volatile uint32_t pllckselr;
    volatile uint32_t pllcfgr;
    volatile uint32_t pll1divr;
    volatile uint32_t pll1fracr;
    volatile uint32_t pll2divr;
    volatile uint32_t pll2fracr;
    volatile uint32_t pll3divr;
    volatile uint32_t pll3fracr;
    volatile uint32_t reserved3;
    volatile uint32_t d1ccipr;
    volatile uint32_t d2ccip1r;
    volatile uint32_t d2ccip2r;
    volatile uint32_t d3ccipr;
    volatile uint32_t reserved4;
    volatile uint32_t cier;
    volatile uint32_t cifr;
    volatile uint32_t cicr;
    volatile uint32_t reserved5;
    volatile uint32_t bdcr;
    volatile uint32_t csr;
    volatile uint32_t reserved6;
    volatile uint32_t ahb3rstr;
    volatile uint32_t ahb1rstr;
    volatile uint32_t ahb2rstr;
    volatile uint32_t ahb4rstr;
    volatile uint32_t apb3rstr;
    volatile uint32_t apb1lrstr;
    volatile uint32_t apb1hrstr;
    volatile uint32_t apb2rstr;
    volatile uint32_t apb4rstr;
    volatile uint32_t gcr;
    volatile uint32_t reserved7;
    volatile uint32_t d3amr;
    volatile uint32_t reserved8[9];
    volatile uint32_t rsr;
    volatile uint32_t ahb3enr;
    volatile uint32_t ahb1enr;
    volatile uint32_t ahb2enr;
    volatile uint32_t ahb4enr;
    volatile uint32_t apb3enr;
    volatile uint32_t apb1lenr;
    volatile uint32_t apb1henr;
    volatile uint32_t apb2enr;
    volatile uint32_t apb4enr;
    volatile uint32_t reserved9;
    volatile uint32_t ahb3lpenr;
    volatile uint32_t ahb1lpenr;
    volatile uint32_t ahb2lpenr;
    volatile uint32_t ahb4lpenr;
    volatile uint32_t apb3lpenr;
    volatile uint32_t apb1llpenr;
    volatile uint32_t apb1hlpenr;
    volatile uint32_t apb2lpenr;
    volatile uint32_t apb4lpenr;
    volatile uint32_t reserved10[4];
    volatile uint32_t c1_rsr;
    volatile uint32_t c1_ahb3enr;
    volatile uint32_t c1_ahb1enr;
    volatile uint32_t c1_ahb2enr;
    volatile uint32_t c1_ahb4enr;
    volatile uint32_t c1_apb3enr;
    volatile uint32_t c1_apb1lenr;
    volatile uint32_t c1_apb1henr;
    volatile uint32_t c1_apb2enr;
    volatile uint32_t c1_apb4enr;
    volatile uint32_t reserved11;
    volatile uint32_t c1_ahb3lpenr;
    volatile uint32_t c1_ahb1lpenr;
    volatile uint32_t c1_ahb2lpenr;
    volatile uint32_t c1_ahb4lpenr;
    volatile uint32_t c1_apb3lpenr;
    volatile uint32_t c1_apb1llpenr;
    volatile uint32_t c1_apb1hlpenr;
    volatile uint32_t c1_apb2lpenr;
    volatile uint32_t c1_apb4lpenr;
    volatile uint32_t reserved12[4];
    volatile uint32_t c2_rsr;
    volatile uint32_t c2_ahb3enr;
    volatile uint32_t c2_ahb1enr;
    volatile uint32_t c2_ahb2enr;
    volatile uint32_t c2_ahb4enr;
    volatile uint32_t c2_apb3enr;
    volatile uint32_t c2_apb1lenr;
    volatile uint32_t c2_apb1henr;
    volatile uint32_t c2_apb2enr;
    volatile uint32_t c2_apb4enr;
    volatile uint32_t reserved13;
    volatile uint32_t c2_ahb3lpenr;
    volatile uint32_t c2_ahb1lpenr;
    volatile uint32_t c2_ahb2lpenr;
    volatile uint32_t c2_ahb4lpenr;
    volatile uint32_t c2_apb3lpenr;
    volatile uint32_t c2_apb1llpenr;
    volatile uint32_t c2_apb1hlpenr;
    volatile uint32_t c2_apb2lpenr;
    volatile uint32_t c2_apb4lpenr;
} rcc_t;

#define RCC ((rcc_t *) 0x58024400)

#endif
