#ifndef DEBUGUART_H
#define DEBUGUART_H

#include "lpc177x_8x_uart.h"
#include "lpc177x_8x_pinsel.h"

/************************** PRIVATE DEFINTIONS *************************/
#define UART_TEST_NUM		0

#define	LPC_UART_DEBUG			(LPC_UART_TypeDef *)LPC_UART1
#define _UART_IRQ_DEBUG			UART1_IRQn
#define UART_DEBUG_IRQHander UART1_IRQHandler
	
/* buffer size definition */
#define UART_RING_nCCS_BUFSIZE 512

/* Buf mask */
#define __BUF_MASK (UART_RING_nCCS_BUFSIZE-1)
/* Check buf is full or not */
#define __BUF_IS_FULL(head, tail) ((tail&__BUF_MASK)==((head+1)&__BUF_MASK))
/* Check buf will be full in next receiving or not */
#define __BUF_WILL_FULL(head, tail) ((tail&__BUF_MASK)==((head+2)&__BUF_MASK))
/* Check buf is empty */
#define __BUF_IS_EMPTY(head, tail) ((head&__BUF_MASK)==(tail&__BUF_MASK))
/* Reset buf */
#define __BUF_RESET(bufidx)	(bufidx=0)
#define __BUF_INCR(bufidx)	(bufidx=(bufidx+1)&__BUF_MASK)



// void packageData(uint8_t* buf, nccs_frame_t* data, uint8_t order);
uint32_t UART1Received(LPC_UART_TypeDef *UARTPort, uint8_t *rxbuf, uint32_t buflen);
uint32_t UART1Send(LPC_UART_TypeDef *UARTPort, uint8_t *txbuf, uint32_t buflen);
int uart1_debug_init(void);

#endif
