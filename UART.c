/*
 * UART.c
 *
 *  Created on: 2013-10-5
 *      Author: Daniel
 */

#include "UART.h"
#include "GPSRelay.h"

//A0接收9600 GPS数据
void InitUARTA0()
{
    /* Disable USCI */
    UCA0CTL1 |= UCSWRST;

    /*
     * Control Register 1
     *
     * UCSSEL_1 -- ACLK
     * ~UCRXEIE -- Erroneous characters rejected and UCAxRXIFG is not set
     * ~UCBRKIE -- Received break characters do not set UCAxRXIFG
     * ~UCDORM -- Not dormant. All received characters will set UCAxRXIFG
     * ~UCTXADDR -- Next frame transmitted is data
     * ~UCTXBRK -- Next frame transmitted is not a break
     * UCSWRST -- Enabled. USCI logic held in reset state
     *
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    UCA0CTL1 = UCSSEL_1 + UCSWRST;

    /*
     * Modulation Control Register
     *
     * UCBRF_0 -- First stage 0
     * UCBRS_3 -- Second stage 3
     * ~UCOS16 -- Disabled
     *
     * Note: ~UCOS16 indicates that UCOS16 has value zero
     */
    UCA0MCTL = UCBRF_0 + UCBRS_3;

    /* Baud rate control register 0 */
    UCA0BR0 = 3;

    /* Enable USCI */
    UCA0CTL1 &= ~UCSWRST;


    IFG2 &= ~(UCA0RXIFG);
    IE2 |= UCA0RXIE;
}

// A1发送4800 GPS数据
void InitUARTA1()
{
    /* Disable USCI */
    UCA1CTL1 |= UCSWRST;

    /*
     * Control Register 1
     *
     * UCSSEL_2 -- SMCLK
     * ~UCRXEIE -- Erroneous characters rejected and UCAxRXIFG is not set
     * ~UCBRKIE -- Received break characters do not set UCAxRXIFG
     * ~UCDORM -- Not dormant. All received characters will set UCAxRXIFG
     * ~UCTXADDR -- Next frame transmitted is data
     * ~UCTXBRK -- Next frame transmitted is not a break
     * UCSWRST -- Enabled. USCI logic held in reset state
     *
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    UCA1CTL1 = UCSSEL_2 + UCSWRST;

    /*
     * Modulation Control Register
     *
     * UCBRF_0 -- First stage 0
     * UCBRS_3 -- Second stage 3
     * ~UCOS16 -- Disabled
     *
     * Note: ~UCOS16 indicates that UCOS16 has value zero
     */
    UCA1MCTL = UCBRF_0 + UCBRS_3;

    /* Baud rate control register 0 */
    UCA1BR0 = 5;

    /* Baud rate control register 1 */
    UCA1BR1 = 13;

    /* Enable USCI */
    UCA1CTL1 &= ~UCSWRST;

    /*
     * USCI_A1 Interrupt Enable Register
     *
     * UCA1TXIE -- Interrupt enabled
     * ~UCA1RXIE -- Interrupt disabled
     *
     * Note: ~UCA1RXIE indicates that UCA1RXIE has value zero
     */
    UC1IFG &= ~(UCA1TXIFG);
    UC1IE |= UCA1TXIE;
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR_HOOK(void)
{
    ReceiveByte(UCA0RXBUF);
}

#pragma vector=USCIAB1TX_VECTOR
__interrupt void USCI1TX_ISR_HOOK(void)
{
	char	c;

	if (GetNextChar(&c))
	{
		UCA1TXBUF = c;
	}
	else
	{
		UC1IFG &= ~(UCA1TXIFG);
	}
}



