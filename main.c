
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
#include "UART.h"
#include "GPSRelay.h"


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


    /*
     * Basic Clock System Control 2
     *
     * SELM_0 -- DCOCLK
     * DIVM_0 -- Divide by 1
     * SELS -- XT2CLK when XT2 oscillator present. LFXT1CLK or VLOCLK when XT2 oscillator not present
     * DIVS_0 -- Divide by 1
     * ~DCOR -- DCO uses internal resistor
     *
     * Note: ~DCOR indicates that DCOR has value zero
     */
    BCSCTL2 = SELM_0 + DIVM_0 + SELS + DIVS_0;

    if (CALBC1_1MHZ != 0xFF) {
        /* Follow recommended flow. First, clear all DCOx and MODx bits. Then
         * apply new RSELx values. Finally, apply new DCOx and MODx bit values.
         */
        DCOCTL = 0x00;
        BCSCTL1 = CALBC1_1MHZ;      /* Set DCO to 1MHz */
        DCOCTL = CALDCO_1MHZ;
    }

    /*
     * Basic Clock System Control 1
     *
     * ~XT2OFF -- Enable XT2CLK
     * ~XTS -- Low Frequency
     * DIVA_0 -- Divide by 1
     *
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    BCSCTL1 |= DIVA_0;

    BCSCTL1 &= ~XT2OFF;

    /*
     * Basic Clock System Control 3
     *
     * XT2S_2 -- 3 - 16 MHz
     * LFXT1S_0 -- If XTS = 0, XT1 = 32768kHz Crystal ; If XTS = 1, XT1 = 0.4 - 1-MHz crystal or resonator
     * XCAP_1 -- ~6 pF
     */
    BCSCTL3 = XT2S_2 + LFXT1S_0 + XCAP_1;


	InitPort();

	InitADC();

	InitSPI();

	InitRF();

	InitSensors();

	Init_BMP085();

	InitTimer();

	InitGPSRelay();
	InitUARTA0();
	InitUARTA1();


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
	P3SEL &= ~(BIT0);				//CSN
	P3DIR |= BIT0;
	P3OUT |= BIT0;					//CSN high
	P2SEL &= ~(BIT6);				// P2.6 CE
	P2DIR |= BIT6;
	P2OUT &= ~BIT6;					// CE low

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

	// GPS模块（P4.0：控制3.3v电源模块，0：禁用，1：输出3.3v。 P3.7：LED指示灯输出，0：亮。 )
	//        (P3.5 A0串口RX（功能引脚，GPS 9600）。 P3.6  A1串口TX（功能引脚，4800））
	//		  (P1.0按键输入（中断，上升沿，下拉启用）
	P4DIR |= BIT0;
	P4SEL &= ~(BIT0);
	GPS_OFF;

	P3DIR |= BIT7;
	P3SEL &= ~(BIT7);
	GPS_LED_OFF;

	P3SEL |= BIT5;		// UA0 RX(GPS IN)
	P3SEL |= BIT6;		// UA1 TX

	P1DIR &= ~(BIT0);	//input
	P1SEL &= ~(BIT0);
	P1REN |= BIT0 ;		//Enable pull down/pull up
	P1OUT &= ~(BIT0);	//pull down
	P1IES &= ~(BIT0);	//上升沿触发
	P1IE |= BIT0;		//允许中断


	//Wind Power

	//Wind Power LED(PWM)

}



//shutdown the module which don't work, to save power.
void ShutdownModule()
{
}


// 没有使用的中断（如果出现，则复位）
#pragma vector=ADC12_VECTOR, COMPARATORA_VECTOR, NMI_VECTOR, TIMERB0_VECTOR, TIMERB1_VECTOR, USCIAB1RX_VECTOR, USCIAB0TX_VECTOR, WDT_VECTOR
__interrupt void ISR_trap(void)
{
	// the following will cause an access violation which results in a PUC reset
	WDTCTL = 0;
}

