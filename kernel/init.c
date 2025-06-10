#include <stdint.h>

extern uint32_t __data;
extern uint32_t __edata;
extern uint32_t __etext;
extern uint32_t __bss;
extern uint32_t __ebss;

int main(void);

extern uint32_t __tos;

void init(void)
{
    /* Mettre bss à 0 */
    for (uint32_t* p = &__bss; p < &__ebss; ++p) {
        *p = 0UL;
    }

    /* Copier les données de la flash vers la ram */
    for (uint32_t *s = &__etext, *d = &__data; d < &__edata; ++s, ++d) {
        *d = *s;
    }

    /* Appel à main */
    main();

    /* Bloquer si main retourne */
    for(;;) continue;
}

void __attribute__((naked)) _start(void)
{
	__asm__ __volatile__(
			"mov sp,%[spvalue]\t\n"
			"bl init\t\n"
			:
			: [spvalue]"r"(&__tos));
}
