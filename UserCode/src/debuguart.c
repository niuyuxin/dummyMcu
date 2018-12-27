/**
* filename: debugUart.c
* brief: 使用uart1 作为调试debug
*/
#include <string.h>
#include "framework.h"
#include "debuguart.h"
#include "user_types.h"

// UART Ring buffer
UART_RING_BUFFER_T rb_uart1;
// Current Tx Interrupt enable state
__IO FlagStatus TxIntStat_uart1;

/************************** PRIVATE VARIABLES *************************/
static uint8_t uart1menu[]=
"\n\r********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" UART Interrupt example \n\r"
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
"\t - UART Communication: 115200 bps \n\r"
" This example used to test UART Interrupt with using ring buffer.\n\r"
"  + Please press any key to be echoed\n\r"
"  + Press 'r' to re-show the welcome string\n\r"
"  + Press ESC to terminate\n\r"
"********************************************************************************\n\r";
/********************************************************************//**
 * @brief 		UART receive function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
static void UART_IntReceive(void)
{
	uint8_t tmpc;
	uint32_t rLen;

	while(1){
		// Call UART read function in UART driver
		rLen = UART_Receive(LPC_UART_DEBUG, &tmpc, 1, NONE_BLOCKING);
		// If data received
		if (rLen) {
			/* Check if buffer is more space
			 * If no more space, remaining character will be trimmed out
			 */
			if (!__BUF_IS_FULL(rb_uart1.rx_head,rb_uart1.rx_tail)) {
				rb_uart1.rx[rb_uart1.rx_head] = tmpc;
				__BUF_INCR(rb_uart1.rx_head);
			}
		} else {		// no more data
			break;
		}
	}
}

/********************************************************************//**
 * @brief 		UART transmit function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
static void UART_IntTransmit(void)
{
    // Disable THRE interrupt
	UART_IntConfig(LPC_UART_DEBUG, UART_INTCFG_THRE, DISABLE);

	/* Wait for FIFO buffer empty, transfer UART_TX_FIFO_SIZE bytes
	 * of data or break whenever ring buffers are empty */
	/* Wait until THR empty */
	while (UART_CheckBusy(LPC_UART_DEBUG) == SET);

	while (!__BUF_IS_EMPTY(rb_uart1.tx_head,rb_uart1.tx_tail)) {
    	if (UART_Send(LPC_UART_DEBUG, (uint8_t *)&rb_uart1.tx[rb_uart1.tx_tail], 1, NONE_BLOCKING)){
        /* Update transmit ring FIFO tail pointer */
        __BUF_INCR(rb_uart1.tx_tail);
    	} else {
    		break;
    	}
	}

    /* If there is no more data to send, disable the transmit
       interrupt - else enable it or keep it enabled */
	if (__BUF_IS_EMPTY(rb_uart1.tx_head, rb_uart1.tx_tail)) {
    	UART_IntConfig(LPC_UART_DEBUG, UART_INTCFG_THRE, DISABLE);
    	// Reset Tx Interrupt state
    	TxIntStat_uart1 = RESET;
    }
    else{
      	// Set Tx Interrupt state
		TxIntStat_uart1 = SET;
    	UART_IntConfig(LPC_UART_DEBUG, UART_INTCFG_THRE, ENABLE);
    }
}

/*********************************************************************//**
 * @brief		UART2 interrupt handler sub-routine
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void UART_DEBUG_IRQHander(void)
{
	uint32_t intsrc, tmp, tmp1;

	/* Determine the interrupt source */
	intsrc = UART_GetIntId(LPC_UART_DEBUG);
	tmp = intsrc & UART_IIR_INTID_MASK;

	// Receive Line Status
	if (tmp == UART_IIR_INTID_RLS){
		// Check line status
		tmp1 = UART_GetLineStatus(LPC_UART_DEBUG);
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
			UART_IntReceive();
	}

	// Transmit Holding Empty
	if (tmp == UART_IIR_INTID_THRE){
			UART_IntTransmit();
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
static uint32_t UART1Send(LPC_UART_TypeDef *UARTPort, uint8_t *txbuf, uint32_t buflen)
{
    uint8_t *data = (uint8_t *) txbuf;
    uint32_t bytes = 0;

	/* Temporarily lock out UART transmit interrupts during this
	   read so the UART transmit interrupt won't cause problems
	   with the index values */
    UART_IntConfig(UARTPort, UART_INTCFG_THRE, DISABLE);

	/* Loop until transmit run buffer is full or until n_bytes
	   expires */
	while ((buflen > 0) && (!__BUF_IS_FULL(rb_uart1.tx_head, rb_uart1.tx_tail)))
	{
		/* Write data from buffer into ring buffer */
		rb_uart1.tx[rb_uart1.tx_head] = *data;
		data++;

		/* Increment head pointer */
		__BUF_INCR(rb_uart1.tx_head);

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
	if (TxIntStat_uart1 == RESET) {
		UART_IntTransmit();
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
static uint32_t UART1Received(LPC_UART_TypeDef *UARTPort, uint8_t *rxbuf, uint32_t buflen)
{
    uint8_t *data = (uint8_t *) rxbuf;
    uint32_t bytes = 0;

	/* Temporarily lock out UART receive interrupts during this
	   read so the UART receive interrupt won't cause problems
	   with the index values */
	UART_IntConfig(UARTPort, UART_INTCFG_RBR, DISABLE);

	/* Loop until receive buffer ring is empty or
		until max_bytes expires */
	while ((buflen > 0) && (!(__BUF_IS_EMPTY(rb_uart1.rx_head, rb_uart1.rx_tail))))
	{
		/* Read data from ring buffer into user buffer */
		*data = rb_uart1.rx[rb_uart1.rx_tail];
		data++;

		/* Update tail pointer */
		__BUF_INCR(rb_uart1.rx_tail);

		/* Increment data count and decrement buffer size count */
		bytes++;
		buflen--;
	}

	/* Re-enable UART interrupts */
	UART_IntConfig(UARTPort, UART_INTCFG_RBR, ENABLE);

    return bytes;
}
/*********************************************************************//**
 * @brief	Print Welcome Screen Menu subroutine
 * @param	None
 * @return	None
 **********************************************************************/
static void print_uart1_menu(void)
{
	uint32_t tmp, tmp2;
	uint8_t *pDat;

	tmp = sizeof(uart1menu);
	tmp2 = 0;
	pDat = (uint8_t *)&uart1menu[0];
	while(tmp) {
		tmp2 = UART1Send(LPC_UART_DEBUG, pDat, tmp);
		pDat += tmp2;
		tmp -= tmp2;
	}
}
/*
 * *******************************************************************
 * @brief		uart3_init uart3 初始化，
						Initialize UART Configuration parameter structure to default state:
 *					Baudrate = 115200bps
 *					8 data bit
 *					1 Stop bit
 *					None parity
 * @param[in]	None
 * @return 		None
 * ********************************************************************
 */

static void uart1_init(void)
{
	// UART Configuration structure variable
	UART_CFG_Type UARTConfigStruct;
	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;

	PINSEL_ConfigPin(2,0,2); // TX
	PINSEL_ConfigPin(2,1,2); // RX

	UART_ConfigStructInit(&UARTConfigStruct);

	UARTConfigStruct.Baud_rate = 4800;

	// Initialize UART0 peripheral with given to corresponding parameter
	UART_Init((LPC_UART_TypeDef *)LPC_UART_DEBUG, &UARTConfigStruct);


	/* Initialize FIFOConfigStruct to default state:
	 * 				- FIFO_DMAMode = DISABLE
	 * 				- FIFO_Level = UART_FIFO_TRGLEV0
	 * 				- FIFO_ResetRxBuf = ENABLE
	 * 				- FIFO_ResetTxBuf = ENABLE
	 * 				- FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Initialize FIFO for UART2 peripheral
	UART_FIFOConfig((LPC_UART_TypeDef *)LPC_UART_DEBUG, &UARTFIFOConfigStruct);


	// Enable UART Transmit
	UART_TxCmd((LPC_UART_TypeDef *)LPC_UART_DEBUG, ENABLE);

    /* Enable UART Rx interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART_DEBUG, UART_INTCFG_RBR, ENABLE);
	/* Enable UART line status interrupt */
	UART_IntConfig((LPC_UART_TypeDef *)LPC_UART_DEBUG, UART_INTCFG_RLS, ENABLE);
	/*
	 * Do not enable transmit interrupt here, since it is handled by
	 * UART_Send() function, just to reset Tx Interrupt state for the
	 * first time
	 */
	TxIntStat_uart1 = RESET;

	// Reset ring buf head and tail idx
	__BUF_RESET(rb_uart1.rx_head);
	__BUF_RESET(rb_uart1.rx_tail);
	__BUF_RESET(rb_uart1.tx_head);
	__BUF_RESET(rb_uart1.tx_tail);

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(_UART_IRQ_DEBUG, ((0x01<<3)|0x04));

	/* Enable Interrupt for UART1 channel */
    NVIC_EnableIRQ(_UART_IRQ_DEBUG);


	// print welcome screen
	print_uart1_menu();
}
static uint16_t CrcCheck_LE(uint8_t* data, int size)
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
/**
* brief: 接收到的大端数据copy成小端
*/
static int copy2le(uint8_t* dst, uint8_t* src, int size)
{
	uint8_t i;
	for (i = 0; i < size; i++) {
		dst[i] = src[size-1-i];
	}
	return 0;
}
/**
 * 解析接收到的串口数据
 */
static int analysisData(uint8_t* buf, int len)
{
	uint8_t *data = buf;
	uint8_t all_size = len;
	int index = 0;
	uint16_t crcValue;
	while (1) {
		if (all_size <= index) return 0;
		len = all_size - index;
		if (len < 4 || len < (data[index+3]+7)) return 0;
		if (data[index] != 0x68 || data[data[index+3]+6+index] != 0x7e) return 0;
		crcValue = (data[data[index+3]+5+index] << 8) | (data[data[index+3]+4+index]);
		if (CrcCheck_LE(data+index, data[index+3]+4) == crcValue) {
			UART1Send(LPC_UART2, data+index, data[index+3]+7);
			switch (buf[index+2]){
			case 0x81: // 启动停止标志
				if (buf[index+2+1] == 1) {
					if (buf[index+2+1+1]) {
						; // evReadyToCharge |= (1 << buf[index+1]);
					} else {
						; // evReadyToCharge &= ~(1 << buf[index+1]);
					}
				}
				break;
			case 0x82: // ev 需求电压电流
//				dinCurrent = ((data[3] << 8) | data[4])/100;
//				dinVoltage = ((data[5] << 8) | data[6])/10;
				break;
			case 0x83: // 电源模块参数
				if (buf[index+2+1] == 8) {
					; // copy2le((uint8_t*)(&psmParam), buf+index+2+1+1, 8);
				}
				break;
			case 0x85: // 开启绝缘检测
				if (data[3]) {
					uint8_t buf[7]; int a;
//					din_isolation(buf, &a);
					UART1Send(LPC_UART2, buf, a);
				}
				break;
			default: break;
			}
		}
		index = index+data[index+2]+6;
	}
}
int uart1_received(void)
{
	static uint8_t len;
	static uint8_t rxbuf[30];
	static uint8_t size;
	
	uint8_t temp;
	while (UART1Received(LPC_UART_DEBUG, &temp, 1) > 0) {
		if (temp == 0x68 && size == 0) {
			rxbuf[size++] = temp;
		} else if ((rxbuf[0] == 0x68) && ((size >= 1)&&(size <= 3))) {
			if (size == 3) {
				len = temp+7;
			}
			rxbuf[size++] = temp;
		} else if ((rxbuf[0] == 0x68) && (size >= 4)) {
			rxbuf[size++] = temp;
			if (size >= len || size >= sizeof(rxbuf)/sizeof(uint8_t)) {
				if (temp == 0x7e) { // 正常接收到结尾
					analysisData(rxbuf, len);
				}
				memset(rxbuf, 0, sizeof(rxbuf));
				size = len = 0;
			} else if (temp == 0x7e) { // 异常接收到结尾
				size = len = 0;
				memset(rxbuf, 0, sizeof(rxbuf));
			}
		} else {
			size = len = 0;
			memset(rxbuf, 0, sizeof(rxbuf));
		}
	}
	return 0;
}
/**
* filename: byteorderexchange
*/
static void byteOrderExchange(uint8_t* buf, uint8_t size)
{
	uint8_t temp;
	uint8_t begin = 0, end = size-1;
	while (begin < end) {
		temp = buf[begin];
		buf[begin] = buf[end];
		buf[end] = temp;
		begin++;
		end--;
	}
}
/**
* filename: packageData
* brief: 打包数据， 添加协议头、协议尾、校验
* param: buf: 输出打包数据， data需要打包数据， size数据长度
*/
static void packageData(uint8_t* buf, nccs_frame_t* data, uint8_t order)
{
	int i;
	uint16_t crc;
	buf[0] = 0x68;
	buf[1] = data->addr;
	buf[2] = data->order;
	buf[3] = data->size;
	if (order == 0) { // 小端字节序
		for (i = 4; i < data->size+4; i++) {
			buf[i] = data->data[i-4];
		}
	} else { // 大端字节序
		for (i = 4; i < data->size+4; i++) {
			buf[i] = data->data[data->size-1-i+4];
		}
	}
	crc = CrcCheck_LE(buf, 4+data->size);
	buf[i++] = crc&0xff;
	buf[i++] = (crc>>8)&0xff;
	buf[i] = 0x7e;
}
/*********************************************************************/
/**
 * @name uart2_nCCS_init
 * @brief 初始化nCCS模拟仿真接口
 * @param[in]	None
 * @return 		bool 
 * ********************************************************************
 */
int uart1_debug_init(void)
{
	uart1_init();
	creatThread(uart1_received, THREAD_UART_DEBUG);
	return 1;

}

