/*
 * SHT11.h
 *
 *  Created on: 2013-1-17
 *      Author: Daniel
 */

#ifndef SHT11_H_
#define SHT11_H_

#include <msp430.h>
#include <stdbool.h>

void InitSensors();
void Init_SHT11();

void StartTemperature();
bool GetTemperature();

void StartHumidity();
bool GetHumidity();


//Unprocess Temperature
extern int g_nTemperature;
//Unprocess Humidity
extern int g_nHumidity;



#endif /* SHT11_H_ */
