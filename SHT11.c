/*
 * SHT11.c
 *
 *  Created on: 2013-1-17
 *      Author: Daniel
 */

#include "SHT11.h"

//#define DELAY	__delay_cycles(640)			// 16MHz下的延迟
#define DELAY	__delay_cycles(40)			// 1MHz下的延迟

#define SHT11_SCK_1		P5OUT |= BIT6
#define SHT11_SCK_0 	P5OUT &= ~BIT6

#define SHT11_DATA_1	P5OUT |= BIT7
#define SHT11_DATA_0	P5OUT &= ~BIT7
#define SHT11_DATA_IN	P5DIR &= ~BIT7
#define SHT11_DATA_OUT	P5DIR |= BIT7

#define SHT11_DATA_READ	(P5IN & BIT7)


//Unprocess Temperature
int g_nTemperature	= 0;
//Unprocess Humidity
int g_nHumidity		= 0;

static unsigned char s_ucCRC;

void InitSensors()
{
}

void Init_SHT11()
{
}

void StartCommand()
{
	DELAY;
	SHT11_DATA_OUT;
	SHT11_DATA_1;

	SHT11_SCK_1;
	DELAY;
	SHT11_DATA_0;
	DELAY;
	SHT11_SCK_0;
	DELAY;
	SHT11_SCK_1;
	DELAY;
	SHT11_DATA_1;
	DELAY;
	SHT11_SCK_0;
	DELAY;
}

void SendByte(unsigned char data)
{
	int				i;
	unsigned char	bit = 0x80;

	for (i = 0; i < 8; i++)
	{
		DELAY;

		//prepare data;
		if (data & bit)
		{
			//SHT11_DATA_1;
			SHT11_DATA_IN;		//pull up by 10K resistor
		}
		else
		{
			SHT11_DATA_OUT;
			SHT11_DATA_0;
		}

		//raise SCK
		SHT11_SCK_1;
		DELAY;
		SHT11_SCK_0;

		bit = bit >> 1;
	}

	//read ACK
	DELAY;
	SHT11_SCK_1;
	//TODO:check ACK, data is low.
	DELAY;
	SHT11_SCK_0;
}

unsigned char ReadByte(bool isCRC)
{
	unsigned char	data;
	int				i;

	for (i = 0; i < 8; i++)
	{
		DELAY;
		SHT11_SCK_1;

		data = data << 1;
		//read data
		data += (P5IN & BIT7) ? 0x01 : 0x00;

		DELAY;
		SHT11_SCK_0;
	}

	//confirm ACK
	DELAY;
	if (!(isCRC))
	{
		SHT11_DATA_OUT;
		SHT11_DATA_0;
	}
	SHT11_SCK_1;
	DELAY;
	SHT11_SCK_0;

	SHT11_DATA_IN;

	return data;
}

void WaitResult()
{
	while (1)
	{
		DELAY;
		//check DATA is low.
		if (!(SHT11_DATA_READ))
		{
			break;
		}
	}
}

void StartTemperature()
{
	StartCommand();
	SendByte(3);		//Measure Temperature 00011
}

bool GetTemperature()
{
	//check DATA is low.
	if (SHT11_DATA_READ)
	{
		//still high, measuring...
		return false;
	}

	g_nTemperature = ReadByte(false);
	g_nTemperature = g_nTemperature << 8;
	g_nTemperature += ReadByte(false);
	s_ucCRC	= ReadByte(true);

	return true;
}


void StartHumidity()
{
	StartCommand();
	SendByte(5);		//Measure Relative Humidity 00101
}

bool GetHumidity()
{
	//check DATA is low.
	if (SHT11_DATA_READ)
	{
		//still high, measuring...
		return false;
	}

	g_nHumidity = ReadByte(false);
	g_nHumidity = g_nHumidity << 8;
	g_nHumidity += ReadByte(false);
	s_ucCRC	= ReadByte(true);

	return true;
}






