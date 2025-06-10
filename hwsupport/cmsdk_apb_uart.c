#include <stdint.h>

#include "stm_uart.h"
#include "stm32h7xx.h"

typedef struct _tagUSART {
    volatile uint32_t data;
    volatile uint32_t state;
    volatile uint32_t ctrl;
    volatile uint32_t intstatus;
    volatile uint32_t bauddiv;
} usart_t;

#define USART ((usart_t *) 0x40004000)
#define TX_ENABLE (1 << 0)
#define RX_ENABLE (1 << 1)
#define TX_FULL (1 << 0)
#define RX_FULL (1 << 1)

void usart_init(uint32_t baudrate)
{
	USART->bauddiv = SYSPR_CLK / baudrate;
	USART->ctrl = TX_ENABLE | RX_ENABLE;
}

void usart_write(char c)
{
	while ((USART->state & TX_FULL) != 0) continue;
	USART->data = (uint32_t) c;
}

int usart_read(void)
{
	while ((USART->state & RX_FULL) == 0) continue;
	return USART->data;
}
