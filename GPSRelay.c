/*
 * GPSRelay.c
 *
 *  Created on: 2013-10-5
 *      Author: Daniel
 */

#include "GPSRelay.h"
#include <string.h>
#include <stdio.h>

char			GP_HEAD[6];		// 缓存接收的头，用来区分准备存放在哪个缓冲区
unsigned char	head_Pos;		// 当前头部接收的字节

char			GPRMC[70 + 7 + 2];
char			GPGSA[65 + 7 + 2];
#define GPRMC_SIZE sizeof(GPRMC)
#define GPGSA_SIZE sizeof(GPGSA)
#define HEAD_SIZE  sizeof(GP_HEAD)

char*			pRXBuffer;		// 当前正在写入的Buffer
unsigned char	rxPos;
unsigned char	maxSize;

char*			pTXBuffer;		// 当前正在发送的Buffer
char*			pTXBuffer2;		// 备用发送Buffer，当pTXBuffer还在发送时，存放下一个准备发送的Buffer指针
unsigned char	txPos;

void ReceiveHead(char c);
void ReceiveGPxxx(char c);

void InitGPSRelay()
{
	head_Pos = 0;

	pRXBuffer = 0;
	rxPos = 0;
	maxSize = HEAD_SIZE;

	pTXBuffer = 0;
	pTXBuffer2 = 0;
	txPos = 0;

	GPRMC[0] = '$';
	GPRMC[1] = 'G';
	GPRMC[2] = 'P';
	GPRMC[3] = 'R';
	GPRMC[4] = 'M';
	GPRMC[5] = 'C';
	GPRMC[6] = ',';

	GPGSA[0] = '$';
	GPGSA[1] = 'G';
	GPGSA[2] = 'P';
	GPGSA[3] = 'G';
	GPGSA[4] = 'S';
	GPGSA[5] = 'A';
	GPGSA[6] = ',';
}

// 串口接收到一个字节数据
// 过滤GPGSA和GPRMC两行数据，然后存储转发
void ReceiveByte(char c)
{
	if (pRXBuffer == 0)
	{
		// 还在接收头部
		ReceiveHead(c);
	}
	else
	{
		ReceiveGPxxx(c);
	}

}

void ReceiveHead(char c)
{
	switch (head_Pos)
	{
	case 0:
		if (c == '$')
		{
			GP_HEAD[0] = c;
			head_Pos++;
		}
		break;

	case HEAD_SIZE:
		// 判断头部是否匹配GPRMC和GPGSA，其他的就忽略
		if (strncmp(GP_HEAD + 1, "GPRMC", 5) == 0)
		{
			pRXBuffer = GPRMC;
			maxSize = GPRMC_SIZE;
		}
		else if (strncmp(GP_HEAD + 1, "GPGSA", 5) == 0)
		{
			pRXBuffer = GPGSA;
			maxSize = GPGSA_SIZE;
		}
		rxPos = 7;			// 前头7个字节已经在初始化时赋值

		head_Pos = 0;		// 复位，重新等待$（无论是否匹配，都会复位重来）
		break;

	default:
		// 理论上不会越界
		GP_HEAD[head_Pos] = c;
		head_Pos++;
		break;
	}
}

void ReceiveGPxxx(char c)
{
	// 判断是否‘0D’，‘0A’（0A不需要再等待接收，自己填充，然后复位接收Pos）
	if (c == 0x0D)
	{
		// 接收完整
		pRXBuffer[rxPos] = 0x0D;
		rxPos++;
		pRXBuffer[rxPos] = 0x0A;

		// 启动发送
		if (pTXBuffer == 0)
		{
			pTXBuffer = pRXBuffer;
			UCA1TXBUF = pTXBuffer[0];
			txPos = 1;
		}
		else
		{
			// 使用备份Buffer缓存
			pTXBuffer2 = pRXBuffer;
		}

		// 分析GPRMC中的时间

		pRXBuffer = 0;
		rxPos = 7;
	}
	else
	{
		// 判断有没有越界
		if (rxPos >= maxSize)
		{
			// 本次接收失败
			pRXBuffer = 0;
			rxPos = 7;
		}
		else
		{
			pRXBuffer[rxPos] = c;
			rxPos++;
		}
	}
}

// 获取下一个发送字节
unsigned char GetNextChar(char* pNextChar)
{
	if (pTXBuffer[txPos - 1] == 0x0A)
	{
		// 前一个发送的字节已经是最后一个字节
		if (pTXBuffer2 != 0)
		{
			// 有备用发送
			pTXBuffer = pTXBuffer2;
			pTXBuffer2 = 0;
			txPos = 0;
		}
		else
		{
			pTXBuffer = 0;
			txPos = 0;

			return 0;
		}
	}

	// 返回下一个字节
	*pNextChar = pTXBuffer[txPos];
	txPos++;

	return 1;


}


