/*
 * GPSRelay.h
 *
 *  Created on: 2013-10-5
 *      Author: Daniel
 */

#ifndef GPSRELAY_H_
#define GPSRELAY_H_

#include <msp430.h>

void InitGPSRelay();
void ReceiveByte(char c);

unsigned char GetNextChar(char* pNextChar);

void TurnOnGPS(int manual);
void TurnOffGPS();

#endif /* GPSRELAY_H_ */
