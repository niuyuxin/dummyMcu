/**
* @filename: uart2_nCCS.h
* @brief: nCCS Э�������ļ�
*/
#ifndef UART2_NCCS_H
#define UART2_NCCS_H

#include "lpc177x_8x_uart.h"
#include "lpc177x_8x_pinsel.h"
#include "user_types.h"

/************************** PRIVATE DEFINTIONS *************************/
#define UART_TEST_NUM		0

#define	_LPC_UART_nCCS			(LPC_UART_TypeDef *)LPC_UART2
#define _UART_IRQ_nCCS			UART2_IRQn
#define _UART_nCCS_IRQHander		UART2_IRQHandler
	
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


void packageData(uint8_t* buf, nccs_frame_t* data, uint8_t order);
uint32_t UART2Received(LPC_UART_TypeDef *UARTPort, uint8_t *rxbuf, uint32_t buflen);
uint32_t UART2Send(LPC_UART_TypeDef *UARTPort, uint8_t *txbuf, uint32_t buflen);
int uart2_nCCS_init(void);

#endif

/* **************************** end of file *************************** */
