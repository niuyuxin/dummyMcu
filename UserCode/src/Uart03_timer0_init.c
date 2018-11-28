

#include "lpc177x_8x_uart.h"
#include "lpc177x_8x_timer.h"
#include "lpc177x_8x_pinsel.h"
#include "lpc177x_8x_gpio.h"
#include "Uart03_timer0_init.h"

uint8_t buffer[128];
uint8_t gbtUartRec;
uint8_t dinUartRec;
uint8_t bms;
uint8_t p1_bms;
uint16_t vol;
uint16_t cur;
uint8_t soc;
uint16_t min;
float dinVoltage = 1.2;
float dinCurrent = 2.4;
uint8_t LrcCheck(uint8_t *ch, int size)
{
    uint8_t lrc = 0;
		int i = 0;
    for (i = 0; i < size; i++) {
        lrc += ch[i];
    }
    return lrc;
}
uint16_t CrcCheck(uint8_t* data, int size)
{
    uint16_t i, j, tmp, crc16 = 0xffff;
    for (i = 0; i < size; i++) {
        crc16 = data[i]^crc16;
        for (j = 0; j < 8; j++) {
            tmp=crc16 & 0x0001;
            crc16 = crc16 >> 1;
            if (tmp) crc16 = crc16 ^ 0xa001;
        }
    }

    return crc16;
}
void meterValue(uint8_t *buf, int* len)
{
	static uint32_t mValue;
	uint32_t tempMeterValue;
	uint8_t bcd[sizeof(uint32_t)];
	uint8_t tempBcd;
	int i;
	tempMeterValue = mValue;
	for (i = sizeof(uint32_t); i > 0; i--) {
		tempBcd = tempMeterValue%100;
		bcd[i-1] = ((tempBcd/10)<< 4)+((tempBcd%10)&0xf);
		tempMeterValue/=100;
	}
	mValue++;
	buf[0] = 0x68;
	buf[1] = 0x01;
	buf[2] = 4;
	for (i = 0; i < 4; i++) {
		buf[3+i] = bcd[i];
	}
	buf[7] = LrcCheck(bcd, sizeof(bcd));
	buf[8] = 0x7e;
	*len = 9;
}
void plugStatus(uint8_t *buf, int* len)
{
	uint8_t pState;
	if (GPIO_ReadValue(0) & (1<<14)) {
		pState = 0;
	} else {
		pState = 2;
	}
	
	buf[0] = 0x68;
	buf[1] = 0x11;
	buf[2] = 1;
	buf[3] = pState;
	buf[4] = pState;
	buf[5] = 0x7e;
	*len = 6;
}
void bmsStatus(uint8_t *buf, int*len) {
	buf[0] = 0x68;
	buf[1] = 0x13;
	buf[2] = 1;
	buf[3] = bms;
	buf[4] = bms;
	buf[5] = 0x7e;
	*len = 6;	
}
void bcsStatus(uint8_t *buf, int*len) {
	if (bms == 2) {
		if (vol < 7500) vol += 10;
		if (cur < 1600+4000) cur += 10;
		if (soc < 99) soc++;
		if (min > 1) min--;
	} else {
		vol = soc = 0;
		min = 1000;
		cur = 4000;
	}
	buf[0] = 0x68;
	buf[1] = 0x62;
	buf[2] = 9;
	buf[3] = vol & 0xff;
	buf[4] = vol >> 8;
	buf[5] = cur;
	buf[6] = cur >>8;
	buf[7] = 0;
	buf[8] = 0;
	buf[9] = soc;
	buf[10] = min;
	buf[11] = min>>8;
	buf[12] = LrcCheck(buf+3, 9);
	buf[13] = 0x7e;
	*len = 14;
}
#define ERRORPORT (1<<20)
void chargerStatus(uint8_t *buf, int *len)
{
	uint32_t portValue = GPIO_ReadValue(1);
	uint16_t errorCode = 0;
	if (!(portValue & ERRORPORT)) {
		errorCode = 0xffff;
	}
	buf[0] = 0x68;
	buf[1] = 0x51;
	buf[2] = 2;
	buf[3] = (errorCode >> 8);
	buf[4] = errorCode&0xff;
	buf[5] = LrcCheck(buf+3, 2);
	buf[6] = 0x7e;
	*len = 7;
}
void (*gbt_fun_protocol[])(uint8_t *buf, int* len) = {
	meterValue, 
	chargerStatus,
	plugStatus,
	bmsStatus,
	bcsStatus
};
void din_meterValue(uint8_t* buf, int* len) {
	static uint32_t mValue;
	uint32_t tempMeterValue;
	uint8_t bcd[sizeof(uint32_t)];
	uint8_t tempBcd;
	int i;
	uint16_t crcValue;
	tempMeterValue = mValue;
	for (i = sizeof(uint32_t); i > 0; i--) {
		tempBcd = tempMeterValue%100;
		bcd[i-1] = ((tempBcd/10)<< 4)+((tempBcd%10)&0xf);
		tempMeterValue/=100;
	}
	mValue++;
	buf[0] = 0x68;
	buf[1] = 0x01;
	buf[2] = 4;
	for (i = 0; i < 4; i++) {
		buf[3+i] = bcd[i];
	}
	crcValue = CrcCheck(buf, 7);
	buf[7] = crcValue;
	buf[8] = crcValue>>8;
	buf[9] = 0x7e;
	*len = 10;
}
void din_plugStatus(uint8_t* buf, int* len) {
	uint8_t pState;
	uint16_t crcValue;
	if (GPIO_ReadValue(1) & (1<<22)) { // P1.22 din plug status
		pState = 2;
	} else {
		pState = 3;
	}
	
	buf[0] = 0x68;
	buf[1] = 0x03;
	buf[2] = 1;
	buf[3] = pState;
	crcValue = CrcCheck(buf, 4);
	buf[4] = crcValue;
	buf[5] = crcValue >> 8;
	buf[6] = 0x7e;
	*len = 7;
}
void din_fault(uint8_t* buf, int* len)
{
	uint16_t fState;
	uint16_t crcValue;
	if (GPIO_ReadValue(4) & (1<<0)) {
		fState = 0;
	} else {
		fState = 0xffff;
	}
	buf[0] = 0x68;
	buf[1] = 0x05;
	buf[2] = 2;
	buf[3] = fState;
	buf[4] = fState>>8;
	crcValue = CrcCheck(buf, 5);
	buf[5] = crcValue;
	buf[6] = crcValue >> 8;
	buf[7] = 0x7e;
	*len = 8;
}
void din_outputVcValue(uint8_t* buf, int* len)
{
	union {
		uint32_t v;
		uint8_t ch[4];
	}temp;
	int i;
	uint16_t crcValue;
	buf[0] = 0x68;
	buf[1] = 0x06;
	buf[2] = 8;
	temp.v = *((uint32_t*)(&dinVoltage));
	for (i = 0; i < 4; i++) {
		buf[3+i] = temp.ch[3-i];
	}
	temp.v = *((uint32_t*)(&dinCurrent));
	for (i = 0; i < 4; i++) {
		buf[7+i] = temp.ch[3-i];
	}
	crcValue = CrcCheck(buf, 11);
	buf[11] = crcValue;
	buf[12] = crcValue >> 8;
	buf[13] = 0x7e;
	*len = 14;
}
void din_isolation(uint8_t *buf, int *len)
{
	uint16_t crcValue;
	buf[0] = 0x68;
	buf[1] = 0x4;
	buf[2] = 1;
	buf[3] = 1;
	crcValue = CrcCheck(buf, 4);
	buf[4] = crcValue;
	buf[5] = crcValue >> 8;
	buf[6] = 0x7e;
	*len = 7;
}
void din_bmsStatus(uint8_t *buf, int *len)
{
	uint16_t crcValue;
	buf[0] = 0x68;
	buf[1] = 0x13;
	buf[2] = 1;
	buf[3] = p1_bms;
	crcValue = CrcCheck(buf, 4);
	buf[4] = crcValue;
	buf[5] = crcValue >> 8;
	buf[6] = 0x7e;
	*len = 7;
}
void (*din_fun_protocol[])(uint8_t *buf, int* len) = {
	din_meterValue,
	din_plugStatus,
	din_fault,
	din_outputVcValue,
	din_bmsStatus
};
/************************** PRIVATE DEFINTIONS *************************/

#define	_LPC_UART_GBT			(LPC_UART_TypeDef *)LPC_UART0
#define _UART_IRQ_GBT			UART0_IRQn
#define _UART_IRQHander_GBT		UART0_IRQHandler

#define _LPC_UART_DIN			(LPC_UART_TypeDef *)LPC_UART3
#define _UART_IRQ_DIN			UART3_IRQn
#define _UART_IRQHander_DIN		UART3_IRQHandler

/* buffer size definition */
#define UART_RING_BUFSIZE 256

/* Buf mask */
#define __BUF_MASK (UART_RING_BUFSIZE-1)
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
    __IO uint8_t  tx[UART_RING_BUFSIZE];  /*!< UART Tx data ring buffer */
    __IO uint8_t  rx[UART_RING_BUFSIZE];  /*!< UART Rx data ring buffer */
} UART_RING_BUFFER_T;
typedef struct
{
    __IO uint32_t head;                /*!< UART Rx ring buffer head index */
    __IO uint32_t tail;                /*!< UART Rx ring buffer tail index */
	uint8_t  data[UART_RING_BUFSIZE];  /*!< UART Tx data ring buffer */
} USER_RING_BUFFER_T;

/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[]=
"\r\n********************************************************************************\r\n"
" Hello NXP Semiconductors \r\n"
" UART Interrupt example \r\n"
"\t - MCU: LPC177x_8x \r\n"
"\t - Core: ARM CORTEX-M3 \r\n"
"\t - UART Communication: 115200 bps \r\n"
" This example used to test UART Interrupt with using ring buffer. \r\n"
"  + Please press any key to be echoed \r\n"
"  + Press 'r' to re-show the welcome string \r\n"
"  + Press ESC to terminate \r\n"
"******************************************************************************** \r\n";

uint8_t menu2[] = "\n\rUART Interrupt demo terminated!";

// UART Ring buffer
UART_RING_BUFFER_T rb_gbt;
USER_RING_BUFFER_T urb_gbt;

UART_RING_BUFFER_T rb_din;
USER_RING_BUFFER_T urb_din;

// Current Tx Interrupt enable state
__IO FlagStatus TxIntStat_gbt;
// Current Tx Interrupt enable state
__IO FlagStatus TxIntStat_din;

/************************** PRIVATE FUNCTIONS *************************/
/* Interrupt service routines */
void _UART_IRQHander_GBT(void);
void UART_IntErr(uint8_t bLSErrType);
void UART_IntTransmit_GBT(void);
void UART_IntReceive_GBT(void);

void _UART_IRQHander_DIN(void);
void UART_IntTransmit_DIN(void);
void UART_IntReceive_DIN(void);

uint32_t UARTReceive_GBT(LPC_UART_TypeDef *UARTPort, uint8_t *rxbuf, uint32_t buflen);
uint32_t UARTSend_GBT(LPC_UART_TypeDef *UARTPort, uint8_t *txbuf, uint32_t buflen);

uint32_t UARTReceive_DIN(LPC_UART_TypeDef *UARTPort, uint8_t *rxbuf, uint32_t buflen);
uint32_t UARTSend_DIN(LPC_UART_TypeDef *UARTPort, uint8_t *txbuf, uint32_t buflen);

void print_menu(void);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief		UART0 interrupt handler sub-routine
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _UART_IRQHander_GBT(void)
{
	uint32_t intsrc, tmp, tmp1;

	/* Determine the interrupt source */
 	intsrc = UART_GetIntId(_LPC_UART_GBT);
	tmp = intsrc & UART_IIR_INTID_MASK;

	// Receive Line Status
	if (tmp == UART_IIR_INTID_RLS){
		// Check line status
		tmp1 = UART_GetLineStatus(_LPC_UART_GBT);
		// Mask out the Receive Ready and Transmit Holding empty status
		tmp1 &= (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE \
				| UART_LSR_BI | UART_LSR_RXFE);
		// If any error exist
		if (tmp1) {
			return;
				// UART_IntErr(tmp1);
		}
	}

	// Receive Data Available or Character time-out
	if ((tmp == UART_IIR_INTID_RDA) || (tmp == UART_IIR_INTID_CTI)){
			gbtUartRec = 5;
			UART_IntReceive_GBT();
	}

	// Transmit Holding Empty
	if (tmp == UART_IIR_INTID_THRE){
			UART_IntTransmit_GBT();
	}

}
/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief		UART3 interrupt handler sub-routine
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _UART_IRQHander_DIN(void)
{
	uint32_t intsrc, tmp, tmp1;

	/* Determine the interrupt source */
 	intsrc = UART_GetIntId(_LPC_UART_DIN);
	tmp = intsrc & UART_IIR_INTID_MASK;

	// Receive Line Status
	if (tmp == UART_IIR_INTID_RLS){
		// Check line status
		tmp1 = UART_GetLineStatus(_LPC_UART_DIN);
		// Mask out the Receive Ready and Transmit Holding empty status
		tmp1 &= (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE \
				| UART_LSR_BI | UART_LSR_RXFE);
		// If any error exist
		if (tmp1) {
			return;
				// UART_IntErr(tmp1);
		}
	}

	// Receive Data Available or Character time-out
	if ((tmp == UART_IIR_INTID_RDA) || (tmp == UART_IIR_INTID_CTI)){
			dinUartRec = 5;
			UART_IntReceive_DIN();
	}

	// Transmit Holding Empty
	if (tmp == UART_IIR_INTID_THRE){
			UART_IntTransmit_DIN();
	}

}
/********************************************************************//**
 * @brief 		UART receive function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
void UART_IntReceive_GBT(void)
{
	uint8_t tmpc;
	uint32_t rLen;

	while(1){
		// Call UART read function in UART driver
		rLen = UART_Receive(_LPC_UART_GBT, &tmpc, 1, NONE_BLOCKING);
		// If data received
		if (rLen){
			/* Check if buffer is more space
			 * If no more space, remaining character will be trimmed out
			 */
			if (!__BUF_IS_FULL(rb_gbt.rx_head,rb_gbt.rx_tail)){
				rb_gbt.rx[rb_gbt.rx_head] = tmpc;
				__BUF_INCR(rb_gbt.rx_head);
			}
		}
		// no more data
		else {
			break;
		}
	}
}
/********************************************************************//**
 * @brief 		UART receive function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
void UART_IntReceive_DIN(void)
{
	uint8_t tmpc;
	uint32_t rLen;

	while(1){
		// Call UART read function in UART driver
		rLen = UART_Receive(_LPC_UART_DIN, &tmpc, 1, NONE_BLOCKING);
		// If data received
		if (rLen){
			/* Check if buffer is more space
			 * If no more space, remaining character will be trimmed out
			 */
			if (!__BUF_IS_FULL(rb_din.rx_head,rb_din.rx_tail)){
				rb_din.rx[rb_din.rx_head] = tmpc;
				__BUF_INCR(rb_din.rx_head);
			}
		}
		// no more data
		else {
			break;
		}
	}
}
/********************************************************************//**
 * @brief 		UART transmit function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
void UART_IntTransmit_GBT(void)
{
    // Disable THRE interrupt
    UART_IntConfig(_LPC_UART_GBT, UART_INTCFG_THRE, DISABLE);

	/* Wait for FIFO buffer empty, transfer UART_TX_FIFO_SIZE bytes
	 * of data or break whenever ring buffers are empty */
	/* Wait until THR empty */
    while (UART_CheckBusy(_LPC_UART_GBT) == SET);

	while (!__BUF_IS_EMPTY(rb_gbt.tx_head,rb_gbt.tx_tail))
    {
        /* Move a piece of data into the transmit FIFO */
    	if (UART_Send(_LPC_UART_GBT, (uint8_t *)&rb_gbt.tx[rb_gbt.tx_tail], 1, NONE_BLOCKING)){
        /* Update transmit ring FIFO tail pointer */
        __BUF_INCR(rb_gbt.tx_tail);
    	} else {
    		break;
    	}
    }

    /* If there is no more data to send, disable the transmit
       interrupt - else enable it or keep it enabled */
	if (__BUF_IS_EMPTY(rb_gbt.tx_head, rb_gbt.tx_tail)) {
    	UART_IntConfig(_LPC_UART_GBT, UART_INTCFG_THRE, DISABLE);
    	// Reset Tx Interrupt state
    	TxIntStat_gbt = RESET;
    }
    else{
      	// Set Tx Interrupt state
		TxIntStat_gbt = SET;
    	UART_IntConfig(_LPC_UART_GBT, UART_INTCFG_THRE, ENABLE);
    }
}
void UART_IntTransmit_DIN(void)
{
    // Disable THRE interrupt
    UART_IntConfig(_LPC_UART_DIN, UART_INTCFG_THRE, DISABLE);

	/* Wait for FIFO buffer empty, transfer UART_TX_FIFO_SIZE bytes
	 * of data or break whenever ring buffers are empty */
	/* Wait until THR empty */
    while (UART_CheckBusy(_LPC_UART_DIN) == SET);

	while (!__BUF_IS_EMPTY(rb_din.tx_head,rb_din.tx_tail))
    {
        /* Move a piece of data into the transmit FIFO */
    	if (UART_Send(_LPC_UART_DIN, (uint8_t *)&rb_din.tx[rb_din.tx_tail], 1, NONE_BLOCKING)){
        /* Update transmit ring FIFO tail pointer */
        __BUF_INCR(rb_din.tx_tail);
    	} else {
    		break;
    	}
    }

    /* If there is no more data to send, disable the transmit
       interrupt - else enable it or keep it enabled */
	if (__BUF_IS_EMPTY(rb_din.tx_head, rb_din.tx_tail)) {
    	UART_IntConfig(_LPC_UART_DIN, UART_INTCFG_THRE, DISABLE);
    	// Reset Tx Interrupt state
    	TxIntStat_din = RESET;
    }
    else{
      	// Set Tx Interrupt state
		TxIntStat_din = SET;
    	UART_IntConfig(_LPC_UART_DIN, UART_INTCFG_THRE, ENABLE);
    }
}

/*********************************************************************//**
 * @brief		UART Line Status Error
 * @param[in]	bLSErrType	UART Line Status Error Type
 * @return		None
 **********************************************************************/
void UART_IntErr(uint8_t bLSErrType)
{
	uint8_t test;
	// Loop forever
	while (1){
		// For testing purpose
		test = bLSErrType;
	}
}

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief		UART transmit function for interrupt mode (using ring buffers)
 * @param[in]	UARTPort	Selected UART peripheral used to send data,
 * 				should be UART0
 * @param[out]	txbuf Pointer to Transmit buffer
 * @param[in]	buflen Length of Transmit buffer
 * @return 		Number of bytes actually sent to the ring buffer
 **********************************************************************/
uint32_t UARTSend(LPC_UART_TypeDef *UARTPort, uint8_t *txbuf, uint32_t buflen)
{
    uint8_t *data = (uint8_t *) txbuf;
    uint32_t bytes = 0;

	/* Temporarily lock out UART transmit interrupts during this
	   read so the UART transmit interrupt won't cause problems
	   with the index values */
    UART_IntConfig(UARTPort, UART_INTCFG_THRE, DISABLE);

	/* Loop until transmit run buffer is full or until n_bytes
	   expires */
	while ((buflen > 0) && (!__BUF_IS_FULL(rb_gbt.tx_head, rb_gbt.tx_tail)))
	{
		/* Write data from buffer into ring buffer */
		rb_gbt.tx[rb_gbt.tx_head] = *data;
		data++;

		/* Increment head pointer */
		__BUF_INCR(rb_gbt.tx_head);

		/* Increment data count and decrement buffer size count */
		bytes++;
		buflen--;
	}

	/*
	 * Check if current Tx interrupt enable is reset,
	 * that means the Tx interrupt must be re-enabled
	 * due to call UART_IntTransmit() function to trigger
	 * this interrupt type
	 */
	if (TxIntStat_gbt == RESET) {
		UART_IntTransmit_GBT();
	}
	/*
	 * Otherwise, re-enables Tx Interrupt
	 */
	else {
		UART_IntConfig(UARTPort, UART_INTCFG_THRE, ENABLE);
	}

    return bytes;
}

uint32_t UARTSend_DIN(LPC_UART_TypeDef *UARTPort, uint8_t *txbuf, uint32_t buflen)
{
    uint8_t *data = (uint8_t *) txbuf;
    uint32_t bytes = 0;

	/* Temporarily lock out UART transmit interrupts during this
	   read so the UART transmit interrupt won't cause problems
	   with the index values */
    UART_IntConfig(UARTPort, UART_INTCFG_THRE, DISABLE);

	/* Loop until transmit run buffer is full or until n_bytes
	   expires */
	while ((buflen > 0) && (!__BUF_IS_FULL(rb_din.tx_head, rb_din.tx_tail)))
	{
		/* Write data from buffer into ring buffer */
		rb_din.tx[rb_din.tx_head] = *data;
		data++;

		/* Increment head pointer */
		__BUF_INCR(rb_din.tx_head);

		/* Increment data count and decrement buffer size count */
		bytes++;
		buflen--;
	}

	/*
	 * Check if current Tx interrupt enable is reset,
	 * that means the Tx interrupt must be re-enabled
	 * due to call UART_IntTransmit() function to trigger
	 * this interrupt type
	 */
	if (TxIntStat_din == RESET) {
		UART_IntTransmit_DIN();
	}
	/*
	 * Otherwise, re-enables Tx Interrupt
	 */
	else {
		UART_IntConfig(UARTPort, UART_INTCFG_THRE, ENABLE);
	}

    return bytes;
}
/*********************************************************************//**
 * @brief		UART read function for interrupt mode (using ring buffers)
 * @param[in]	UARTPort	Selected UART peripheral used to send data,
 * 				should be UART0
 * @param[out]	rxbuf Pointer to Received buffer
 * @param[in]	buflen Length of Received buffer
 * @return 		Number of bytes actually read from the ring buffer
 **********************************************************************/
uint32_t UARTReceive_GBT(LPC_UART_TypeDef *UARTPort, uint8_t *rxbuf, uint32_t buflen)
{
    uint8_t *data = (uint8_t *) rxbuf;
    uint32_t bytes = 0;

	/* Temporarily lock out UART receive interrupts during this
	   read so the UART receive interrupt won't cause problems
	   with the index values */
	UART_IntConfig(UARTPort, UART_INTCFG_RBR, DISABLE);

	/* Loop until receive buffer ring is empty or
		until max_bytes expires */
	while ((buflen > 0) && (!(__BUF_IS_EMPTY(rb_gbt.rx_head, rb_gbt.rx_tail))))
	{
		/* Read data from ring buffer into user buffer */
		*data = rb_gbt.rx[rb_gbt.rx_tail];
		data++;

		/* Update tail pointer */
		__BUF_INCR(rb_gbt.rx_tail);

		/* Increment data count and decrement buffer size count */
		bytes++;
		buflen--;
	}

	/* Re-enable UART interrupts */
	UART_IntConfig(UARTPort, UART_INTCFG_RBR, ENABLE);

    return bytes;
}
uint32_t UARTReceive_DIN(LPC_UART_TypeDef *UARTPort, uint8_t *rxbuf, uint32_t buflen)
{
    uint8_t *data = (uint8_t *) rxbuf;
    uint32_t bytes = 0;

	/* Temporarily lock out UART receive interrupts during this
	   read so the UART receive interrupt won't cause problems
	   with the index values */
	UART_IntConfig(UARTPort, UART_INTCFG_RBR, DISABLE);

	/* Loop until receive buffer ring is empty or
		until max_bytes expires */
	while ((buflen > 0) && (!(__BUF_IS_EMPTY(rb_din.rx_head, rb_din.rx_tail))))
	{
		/* Read data from ring buffer into user buffer */
		*data = rb_din.rx[rb_din.rx_tail];
		data++;

		/* Update tail pointer */
		__BUF_INCR(rb_din.rx_tail);

		/* Increment data count and decrement buffer size count */
		bytes++;
		buflen--;
	}

	/* Re-enable UART interrupts */
	UART_IntConfig(UARTPort, UART_INTCFG_RBR, ENABLE);

    return bytes;
}
int monitorGbtData(uint8_t *buf, uint32_t size)
{
	uint8_t *data = buf;
	uint8_t all_size = size;
	int index = 0;
	while (1) {
		if (all_size <= index) return 0;
		size = all_size - index;
		if (size < 3 || size < (data[index+2]+5)) return 0;
		if (data[index] != 0x68 ||
			data[data[index+2]+4+index] != 0x7e) return 0;
		if (LrcCheck(data+index+3, data[index+2]) == data[data[index+2]+3+index]) {
			switch (buf[index+1]){
			case 0x41:
				bms = 2;
				break;
			case 0x42:
				bms = 0;
				break;
			default: break;
			}
		}
		index = index+data[index+2]+5;
	}
}
int monitorDinData(uint8_t *buf, uint32_t size)
{
	int breaker;
	uint8_t *data = buf;
	uint8_t all_size = size;
	int index = 0;
	uint16_t crcValue;
	while (1) {
		if (all_size <= index) return 0;
		size = all_size - index;
		if (size < 3 || size < (data[index+2]+6)) return 0;
		if (data[index] != 0x68 ||
			data[data[index+2]+5+index] != 0x7e) return 0;
		crcValue = (data[data[index+2]+4+index] << 8) | (data[data[index+2]+3+index]);
		if (CrcCheck(data+index, data[index+2]+3) == crcValue) {
			if (buf[index+1] == 0x81) {
				breaker = 1;
			}
			if (buf[index+1] == 0x83) {
				breaker = 3;
			}
			if (buf[index+1] == 0x86) {
				breaker = 2;
			}
			UARTSend_DIN(_LPC_UART_DIN, data+index, data[index+2]+6);
			switch (buf[index+1]){
			case 0x81:
				if (data[3])
					p1_bms = 2;
				else p1_bms = 0;
				break;
			case 0x82:
				dinCurrent = ((data[3] << 8) | data[4])/100;
				dinVoltage = ((data[5] << 8) | data[6])/10;
				break;
			case 0x83:
				break;
			case 0x85:
				if (data[3]) {
					uint8_t buf[7]; int a;
					din_isolation(buf, &a);
					UARTSend_DIN(_LPC_UART_DIN, buf, a);
				}
				break;
			default: break;
			}
		}
		index = index+data[index+2]+6;
	}
}
/*********************************************************************//**
 * @brief	Print Welcome Screen Menu subroutine
 * @param	None
 * @return	None
 **********************************************************************/
void print_menu(void)
{
	uint32_t tmp, tmp2;
	uint8_t *pDat;

	tmp = sizeof(menu1);
	tmp2 = 0;
	pDat = (uint8_t *)&menu1[0];
	while(tmp)
	{
		tmp2 = UARTSend(_LPC_UART_GBT, pDat, tmp);
		pDat += tmp2;
		tmp -= tmp2;
	}
}
void print_menu_uart3(void)
{
	uint32_t tmp, tmp2;
	uint8_t *pDat;

	tmp = sizeof(menu1);
	tmp2 = 0;
	pDat = (uint8_t *)&menu1[0];
	while(tmp)
	{
		tmp2 = UARTSend_DIN(_LPC_UART_DIN, pDat, tmp);
		pDat += tmp2;
		tmp -= tmp2;
	}
}
//timer init
TIM_TIMERCFG_Type TIM_ConfigStruct;
TIM_MATCHCFG_Type TIM_MatchConfigStruct ;
uint8_t volatile timer0_flag = FALSE, timer1_flag = FALSE;
FunctionalState LEDStatus = ENABLE;

/*********************************************************************//**
 * @brief		TIMER0 interrupt handler sub-routine
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void TIMER1_IRQHandler(void)
{
	static int peridic;
	int i;
	int allLen = 0, len;
	if (TIM_GetIntStatus(LPC_TIM1, TIM_MR0_INT)== SET)
	{
		if (peridic%100 == 0) {
			for (i = 0; i < sizeof(gbt_fun_protocol)/4; i++) {
				gbt_fun_protocol[i](buffer+allLen, &len);
				allLen += len;
			}
			UARTSend(_LPC_UART_GBT, buffer, allLen);
		} else if (peridic%101 == 0) {
			for (i = 0; i < sizeof(din_fun_protocol)/4; i++) {
				din_fun_protocol[i](buffer+allLen, &len);
				allLen += len;
			}
			UARTSend_DIN(_LPC_UART_DIN, buffer, allLen);
		}
		if (gbtUartRec > 1) {
			gbtUartRec--;
			if (gbtUartRec <= 1) {
				len = UARTReceive_GBT(_LPC_UART_GBT, urb_gbt.data, 255);
				monitorGbtData(urb_gbt.data, len);
				gbtUartRec = 0;
			}
		}
		if (dinUartRec > 1) {
			dinUartRec--;
			if (dinUartRec <= 1) {
				len = UARTReceive_DIN(_LPC_UART_DIN, urb_din.data, 255);
				monitorDinData(urb_din.data, len);
				dinUartRec = 0;
			}
		}
		peridic++;
	}
	TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
}


/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief		Print menu
 * @param[in]	None
 * @return 		None
 **********************************************************************/

void debug_uart0(void)
{
	// UART Configuration structure variable
	UART_CFG_Type UARTConfigStruct;
	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;

	PINSEL_ConfigPin(0,2,1);
	PINSEL_ConfigPin(0,3,1);
	/* Initialize UART Configuration parameter structure to default state:
	 * Baudrate = 9600bps
	 * 8 data bit
	 * 1 Stop bit
	 * None parity
	 */
	UART_ConfigStructInit(&UARTConfigStruct);

	// Initialize UART0 peripheral with given to corresponding parameter
	UART_Init((LPC_UART_TypeDef *)_LPC_UART_GBT, &UARTConfigStruct);


	/* Initialize FIFOConfigStruct to default state:
	 * 				- FIFO_DMAMode = DISABLE
	 * 				- FIFO_Level = UART_FIFO_TRGLEV0
	 * 				- FIFO_ResetRxBuf = ENABLE
	 * 				- FIFO_ResetTxBuf = ENABLE
	 * 				- FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Initialize FIFO for UART0 peripheral
	UART_FIFOConfig((LPC_UART_TypeDef *)_LPC_UART_GBT, &UARTFIFOConfigStruct);


	// Enable UART Transmit
	UART_TxCmd((LPC_UART_TypeDef *)_LPC_UART_GBT, ENABLE);

    /* Enable UART Rx interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)_LPC_UART_GBT, UART_INTCFG_RBR, ENABLE);
	/* Enable UART line status interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)_LPC_UART_GBT, UART_INTCFG_RLS, ENABLE);
	/*
	 * Do not enable transmit interrupt here, since it is handled by
	 * UART_Send() function, just to reset Tx Interrupt state for the
	 * first time
	 */
	TxIntStat_gbt = RESET;

	// Reset ring buf head and tail idx
	__BUF_RESET(rb_gbt.rx_head);
	__BUF_RESET(rb_gbt.rx_tail);
	__BUF_RESET(rb_gbt.tx_head);
	__BUF_RESET(rb_gbt.tx_tail);
	
    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(_UART_IRQ_GBT, ((0x01<<3)|0x01));

	/* Enable Interrupt for UART0 channel */
    NVIC_EnableIRQ(_UART_IRQ_GBT);


	// print welcome screen
	print_menu();
}
void debug_uart3(void)
{
	// UART Configuration structure variable
	UART_CFG_Type UARTConfigStruct;
	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;

	PINSEL_ConfigPin(4,28,2);
	PINSEL_ConfigPin(4,29,2);
	/* Initialize UART Configuration parameter structure to default state:
	 * Baudrate = 9600bps
	 * 8 data bit
	 * 1 Stop bit
	 * None parity
	 */
	UART_ConfigStructInit(&UARTConfigStruct);

	UARTConfigStruct.Baud_rate = 115200;
	// Initialize UART3 peripheral with given to corresponding parameter
	UART_Init((LPC_UART_TypeDef *)_LPC_UART_DIN, &UARTConfigStruct);


	/* Initialize FIFOConfigStruct to default state:
	 * 				- FIFO_DMAMode = DISABLE
	 * 				- FIFO_Level = UART_FIFO_TRGLEV0
	 * 				- FIFO_ResetRxBuf = ENABLE
	 * 				- FIFO_ResetTxBuf = ENABLE
	 * 				- FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Initialize FIFO for UART0 peripheral
	UART_FIFOConfig((LPC_UART_TypeDef *)_LPC_UART_DIN, &UARTFIFOConfigStruct);


	// Enable UART Transmit
	UART_TxCmd((LPC_UART_TypeDef *)_LPC_UART_DIN, ENABLE);

    /* Enable UART Rx interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)_LPC_UART_DIN, UART_INTCFG_RBR, ENABLE);
	/* Enable UART line status interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)_LPC_UART_DIN, UART_INTCFG_RLS, ENABLE);
	/*
	 * Do not enable transmit interrupt here, since it is handled by
	 * UART_Send() function, just to reset Tx Interrupt state for the
	 * first time
	 */
	TxIntStat_din = RESET;

	// Reset ring buf head and tail idx
	__BUF_RESET(rb_din.rx_head);
	__BUF_RESET(rb_din.rx_tail);
	__BUF_RESET(rb_din.tx_head);
	__BUF_RESET(rb_din.tx_tail);

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(_UART_IRQ_DIN, ((0x01<<3)|0x02));

	/* Enable Interrupt for UART3 channel */
    NVIC_EnableIRQ(_UART_IRQ_DIN);
	
	print_menu_uart3();
}
void init_uart2(void)
{
	// UART Configuration structure variable
	UART_CFG_Type UARTConfigStruct;
	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;

	PINSEL_ConfigPin(0,10,1);
	PINSEL_ConfigPin(0,11,1);
	/* Initialize UART Configuration parameter structure to default state:
	 * Baudrate = 9600bps
	 * 8 data bit
	 * 1 Stop bit
	 * None parity
	 */
	UART_ConfigStructInit(&UARTConfigStruct);

	UARTConfigStruct.Baud_rate = 115200;
	// Initialize UART3 peripheral with given to corresponding parameter
	UART_Init(LPC_UART2, &UARTConfigStruct);


	/* Initialize FIFOConfigStruct to default state:
	 * 				- FIFO_DMAMode = DISABLE
	 * 				- FIFO_Level = UART_FIFO_TRGLEV0
	 * 				- FIFO_ResetRxBuf = ENABLE
	 * 				- FIFO_ResetTxBuf = ENABLE
	 * 				- FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Initialize FIFO for UART0 peripheral
	UART_FIFOConfig(LPC_UART2, &UARTFIFOConfigStruct);


	// Enable UART Transmit
	UART_TxCmd((LPC_UART_TypeDef *)_LPC_UART_DIN, ENABLE);

    /* Enable UART Rx interrupt */
	UART_IntConfig(LPC_UART2, UART_INTCFG_RBR, ENABLE);
	/* Enable UART line status interrupt */
	UART_IntConfig(LPC_UART2, UART_INTCFG_RLS, ENABLE);
	/*
	 * Do not enable transmit interrupt here, since it is handled by
	 * UART_Send() function, just to reset Tx Interrupt state for the
	 * first time
	 */
	TxIntStat_din = RESET;

	// Reset ring buf head and tail idx
	__BUF_RESET(rb_din.rx_head);
	__BUF_RESET(rb_din.rx_tail);
	__BUF_RESET(rb_din.tx_head);
	__BUF_RESET(rb_din.tx_tail);

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(UART2_IRQn, ((0x01<<3)|0x03));

	/* Enable Interrupt for UART3 channel */
    NVIC_EnableIRQ(UART2_IRQn);
	
	print_menu_uart3();
}
/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief		c_entry: Main TIMER program body
 * @param[in]	None
 * @return 		int
 **********************************************************************/
int uart03_timer_init(void)
{
		// Reset ring buf head and tail idx
	__BUF_RESET(urb_gbt.head);
	__BUF_RESET(urb_gbt.tail);
	debug_uart0();
	debug_uart3();
	GPIO_SetDir(0, 1<<14, 0); // gbt ev plug
	
	GPIO_SetDir(1, 1<<22, 0); // din ev plug
	GPIO_SetDir(4, 1<<0, 0);
	// Initialize timer 1, prescale count time of 1000uS
	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
	TIM_ConfigStruct.PrescaleValue	= 1000;

	// use channel 0, MR0
	TIM_MatchConfigStruct.MatchChannel = 0;
	// Enable interrupt when MR0 matches the value in TC register
	TIM_MatchConfigStruct.IntOnMatch   = TRUE;
	//Enable reset on MR0: TIMER will reset if MR0 matches it
	TIM_MatchConfigStruct.ResetOnMatch = TRUE;
	//Stop on MR0 if MR0 matches it
	TIM_MatchConfigStruct.StopOnMatch  = FALSE;
	//Toggle MR0.0 pin if MR0 matches it
	TIM_MatchConfigStruct.ExtMatchOutputType =TIM_EXTMATCH_NOTHING;
	// Set Match value, count value of 10000 (10000 * 100uS = 1000000us = 1s --> 1 Hz)
	TIM_MatchConfigStruct.MatchValue   = 10;

	TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &TIM_ConfigStruct);
	TIM_ConfigMatch(LPC_TIM1, &TIM_MatchConfigStruct);

	/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(TIMER1_IRQn, ((0x01<<3)|0x02));
	/* Enable interrupt for timer 0 */
	NVIC_EnableIRQ(TIMER1_IRQn);
	// To start timer
	TIM_Cmd(LPC_TIM1, ENABLE);
	return 1;

}


