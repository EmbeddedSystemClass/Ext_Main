/*
 * Process.c
 *
 *  Created on: 2013-1-17
 *      Author: Daniel
 */

#include "Process.h"
#include "Macro.h"
#include "adc.h"
#include "SHT11.h"
#include "BMP085.h"


//state for LED/Solar charge/other
//BIT0:	LED(Lamp)
//BIT1: Solar Charge
unsigned char			g_ucState = 0;

static unsigned char	s_ucTestLEDState = 0;

static unsigned int		s_unProcessStep = 0;
static unsigned int 	s_unRetry = 0;
static unsigned int		s_unLEDCount = 0;


#define STEP_NONE			0x00
#define STEP_REF_ON			0x01
#define STEP_TEMPERATURE	0x02
#define STEP_HUMIDITY		0x04
#define STEP_TEST_LED		0x08
#define STEP_BMP085_TEMPERATURE	0x10
#define STEP_BMP085_PRESSURE	0x20

//for s_ucTestLEDState
#define LED_MEASURE_SUCCESS	0x01
#define LED_MEASURE_FAILED	0x02
#define LED_SEND_SUCCESS	0x03
#define LED_SEND_FAILED		0x04



void Process()
{
	if (s_unProcessStep == STEP_NONE)
	{
		return;
	}

	//Test LED control
	if (s_unProcessStep & STEP_TEST_LED)
	{
		if (TestLED_Proc())
		{
			//finished.
			s_unProcessStep &= ~STEP_TEST_LED;
			TEST_LED_OFF;
		}
	}

	if (s_unProcessStep & STEP_REF_ON)
	{
		CheckVoltage();

		s_unProcessStep &= ~STEP_REF_ON;
	}

	if (s_unProcessStep & STEP_TEMPERATURE)
	{
		if (s_unRetry == 0)
		{
			//Timeout to get data, cacel
			s_unProcessStep &= ~STEP_TEMPERATURE;

			TestLED_MeasureFailed();
		}
		else
		{
			s_unRetry--;

			if (GetTemperature())
			{
				//next step is getting humidity.
				StartHumidity();
				s_unProcessStep &= ~STEP_TEMPERATURE;
				s_unProcessStep |= STEP_HUMIDITY;

				s_unRetry = 10;
			}
		}
	}

	if (s_unProcessStep & STEP_HUMIDITY)
	{
		if (s_unRetry == 0)
		{
			//Timeout to get data, cancel
			s_unProcessStep &= ~STEP_HUMIDITY;

			TestLED_MeasureFailed();
		}
		else
		{
			s_unRetry--;

			if (GetHumidity())
			{
				//finish SHT11 measure
				s_unProcessStep &= ~STEP_HUMIDITY;

				// 进行温度和湿度的换算，计算出正确的问题，以及相对湿度（暂时不在分机转换，因为1122分机是汇编写的，无法进行浮点运输。还是要到Center运输后，再通过串口送到主机）

				TestLED_MeasureSuccess();
			}
		}
	}

	if (s_unProcessStep & STEP_BMP085_TEMPERATURE)
	{
		s_unProcessStep &= ~STEP_BMP085_TEMPERATURE;

		// 读取温度，继续压力测量
		if (GetTemperature_BMP085())
		{
			StartPressure_BMP085();
			s_unProcessStep |= STEP_BMP085_PRESSURE;
		}
	}
	else if (s_unProcessStep & STEP_BMP085_PRESSURE)
	{
		s_unProcessStep &= ~STEP_BMP085_PRESSURE;

		// 读取压力值，换算温度和压力
		if (GetPressure_BMP085())
		{
			CalculateBMP085Data();
		}
	}
}

void StartMeasure()
{
	//Start SHT11 first
	StartTemperature();
	s_unProcessStep |= STEP_TEMPERATURE;
	s_unRetry = 10;		//about 10 * 62.5 = 625ms

	// 启动BMP085温度测量
	StartTemperature_BMP085();
	s_unProcessStep |= STEP_BMP085_TEMPERATURE;		// 每个周期62.5ms，应该足够完成最长时间的压力测量


	//perpare reference voltage
	RefOn();
	s_unProcessStep |= STEP_REF_ON;
}



//turn on solar charger switch
void SolarOn()
{
	if (!(g_ucState & SOLAR_STATE))
	{
		SOLAR_ON;
		g_ucState |= SOLAR_STATE;
	}
}

//turn off solar charge switch
void SolarOff()
{
	if (g_ucState & SOLAR_STATE)
	{
		SOLAR_OFF;
		g_ucState &= ~SOLAR_STATE;
	}
}

void LED_On()
{
	if (!(g_ucState & LAMP_STATE))
	{
		LAMP_ON;
		g_ucState |= LAMP_STATE;
	}
}

void LED_Off()
{
	if (g_ucState & LAMP_STATE)
	{
		LAMP_OFF;
		g_ucState &= ~LAMP_STATE;
	}
}

// value = voltage * 4096 / 16.5 (1.5*11) = voltage * 248.2424
//Modified on 2011.05.16(change battery to Li-poly)
//to turn off solar charge(14.3v)
//#define BATTERY_MAX_CHARGE			3549		//14.3V
//#define BATTERY_MAX_CHARGE			3723		//15V
#define BATTERY_MAX_CHARGE				3143		//4.22*3=12.66

//#define BATTERY_MAX_NORMAL			3276		//13.2V
//#define BATTERY_MAX_NORMAL			3549		//14.3V
#define BATTERY_MAX_NORMAL				3128		//4.20*3=12.6

//to turn off the LED when battery is low(11v)
//#define BATTERY_LOW							2730		//11V
//#define BATTERY_LOW_NOLOAD			2855		//11.5V(the voltage without load)
#define BATTERY_LOW						2607		//3.5*3=10.5(turn off LED)

//to turn off LED power
#define SOLAR_LOW_UP				62			//250mv
//to turn on LED power
#define SOLAR_LOW_DOWN				50			//200mv

//static int

//check voltage
void CheckVoltage()
{
	GetVoltage();

	//save power, turn off reference voltage
	RefOff();

	//check whether need turn off charge
	if (g_nBattery >= BATTERY_MAX_CHARGE)
	{
		//too hight, need turn off charge
		SolarOff();
	}
	else if (g_nBattery <= BATTERY_MAX_NORMAL)
	{
		//can turn on charge
		SolarOn();
	}

	//check battery is too low
	if (g_nBattery <= BATTERY_LOW)
	{
		//battery is too low, need turn off LED.
		LED_Off();

		// battery is too low. can't power on until re-charge.
		g_ucState |= BATTERY_LOW_STATE;
	}

	//check whether need turn on LED
	//check whether need turn off LED
	if (g_nSolar >= SOLAR_LOW_UP)
	{
		// Sun raise, turn off LED.
		LED_Off();

		// maybe can re-charge
		g_ucState &= ~BATTERY_LOW_STATE;
	}
	else if ((g_nSolar <= SOLAR_LOW_DOWN) && !(g_ucState & BATTERY_LOW_STATE))
	{
		//when solar battery is low enough, and the battery is re-charged. then can turn on.
		LED_On();
	}

}

//control Test LED
bool TestLED_Proc()
{
	s_unLEDCount--;
	if (s_unLEDCount == 0)
	{
		return true;
	}

	switch (s_ucTestLEDState)
	{
	case LED_MEASURE_SUCCESS:
	case LED_SEND_SUCCESS:
		break;

	case LED_MEASURE_FAILED:
	case LED_SEND_FAILED:
		if ((s_unLEDCount & 0x1) == 0)
		{
			TEST_LED_TOGGLE;
		}
		break;

	default:
		return true;
	}

	return false;
}

bool TestLED_Init()
{
	if (s_unProcessStep & STEP_TEST_LED)
	{
		//Test LED is working
		return false;
	}

	s_unProcessStep |= STEP_TEST_LED;
	TEST_LED_ON;

	return true;
}

void TestLED_MeasureSuccess()
{
	if (TestLED_Init())
	{
		s_ucTestLEDState = LED_MEASURE_SUCCESS;
		s_unLEDCount = 3;	//On(3)3* 62.5 = 187.5ms
	}
}

void TestLED_MeasureFailed()
{
	if (TestLED_Init())
	{
		s_ucTestLEDState = LED_MEASURE_FAILED;
		s_unLEDCount = 6;	//On(2),Off(2),On(2)
	}
}

void TestLED_SendSuccess()
{
	if (TestLED_Init())
	{
		s_ucTestLEDState = LED_SEND_SUCCESS;
		s_unLEDCount = 8;	//On(8)
	}
}

void TestLED_SendFailed()
{
	if (TestLED_Init())
	{
		s_ucTestLEDState = LED_SEND_FAILED;
		s_unLEDCount = 14;	//On(2),Off(2),On(2),Off(2),On(2),Off(2),On(2)
	}
}



