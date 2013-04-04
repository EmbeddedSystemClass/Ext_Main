/*
 * SPI.h
 *
 *  Created on: 2013-1-18
 *      Author: Daniel
 */

#ifndef SPI_H_
#define SPI_H_

#include <msp430.h>

void InitSPI();

unsigned char SPIRead(unsigned char ucCommand, unsigned char* pData, unsigned char ucLength);
unsigned char SPIWrite(unsigned char ucCommand, unsigned char* pData, unsigned char ucLength);



#endif /* SPI_H_ */
