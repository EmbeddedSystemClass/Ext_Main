/*
 * main.h
 *
 *  Created on: 2013-1-17
 *      Author: Daniel
 */

#ifndef MAIN_H_
#define MAIN_H_


void Init();
void InitPort();
void ShutdownModule();


/*
P1.0

P2.0
P2.1
P2.2
P2.3
P2.4
P2.5
P2.6
P2.7	nRF24L01+ IRQ(near port 3)

P3.0	nRF24L01+ CSN
P3.1	nRF24L01+ MOSI
P3.2	nRF24L01+ MISO
P3.3	nRF24L01+ SCK

P3.4	Test LED
P3.5	nRF24L01+ CE
P3.6	RS232()				（保留）
P3.7	RS232()				（保留）

P4.0
P4.1	Solar control Port
P4.2	LED(outside)
P4.3	LED(outside)
P4.4	LED(outside)
P4.5	LED(outside)

P5.0	BMP085 XCLR（复位管脚）
P5.1	I2C SDA（气压传感器）
P5.2	I2C SCL（气压传感器）
P5.3
P5.4
P5.5
P5.6	SHT11(SCK)
P5.7	SHT11(DATA)


P6.0	Solar battery Voltage Port
P6.1	12V PB Battery Voltage Port
P6.2	Charge current Port
P6.3

*/




#endif /* MAIN_H_ */
