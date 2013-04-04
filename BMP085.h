/*
 * BMP085.h
 *
 *  Created on: 2013-1-19
 *      Author: Daniel
 */

#ifndef BMP085_H_
#define BMP085_H_

#include <msp430.h>
#include <stdbool.h>

void Init_BMP085();
void StartTemperature_BMP085();
bool GetTemperature_BMP085();
void StartPressure_BMP085();
bool GetPressure_BMP085();

void CalculateBMP085Data();

extern int			g_nPressure;				// 10hPa的压力值
extern int			g_nTemperature_BMP085;		// 0.1的温度值


#endif /* BMP085_H_ */
