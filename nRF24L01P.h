/*
 * nRF24L01P.h
 *
 *  Created on: 2013-1-17
 *      Author: Daniel
 */

#ifndef NRF24L01P_H_
#define NRF24L01P_H_

#include <msp430.h>

/** @name - Instruction Set - */
//@{
/* nRF24L01 Instruction Definitions */
#define READ_REG		0x00
#define WRITE_REG     	0x20  /**< Register write command */
#define RD_RX_PLOAD_W   0x60  /**< Read RX payload command */
#define RD_RX_PLOAD   	0x61  /**< Read RX payload command */
#define WR_TX_PLOAD   	0xA0  /**< Write TX payload command */
#define WR_ACK_PLOAD  	0xA8  /**< Write ACK payload command */
#define WR_NAC_TX_PLOAD 0xB0  /**< Write ACK payload command */
#define FLUSH_TX      	0xE1  /**< Flush TX register command */
#define FLUSH_RX      	0xE2  /**< Flush RX register command */
#define REUSE_TX_PL   	0xE3  /**< Reuse TX payload command */
#define LOCK_UNLOCK   	0x50  /**< Lock/unlcok exclusive features */

#define NRF_NOP       	0xFF  /**< No Operation command, used for reading status register */
//@}

/** @name  - Register Memory Map - */
//@{
/* nRF24L01 * Register Definitions * */
#define CONFIG        0x00  /**< nRF24L01 config register */
#define EN_AA         0x01  /**< nRF24L01 enable Auto-Acknowledge register */
#define EN_RXADDR     0x02  /**< nRF24L01 enable RX addresses register */
#define SETUP_AW      0x03  /**< nRF24L01 setup of address width register */
#define SETUP_RETR    0x04  /**< nRF24L01 setup of automatic retransmission register */
#define RF_CH         0x05  /**< nRF24L01 RF channel register */
#define RF_SETUP      0x06  /**< nRF24L01 RF setup register */
#define STATUS        0x07  /**< nRF24L01 status register */
#define OBSERVE_TX    0x08  /**< nRF24L01 transmit observe register */
#define CD            0x09  /**< nRF24L01 carrier detect register */
#define RX_ADDR_P0    0x0A  /**< nRF24L01 receive address data pipe0 */
#define RX_ADDR_P1    0x0B  /**< nRF24L01 receive address data pipe1 */
#define RX_ADDR_P2    0x0C  /**< nRF24L01 receive address data pipe2 */
#define RX_ADDR_P3    0x0D  /**< nRF24L01 receive address data pipe3 */
#define RX_ADDR_P4    0x0E  /**< nRF24L01 receive address data pipe4 */
#define RX_ADDR_P5    0x0F  /**< nRF24L01 receive address data pipe5 */
#define TX_ADDR       0x10  /**< nRF24L01 transmit address */
#define RX_PW_P0      0x11  /**< nRF24L01 \# of bytes in rx payload for pipe0 */
#define RX_PW_P1      0x12  /**< nRF24L01 \# of bytes in rx payload for pipe1 */
#define RX_PW_P2      0x13  /**< nRF24L01 \# of bytes in rx payload for pipe2 */
#define RX_PW_P3      0x14  /**< nRF24L01 \# of bytes in rx payload for pipe3 */
#define RX_PW_P4      0x15  /**< nRF24L01 \# of bytes in rx payload for pipe4 */
#define RX_PW_P5      0x16  /**< nRF24L01 \# of bytes in rx payload for pipe5 */
#define FIFO_STATUS   0x17  /**< nRF24L01 FIFO status register */
#define DYNPD         0x1C  /**< nRF24L01 Dynamic payload setup */
#define FEATURE       0x1D  /**< nRF24L01 Exclusive feature setup */

//@}

/** @name CONFIG register bit definitions */
//@{

#define MASK_RX_DR    (1<<6)     /**< CONFIG register bit 6 */
#define MASK_TX_DS    (1<<5)     /**< CONFIG register bit 5 */
#define MASK_MAX_RT   (1<<4)     /**< CONFIG register bit 4 */
#define EN_CRC        (1<<3)     /**< CONFIG register bit 3 */
#define CRCO          (1<<2)     /**< CONFIG register bit 2 */
#define PWR_UP        (1<<1)     /**< CONFIG register bit 1 */
#define PRIM_RX       (1<<0)     /**< CONFIG register bit 0 */
//@}

/** @name RF_SETUP register bit definitions */
//@{
#define RF_DR_LOW	  (1<<5)
#define PLL_LOCK      (1<<4)     /**< RF_SETUP register bit 4 */
#define RF_DR_HIGHT   (1<<3)     /**< RF_SETUP register bit 3 */
#define RF_PWR1       (1<<2)     /**< RF_SETUP register bit 2 */
#define RF_PWR0       (1<<1)     /**< RF_SETUP register bit 1 */
#define LNA_HCURR     (1<<0)     /**< RF_SETUP register bit 0 */
//@}

/* STATUS 0x07 */
/** @name STATUS register bit definitions */
//@{
#define RX_DR         (1<<6)     /**< STATUS register bit 6 */
#define TX_DS         (1<<5)     /**< STATUS register bit 5 */
#define MAX_RT        (1<<4)     /**< STATUS register bit 4 */
#define TX_FULL       (1<<0)     /**< STATUS register bit 0 */
//@}

/* FIFO_STATUS 0x17 */
/** @name FIFO_STATUS register bit definitions */
//@{
#define TX_REUSE      (1<<6)     /**< FIFO_STATUS register bit 6 */
#define TX_FIFO_FULL  (1<<5)     /**< FIFO_STATUS register bit 5 */
#define TX_EMPTY      (1<<4)     /**< FIFO_STATUS register bit 4 */
#define RX_FULL       (1<<1)     /**< FIFO_STATUS register bit 1 */
#define RX_EMPTY      (1<<0)     /**< FIFO_STATUS register bit 0 */
//@}

/*FEATURE*/
#define EN_DPL		  (1<<2)	//Enables Dynamic Payload Length
#define EN_ACK_PAY    (1<<1)	//Enables Payload with ACK
#define EN_DYN_ACK    (1<<0)	//Enables the W_TX_PAYLOAD_NOACK command


void InitRF();
void nRF_ISR();
void nRF_ISR_MAXRT(unsigned char ucStatus);
void nRF_ISR_DS(unsigned char ucStatus);
void nRF_ISR_DR(unsigned char ucStatus);
void SendData();

struct	struSolarCtrl
{
	unsigned int	unSerial;			// 序号
	unsigned char	ucMsgID;			// 消息ID
	unsigned char	ucState;			// 其他状态

	int				nSolar;				// 太阳能电压
	int				nBattery;			// 电池电压
	int				nCharge;			// 充电电流
	int				nTemperature;		// 温度
	int				nHumidity;			// 湿度
	int				nPressure;			// 大气压（10Pa）
	int				nTemperature_BMP085;// 温度（0.1摄氏度）
};

union struSendData
{
	unsigned char		ucData[32];
	struct struSolarCtrl		Package;
};


#endif /* NRF24L01P_H_ */
