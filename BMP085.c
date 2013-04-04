/*
 * BMP085.c
 *
 *  Created on: 2013-1-19
 *      Author: Daniel
 */

#include "BMP085.h"

struct CalibrationData
{
	short			ac1;
	short			ac2;
	short			ac3;

	unsigned short	ac4;
	unsigned short	ac5;
	unsigned short	ac6;

	short			b1;
	short			b2;
	short			mb;
	short			mc;
	short			md;
};

long x1, x2, x3, b3, b5, b6, p;
unsigned long b4, b7;

static struct CalibrationData		s_CalData;
const unsigned char					oversampling_setting = 3;

static int							s_raw_ut;
static unsigned long				s_raw_up;
static unsigned char				s_raw_up_bytes[3];

int									g_nPressure;				// 10hPa的压力值
int									g_nTemperature_BMP085;		// 0.1的温度值

int ReadI2C(int byteCount, unsigned char* bytes);
int WriteI2C(int byteCount, unsigned char* bytes);

void Init_BMP085()
{
	unsigned char	temp;
	volatile int	i;

	// 初始化I2C芯片
	UCB1CTL1 |= UCSWRST;

	UCB1CTL0	= UCMST + UCMODE_3 + UCSYNC;
	UCB1CTL1 	= UCSSEL_2 + UCSWRST;				// SMCLK 8MHz

	UCB1BR0 	= 5;								// 	400Khz
	UCB1BR1		= 0;

	UCB1I2CSA	= 0x77;								// BMP085 I2C地址

	// 取消I2C模块复位
	UCB1CTL1 &= ~UCSWRST;

	// 复位BMP085
	P5OUT &= ~(BIT0);					// Rest BMP085

	// 延迟10ms，取消复位
	i = 1000;
	while (i != 0)
	{
		i--;
		__delay_cycles(100);
	}
	P5OUT |= BIT0;

	// 等待10ms，BMP085进入正常工作状态
	i = 1000;
	while (i != 0)
	{
		i--;
		__delay_cycles(100);
	}

	// 初始化BMP085，读取校正数据
	temp = 0xAA;
	WriteI2C(1, &temp);
	ReadI2C(sizeof(s_CalData), (unsigned char*)&s_CalData);

	// 调整字节序
	s_CalData.ac1 = _swap_bytes(s_CalData.ac1);
	s_CalData.ac2 = _swap_bytes(s_CalData.ac2);
	s_CalData.ac3 = _swap_bytes(s_CalData.ac3);
	s_CalData.ac4 = _swap_bytes(s_CalData.ac4);
	s_CalData.ac5 = _swap_bytes(s_CalData.ac5);
	s_CalData.ac6 = _swap_bytes(s_CalData.ac6);

	s_CalData.b1  = _swap_bytes(s_CalData.b1);
	s_CalData.b2  = _swap_bytes(s_CalData.b2);

	s_CalData.mb  = _swap_bytes(s_CalData.mb);
	s_CalData.mc  = _swap_bytes(s_CalData.mc);
	s_CalData.md  = _swap_bytes(s_CalData.md);
}

// 启动温度测量
void StartTemperature_BMP085()
{
	unsigned char	contrl[2];

	contrl[0] = 0xF4;
	contrl[1] = 0x2E;
	WriteI2C(2, contrl);
}

bool GetTemperature_BMP085()
{
	unsigned char	temp;

	// 检查状态位（BMP085文档没有说明的，用于代替EOC检测）
	temp = 0xF4;
	WriteI2C(1, &temp);
	ReadI2C(1, &temp);
	if (temp & BIT5)
	{
		return false;
	}

	temp = 0xF6;
	WriteI2C(1, &temp);
	ReadI2C(2, (unsigned char*)&s_raw_ut);
	s_raw_ut = _swap_bytes(s_raw_ut);

	return true;
}

// 启动压力测量
void StartPressure_BMP085()
{
	unsigned char	contrl[2];

	contrl[0] = 0xF4;
	contrl[1] = 0x34 + (oversampling_setting << 6);
	WriteI2C(2, contrl);
}

bool GetPressure_BMP085()
{
	unsigned char	temp;

	// 检查状态位（BMP085文档没有说明的，用于代替EOC检测）
	temp = 0xF4;
	WriteI2C(1, &temp);
	ReadI2C(1, &temp);
	if (temp & BIT5)
	{
		return false;
	}

	temp = 0xF6;
	WriteI2C(1, &temp);
	ReadI2C(3, s_raw_up_bytes);

	s_raw_up = (((unsigned long)s_raw_up_bytes[0]<<16) | ((unsigned long)s_raw_up_bytes[1]<<8) | ((unsigned long)s_raw_up_bytes[2])) >>(8-oversampling_setting);

	return true;
}

// 温度和压力计算
void CalculateBMP085Data()
{
	//calculate true temperature
	x1 = ((long) s_raw_ut - s_CalData.ac6) * s_CalData.ac5 >> 15;
	x2 = ((long) s_CalData.mc << 11) / (x1 + s_CalData.md);
	b5 = x1 + x2;
	g_nTemperature_BMP085 = (int)((b5 + 8) >> 4);

	//calculate true pressure
	b6 = b5 - 4000;
	x1 = (s_CalData.b2 * (b6 * b6 >> 12)) >> 11;
	x2 = s_CalData.ac2 * b6 >> 11;
	x3 = x1 + x2;
	b3 = ((((long) (s_CalData.ac1) * 4 + x3)<<oversampling_setting) + 2) >> 2;
	x1 = s_CalData.ac3 * b6 >> 13;
	x2 = (s_CalData.b1 * (b6 * b6 >> 12)) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	b4 = (s_CalData.ac4 * (unsigned long) (x3 + 32768)) >> 15;
	b7 = ((unsigned long) s_raw_up - b3) * (50000 >> oversampling_setting);
	p = b7 < 0x80000000 ? (b7 * 2) / b4 : (b7 / b4) * 2;

	x1 = (p >> 8) * (p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;
	p = p + ((x1 + x2 + 3791) >> 4);

	g_nPressure = (int)(p / 10);
}


// 读取指定字节数，返回读取的字节数
int ReadI2C(int byteCount, unsigned char* bytes)
{
	int		i;

	while (UCB1CTL1 & UCTXSTP);	// 确认上次传输STOP完成

	UCB1CTL1 &= ~UCTR;			// I2C RX
	UCB1CTL1 |= UCTXSTT;		// I2C RX，发送Start

	while (UCB1CTL1 & UCTXSTT);	// 等待Start发送完成

	// 循环读取指定字节数
	i = 0;
	do
	{
		if (byteCount == 1)
		{
			UCB1CTL1 |= UCTXSTP;		// Stop，最后一个字节接收前要设置stop，否则会多收一个字节
		}
		byteCount--;

		while (!(UC1IFG & UCB1RXIFG));
		bytes[i] = UCB1RXBUF;
		i++;

	} while (byteCount != 0);

	return i;
}

// 写入指定字节数
int WriteI2C(int byteCount, unsigned char* bytes)
{
	int		i;

	while (UCB1CTL1 & UCTXSTP);	// 确认上次传输STOP完成

	UCB1CTL1 |= UCTR + UCTXSTT;	// I2C TX，发送Start

	//while (UCB1CTL1 & UCTXSTT);	// 等待Start发送完成

	// 循环发送指定字节数
	for (i = 0; i < byteCount; i++)
	{
		UCB1TXBUF = bytes[i];
		while (!(UC1IFG & UCB1TXIFG));
	}

	UCB1CTL1 |= UCTXSTP;		// Stop
	UC1IFG &= ~UCB1TXIFG;

	return i;
}
