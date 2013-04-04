/*
 * nRF24L01P.c
 *
 *  Created on: 2013-1-17
 *      Author: Daniel
 */
//nRF24L01P.c

#include "nRF24L01P.h"
#include "SPI.h"
#include "Process.h"
#include "adc.h"
#include "SHT11.h"
#include "BMP085.h"

static unsigned int			s_unSerialNo = 0;

static union struSendData	s_SendData;

#define CE_0		P3OUT &= ~BIT5
#define CE_1		P3OUT |= BIT5

//initialize nRF24L01+
void InitRF()
{
	unsigned char	ucData[6] = {0};

	//CONFIG(all interrupt, CRC 2 bytes, TX mode)
	ucData[0] = EN_CRC + CRCO;
	SPIWrite((WRITE_REG + CONFIG), ucData, 1);

	//SETUP_AW(default 5bytes)

	//SETUP_RETR(2000us * 6 times)
	ucData[0] = ((7<<4) + 6);
	SPIWrite((WRITE_REG + SETUP_RETR), ucData, 1);

	//RF_CH(Debug:70, Release:70)
#ifdef NDEBUG
	ucData[0] = 70;
#else
	ucData[0] = 70;
#endif
	SPIWrite((WRITE_REG + RF_CH), ucData, 1);

	//RF_SETUP(250Kbps)
	ucData[0] = RF_PWR1 + RF_PWR0 + RF_DR_LOW;
	SPIWrite((WRITE_REG + RF_SETUP), ucData, 1);

	//Address(0x3B4B5B6B 01)
	//RX_ADDR_P0, TX_ADDR
	ucData[0] = 0x01;
	ucData[1] = 0x6B;
	ucData[2] = 0x5B;
	ucData[3] = 0x4B;
	ucData[4] = 0x3B;


	SPIWrite(WRITE_REG + RX_ADDR_P0, ucData, 5);
	SPIWrite(WRITE_REG + TX_ADDR, ucData, 5);

	//DYNPD(P0 DPL)
	ucData[0] = BIT0;
	SPIWrite(WRITE_REG + DYNPD, ucData, 1);

	//FEATURE(EN_DPL, EN_ACK_PAY)
	ucData[0] = EN_DPL + EN_ACK_PAY;
	SPIWrite(WRITE_REG + FEATURE, ucData, 1);

	//if warm reset, then need clean TX/RX buffer
	SPIWrite(FLUSH_RX, 0, 0);
	SPIWrite(FLUSH_RX, 0, 0);
	SPIWrite(FLUSH_RX, 0, 0);

	SPIWrite(FLUSH_TX, 0, 0);
	SPIWrite(FLUSH_TX, 0, 0);
	ucData[0] = SPIWrite(FLUSH_TX, 0, 0);

	//clean IRQ on nRF24L01+
	ucData[0] =0x70;
	SPIWrite(WRITE_REG + STATUS, ucData, 1);

	//reset ifg
	P2IFG &= ~BIT7;
}

/****************************
Send Data
1.Battery Voltage
2.Solar Voltage
3.Charge Current
4.Temperature
5.Humidity
6.State(include:Solar Charge State,LED State)
*****************************/
void SendData()
{
	unsigned char	ucData;

	//power on
	ucData = EN_CRC + CRCO + PWR_UP;
	ucData = SPIWrite(WRITE_REG + CONFIG, &ucData, 1);

	//prepare the payload
	s_SendData.Package.unSerial		= s_unSerialNo;
	s_SendData.Package.ucMsgID		= MSGID_MAIN_INFO;
	s_SendData.Package.ucState		= g_ucState;
	s_SendData.Package.nSolar		= g_nSolar;
	s_SendData.Package.nBattery 	= g_nBattery;
	s_SendData.Package.nCharge		= g_nCharge;
	s_SendData.Package.nTemperature	= g_nTemperature;
	s_SendData.Package.nHumidity	= g_nHumidity;
	s_SendData.Package.nPressure	= g_nPressure;
	s_SendData.Package.nTemperature_BMP085 = g_nTemperature_BMP085;

	//Write Payload
	ucData = SPIWrite(WR_TX_PLOAD, (unsigned char*)&(s_SendData.Package), sizeof(struct struSolarCtrl));

	//CE on for 10+us
	CE_1;
	__delay_cycles(200);
	CE_0;

	//s_unSerialNo++;(except 0, must ACK, then add to 1)
}

//nRF24L01+ ISR  Port2.7
#pragma vector=PORT2_VECTOR
__interrupt void Port2_ISR()
{
	//check whether nRF24L01+ interrupt
	if (P2IFG & BIT7)
	{
		P2IFG &= ~BIT7;	//clear IFG
		nRF_ISR();
	}
}

void nRF_ISR()
{
	//read status
	unsigned char	ucStatus;
	unsigned char	ucTemp;

	ucStatus = SPIRead(NRF_NOP, 0, 0);

	if (ucStatus & MAX_RT)
	{
		//Send failed, need flush_tx
		nRF_ISR_MAXRT(ucStatus);
	}

	if (ucStatus & TX_DS)
	{
		//Send success
		nRF_ISR_DS(ucStatus);
	}

	if (ucStatus & RX_DR)
	{
		//have ACK payload
		nRF_ISR_DR(ucStatus);
	}

	//clean IRQ on nRF24L01+
	ucStatus = 0x70;
	SPIWrite(WRITE_REG + STATUS, &ucStatus, 1);

	//Power off
	ucTemp = EN_CRC + CRCO;
	SPIWrite((WRITE_REG + CONFIG), &ucTemp, 1);
}

void nRF_ISR_MAXRT(unsigned char ucStatus)
{
	TestLED_SendFailed();

	//clear the data which was sent failed.
	SPIWrite(FLUSH_TX, 0, 0);

	if (s_unSerialNo != 0)
	{
		//when serial no is "0", this packet must be sent successully.
		//so, if "0" package sent failed, then can't increase.
		s_unSerialNo++;

		//"0" package can stand for "reboot".
	}
}

void nRF_ISR_DS(unsigned char ucStatus)
{
	TestLED_SendSuccess();

	//increase serial no.
	s_unSerialNo++;

	if (s_unSerialNo == 0)
	{
		//skip 0.
		s_unSerialNo++;
	}
}

void nRF_ISR_DR(unsigned char ucStatus)
{
	TestLED_SendSuccess();

	//It is unuseful now. need FLUSH_RX
	SPIWrite(FLUSH_RX, 0, 0);
}



