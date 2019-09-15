/**
 * filename: PCF8563.C
 * author: niuyuxin
 * date: 2018-11-13
 * brief: ʹ��io��ģ�����i2c�Ĺ���, p0.27 sda, p0.28 scl
 */
#include "lpc177x_8x_pinsel.h"
#include "lpc177x_8x_gpio.h"
#define SET_I2C_SDA_OUT (GPIO_SetDir(0, 1<<27, 1))
#define SET_I2C_SDA_IN (GPIO_SetDir(0, 1<<27, 0))

void I2C_Init(void)
{					     
	PINSEL_ConfigPin(0, 27, 0);
	PINSEL_ConfigPin(0, 28, 0);
	GPIO_SetDir(0, 1<<28, 1); // scl ����Ϊ���
}

/**
* function: I2C_Start(void)
* brief: I2C start 
*/
void I2C_Start(void)
{
//	SET_I2C_SDA_OUT();     //sda�����
//	IIC_SDA=1;	  	  
//	IIC_SCL=1;
//	delay_us(4);
// 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
//	delay_us(4);
//	IIC_SCL=0;//ǯסI2C���ߣ�׼�����ͻ�������� 
}

