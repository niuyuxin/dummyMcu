/**
* @filename: uart2_nCCS.h
* @brief: nCCS 协议配置文件
*/
#ifndef UART2_NCCS_H
#define UART2_NCCS_H

#include "lpc177x_8x_uart.h"
#include "lpc177x_8x_pinsel.h"

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


/************************** PRIVATE TYPES *************************/
/** @brief UART Ring buffer structure */
typedef struct
{
    __IO uint32_t tx_head;                /*!< UART Tx ring buffer head index */
    __IO uint32_t tx_tail;                /*!< UART Tx ring buffer tail index */
    __IO uint32_t rx_head;                /*!< UART Rx ring buffer head index */
    __IO uint32_t rx_tail;                /*!< UART Rx ring buffer tail index */
    __IO uint8_t  tx[UART_RING_nCCS_BUFSIZE];  /*!< UART Tx data ring buffer */
    __IO uint8_t  rx[UART_RING_nCCS_BUFSIZE];  /*!< UART Rx data ring buffer */
} UART_RING_BUFFER_T;

#pragma pack(1)
typedef struct {
	uint8_t head;
	uint8_t addr;
	uint8_t order;
	uint8_t size;
	uint8_t* data;
	uint16_t crc16le;
	uint8_t tail;
} nccs_frame_t;
#pragma pack()

void packageData(uint8_t* buf, nccs_frame_t* data, uint8_t order);
uint32_t UART2Received(LPC_UART_TypeDef *UARTPort, uint8_t *rxbuf, uint32_t buflen);
uint32_t UART2Send(LPC_UART_TypeDef *UARTPort, uint8_t *txbuf, uint32_t buflen);
int uart2_nCCS_init(void);

#endif

/* **************************** end of file *************************** */
