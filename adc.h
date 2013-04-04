/*
 * adc.h
 *
 *  Created on: 2013-1-17
 *      Author: Daniel
 */

#ifndef ADC_H_
#define ADC_H_

#include <msp430.h>

void InitADC();
void RefOn();
void RefOff();
void GetVoltage();

//Solar voltage
extern int g_nSolar;
//Battery voltage
extern int g_nBattery;
//Charge current
extern int g_nCharge;


#endif /* ADC_H_ */
