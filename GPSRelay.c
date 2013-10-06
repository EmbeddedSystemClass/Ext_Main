/*
 * GPSRelay.c
 *
 *  Created on: 2013-10-5
 *      Author: Daniel
 */

#include "GPSRelay.h"
#include "Macro.h"
#include <string.h>



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
void GetDateTime();
int dayofweek();
int string2number(char c1, char c2);
void NeedTurnOffGPS();


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
		rxPos++;
		pRXBuffer[rxPos] = 0;		// 多写个结束符，避免后续处理字符串异常

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
		if (pRXBuffer == GPRMC)
		{
			GetDateTime();
		}

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

int		hour;
int		minute;
int		year;
int		month;
int		day;
int		weekday;

int		isManual;

void GetDateTime()
{
	// 从GPRMC中分离处理UCT时间，并计算出星期几（要判断当前RMC是否有效）
	char*	p;
	int		index;

	p = GPRMC;
	index = 0;

	while (*p != 0x0D)
	{
		if (*p == ',')
		{
			switch (index)
			{
			case 0:			// 时间
				hour = string2number(*(p + 1), *(p + 2));
				if (hour < 0)
					return;
				minute = string2number(*(p + 3), *(p + 4));
				if (minute < 0)
					return;

				break;

			case 1:			// 定位状态（A：有效）、
				if (*(p + 1) != 'A')
				{
					// 非定位状态，时间不可靠
					return;
				}
				break;

			case 8:			// 日期
				day = string2number(*(p + 1), *(p + 2));
				month = string2number(*(p + 3), *(p + 4));
				year = string2number(*(p + 5), *(p + 6)) + 2000;
				// 日期不需要分析有效性，因为在定位状态时，就已经会跳出判断了。

				// 分析星期几
				weekday = dayofweek();

				// 检查是否需要关闭GPS
				NeedTurnOffGPS();

				return;
			}

			index++;
		}

		p++;
	}
}


/**
 * dayofweek - 蔡勒公式 calculate the week day of one day
 * @return: 0 Sun, 1 Mon, 2 Tue, 3 Wed, 4 Thu, 5 Fri, 6 Sat
 */
int dayofweek()
{
	static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
	year -= month < 3;
	return (year + year/4 - year/100 + year/400 + t[month-1] + day) % 7;
}

int string2number(char c1, char c2)
{
	if ((c1 < '0') || (c1 > '9') || (c2 < '0') || (c2 > '9'))
	{
		// 有不合法字符
		return -1;
	}

	return (c1 - '0') * 10 + (c2 - '0');
}

// 开启GPS。1：手动启动GPS，0：自动启动
void TurnOnGPS(int manual)
{
	GPS_LED_ON;
	GPS_ON;

	isManual = manual;
}

void TurnOffGPS()
{
	GPS_LED_OFF;
	GPS_OFF;
}

// 判断是否需要开启或关闭GPS
void NeedTurnOffGPS()
{
	// 判断是否手动启动，如果是，就不自动关闭
	if (isManual)
		return;

	// 不是星期天或星期一（UTC时间），就关闭GPS。（GPS预计开启时间为：UTC星期天22点～星期一0：15。）
	if (weekday == 0)
	{
		if (hour > 20)	// 中国时间大概周一凌晨4点前，关闭GPS
		{
			// 已经是中国时间周一凌晨5点了，不关闭GPS
			return;
		}
	}
	else if (weekday == 1)
	{
		if ((hour == 0) && (minute < 15))
		{
			// 还没有到中国时间周一8：15，不关闭GPS
			return;
		}
	}

	// 其他时间都关闭GPS
	TurnOffGPS();
}


