/*
 * adc.c
 *
 *  Created on: 2013-1-17
 *      Author: Daniel
 */


#include "adc.h"

//Solar voltage
int g_nSolar	= 0;
//Battery voltage
int g_nBattery	= 0;
//Charge current
int g_nCharge	= 0;

void InitADC()
{
	//use 1.5v reference voltage. use longest smaple-hold time.
	ADC12CTL0 = SHT0_15 + MSC /*+ ADC12ON*/;
	ADC12CTL1 = ADC12SSEL_2 + SHP + CONSEQ_1;;

	//0:solar voltage, 1:battery, 2:charge current
	ADC12MCTL0 = SREF_1 + 0;
	ADC12MCTL1 = SREF_1 + 1;
	ADC12MCTL2 = SREF_1 + 2 + EOS;
}

//Set Reference Voltage on
void RefOn()
{
	ADC12CTL0 |= REFON;
}

void RefOff()
{
	ADC12CTL0 &= ~REFON;
}

void GetVoltage()
{
	//turn on adc
	ADC12CTL0 |= ADC12ON + ENC + ADC12SC;

	//wait until finish
	do
	{
		__no_operation();
	} while (ADC12CTL1 & ADC12BUSY);

	//turn off adc
	ADC12CTL0 &= ~(ADC12ON + ENC);

	//return voltage
	g_nSolar 	= ADC12MEM0;
	g_nBattery 	= ADC12MEM1;
	g_nCharge	= ADC12MEM2;
}



