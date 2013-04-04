/*
 * Macro.h
 *
 *  Created on: 2013-1-17
 *      Author: Daniel
 */

#ifndef MACRO_H_
#define MACRO_H_


#define TEST_LED_ON		P3OUT |= BIT4
#define TEST_LED_OFF	P3OUT &= ~BIT4
#define TEST_LED_TOGGLE	P3OUT ^= BIT4

#define LAMP_ON			P4OUT |= BIT2 + BIT3 + BIT4 + BIT5
#define LAMP_OFF		P4OUT &= ~(BIT2 + BIT3 + BIT4 + BIT5)

#define SOLAR_ON		P4OUT |= BIT1
#define SOLAR_OFF		P4OUT &= ~BIT1


#define LAMP_STATE		BIT0
#define SOLAR_STATE		BIT1
#define BATTERY_LOW_STATE	BIT2


#endif /* MACRO_H_ */
