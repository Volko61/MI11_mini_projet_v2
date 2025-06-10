/*
 * stm_uart.h
 *
 *  Created on: 1 avr. 2021
 *      Author: bonnetst
 */

#ifndef STM_UART_H_
#define STM_UART_H_

#include <stdint.h>

void usart_init(uint32_t baudrate);
void usart_write(char c);
int usart_read(void);

#endif /* STM_UART_H_ */
