
#include <msp430.h>
#include "main.h"
#include "Macro.h"
#include "adc.h"
#include "timer.h"
#include "SPI.h"
#include "SHT11.h"
#include "BMP085.h"
#include "nRF24L01P.h"
#include "Process.h"

int main( void )
{
  	Init();

	//delay
	__delay_cycles(200000);

	//switch to idel mode
	__low_power_mode_3();
	
	return 0;
}

//Initialize system
void Init()
{
	IFG1 &= ~OFIFG;

	// Stop watchdog timer to prevent time out reset
    WDTCTL = WDTPW + WDTHOLD;

	//Adjust DCO frequency
    //DCOCTL	= DCO0 + DOC1 +DOC2;
    DCOCTL	= 0x80;
	BCSCTL1	= XT2OFF + RSEL0 + RSEL1 + RSEL2;
    //BCSCTL1	= RSEL0 + RSEL1 + RSEL2;

	BCSCTL2	= 0;		// 实用外部晶振（16M）

	//BCSCTL2 = SELM_2;
	//BCSCTL3 |= XT2S_2;


	InitPort();

	InitADC();

	InitSPI();

	InitRF();

	InitSensors();

	Init_BMP085();

	InitTimer();


	ShutdownModule();
}


void InitPort()
{
	//Default to output first.
	P1SEL = 0;
	P1DIR = 0xFF;
	P1OUT = 0;

	P2SEL = 0;
	P2DIR = 0xFF;
	P2OUT = 0;

	P3SEL = 0;
	P3DIR = 0xFF;
	P3OUT = 0;

	P4SEL = 0;
	P4DIR = 0xFF;
	P4OUT = 0;

	P5SEL = 0;
	P5DIR = 0xFF;
	P5OUT = 0;

	P6SEL = 0;
	P6DIR = 0xFF;
	P6OUT = 0;


	//LED inside box
	P3DIR |= BIT4;		//Output
	P3SEL &= ~BIT4;		//GPIO function
	TEST_LED_OFF;

	//Solar control Port
	P4DIR |= BIT1;
	P4SEL &= ~BIT1;
	P4OUT &= ~BIT1;		//output 0

	//default:turn on charge switcher
	SOLAR_ON;

	//LED control Port(4)
	P4DIR |= BIT2 + BIT3 + BIT4 + BIT5;
	P4SEL &= ~(BIT2 + BIT3 + BIT4 + BIT5);
	LAMP_OFF;

	//TEST
	//P4OUT |= (BIT2 + BIT3 + BIT4 + BIT5);
	//LAMP_ON; (don't user this. Otherwise, the state will be wrong.)
	LED_On();

	//Solar battery Voltage Port
	P6DIR &= ~BIT0;		//Input;
	P6SEL |= BIT0;		//ADC function.

	//12V PB Battery Voltage Port
	P6DIR &= ~BIT1;
	P6SEL |= BIT1;

	//Charge current Port
	P6DIR &= ~BIT2;
	P6SEL |= BIT2;

	//SPI(USART0 3 wire SPI Mode)(//nRF24L01+)
	P3SEL &= ~(BIT0 + BIT5);		//CSN and CE
	P3DIR |= BIT0 + BIT5;
	P3OUT &= ~BIT5;					//CE low
	P3OUT |= BIT0;					//CSN high

	P2SEL &= ~BIT7;					//IRQ
	P2DIR &= ~BIT7;
	P2IES |= BIT7;					//fall edge interrupt
	P2IE  |= BIT7;					//Enable P2.7 interrupt

	P3SEL |= BIT1 + BIT2 + BIT3;	//SPI(SIMO/SOMI/UCLK)


	//Sensor(SHT11 Temperature & humidity)
	P5SEL &= ~(BIT6 + BIT7);
	P5DIR |= BIT6;				//for SCK
	P5OUT &= ~BIT6;
	P5DIR |= BIT7;				//for DATA, thir-state first.

	// 初始化气压传感器（IIC总线）
	P5DIR |= BIT0;					// P5.0 BMP085 XCLR output(Low active)
	P5SEL |= BIT1 + BIT2;			// I2C总线


	//Wind Power

	//Wind Power LED(PWM)

}



//shutdown the module which don't work, to save power.
void ShutdownModule()
{
}


// 没有使用的中断（如果出现，则复位）
#pragma vector=ADC12_VECTOR, COMPARATORA_VECTOR, NMI_VECTOR, PORT1_VECTOR, TIMERB0_VECTOR, TIMERB1_VECTOR, USCIAB0RX_VECTOR, USCIAB0TX_VECTOR, USCIAB1RX_VECTOR, USCIAB1TX_VECTOR, WDT_VECTOR
__interrupt void ISR_trap(void)
{
	// the following will cause an access violation which results in a PUC reset
	WDTCTL = 0;
}

