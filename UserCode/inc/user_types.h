#ifndef USER_TYPES_H
#define USER_TYPES_H
#include <stdint.h>
#include <core_cm3.h>

typedef struct
{
    __IO uint32_t tx_head;                /*!< UART Tx ring buffer head index */
    __IO uint32_t tx_tail;                /*!< UART Tx ring buffer tail index */
    __IO uint32_t rx_head;                /*!< UART Rx ring buffer head index */
    __IO uint32_t rx_tail;                /*!< UART Rx ring buffer tail index */
    __IO uint8_t  tx[512];  /*!< UART Tx data ring buffer */
    __IO uint8_t  rx[512];  /*!< UART Rx data ring buffer */
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


#endif
