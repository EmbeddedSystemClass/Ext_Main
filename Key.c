#include "Macro.h"
#include "Process.h"

#pragma vector=PORT1_VECTOR
__interrupt void P1Interrupt()
{
	// 判断上次按键后，是否已经经过2秒。

	// 清除IFG
	P1IFG &= ~(BIT0);

	GPS_LED_TOGGLE;
}
