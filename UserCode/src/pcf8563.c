/**
 * filename: PCF8563.C
 * author: niuyuxin
 * date: 2018-11-13
 * brief: 使用io口模拟完成i2c的功能, p0.27 sda, p0.28 scl
 */
#include "lpc177x_8x_pinsel.h"
#include "lpc177x_8x_gpio.h"
#define SET_I2C_SDA_OUT (GPIO_SetDir(0, 1<<27, 1))
#define SET_I2C_SDA_IN (GPIO_SetDir(0, 1<<27, 0))

void I2C_Init(void)
{					     
	PINSEL_ConfigPin(0, 27, 0);
	PINSEL_ConfigPin(0, 28, 0);
	GPIO_SetDir(0, 1<<28, 1); // scl 设置为输出
}

/**
* function: I2C_Start(void)
* brief: I2C start 
*/
void I2C_Start(void)
{
//	SET_I2C_SDA_OUT();     //sda线输出
//	IIC_SDA=1;	  	  
//	IIC_SCL=1;
//	delay_us(4);
// 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
//	delay_us(4);
//	IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 
}

