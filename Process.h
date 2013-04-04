/*
 * Process.h
 *
 *  Created on: 2013-1-17
 *      Author: Daniel
 */

#ifndef PROCESS_H_
#define PROCESS_H_

#include <msp430.h>
#include <stdbool.h>

void Process();

void StartMeasure();

void SolarOn();
void SolarOff();

void LED_On();
void LED_off();

void CheckVoltage();

bool TestLED_Proc();

void TestLED_MeasureFailed();
void TestLED_MeasureSuccess();
void TestLED_SendSuccess();
void TestLED_SendFailed();

extern unsigned char			g_ucState;


// 消息ID（无线消息）
#define		MSGID_MAIN_INFO				1			// 主分机消息（带大气压）


#endif /* PROCESS_H_ */
