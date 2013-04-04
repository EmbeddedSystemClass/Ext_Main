/*
 * timer.c
 *
 *  Created on: 2013-1-18
 *      Author: Daniel
 */

#include "timer.h"
#include "Macro.h"
#include "Process.h"
#include "nRF24L01P.h"

void InitTimer()
{
	//62.5ms
	TACCR0 = 256;

	//ACLK 32K,8 divider,Up mode,Interrupt enable
	TACTL = TASSEL_1 + ID_3 + MC_1 + TAIE;

	//enable interrupt
	TACCTL0 = CCIE;
}



static unsigned int s_uCounter = 0;
//static unsigned int s_uCounter = 470;

#pragma vector=TIMERA0_VECTOR
__interrupt void TimerA0Interrupt()
{
	//2010.11.1
	//Check state per 62.5
	Process();

	if (s_uCounter >= 480)		//per 30seconds
	{
		s_uCounter = 0;

		//Send data by RF
		SendData();

		//Send data, and do no measure. skin on 8 second.
		//in order to avoid test led confild.
	}
	else
	{
		//check solar/voltage per 128*62.5ms=8seconds
		if ((s_uCounter & 0x7F) == 0)
		{
			//StartMeasure
			StartMeasure();
		}
	}

	s_uCounter++;
}

#pragma vector=TIMERA1_VECTOR
__interrupt void TimerA1Interrupt()
{
	//need check A1 or A2 or TA
	//2010.11.03(Important, TAIV need be accessed, otherwise this interrupt can't stop).
	switch (TAIV)
	{
	case TAIV_TACCR1:
	case TAIV_TACCR2:
	case TAIV_TAIFG:
		break;
	}
}



