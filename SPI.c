/*
 * SPI.c
 *
 *  Created on: 2013-1-18
 *      Author: Daniel
 */

#include "SPI.h"

#define RF_CSN_0		P3OUT &= ~BIT0
#define RF_CSN_1		P3OUT |= BIT0

#define WAIT_RXIFG	do{	__delay_cycles(10);} while (!(IFG2 & UCB0RXIFG))
//#define WAIT_TXIFG	do{	__delay_cycles(10);} while (!(IFG1 & UTXIFG0))


void InitSPI()
{
	//use UCB0
	UCB0CTL0 = UCCKPH | UCMSB | UCMST | UCMODE_0 | UCSYNC; // SPI mode 0, master
	UCB0CTL1 = UCSSEL1;						// SMCLK

	//UCB0BR0 = 16;							//16MHz / 8, about 2MHz for UCLK
	UCB0BR0 = 1;							//2MHz DCO / 1, about 2MHz for UCLK
	UCB0BR1 = 0;

	//

	// 解除复位
	UCB0CTL1 &= ~UCSWRST;
}

//Read from SPI(write 1 byte command first)
//return status
unsigned char SPIRead(unsigned char ucCommand, unsigned char* pData, unsigned char ucLength)
{
	unsigned char	i;
	unsigned char	ucStatus;

	//read RXBUF first, to reset the RXIFG which is set by last operation.
	ucStatus = UCB0RXBUF;

	//begin
	RF_CSN_0;

	//command and read status
	UCB0TXBUF = ucCommand;
	WAIT_RXIFG;
	ucStatus = UCB0RXBUF;

	for (i = 0; i < ucLength; i++)
	{
		UCB0TXBUF = 0xFF;
		WAIT_RXIFG;
		pData[i] = UCB0RXBUF;
	}

	//end
	RF_CSN_1;

	return ucStatus;
}

//Write to SPI, pData include ucCommand+Data.
unsigned char SPIWrite(unsigned char ucCommand, unsigned char* pData, unsigned char ucLength)
{
	unsigned char	i;
	volatile unsigned char	ucTemp;
	unsigned char	ucStatus;

	//read RXBUF first, to reset the RXIFG which is set by last operation.
	ucTemp = UCB0RXBUF;

	//begin
	RF_CSN_0;

	//command and read status
	UCB0TXBUF = ucCommand;
	WAIT_RXIFG;
	ucStatus = UCB0RXBUF;

	for (i = 0; i < ucLength; i++)
	{
		UCB0TXBUF = pData[i];
		WAIT_RXIFG;
		ucTemp = UCB0RXBUF;
	}

	//end
	RF_CSN_1;

	return ucStatus;
}




