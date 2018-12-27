

#include "Uart03_timer0_init.h"
#include "uart2_nCCS.h"
#include "framework.h"
#include "dummyev.h"
#include "lpc177x_8x_gpio.h"
#include "debuguart.h"

int led_blink(void) {
	static int v;
	timeTickStart(TIMER_TICK_THREAD_LED, 20);
	v = v ? 0:1;
	GPIO_SetDir(3, 1<<24, 1); // led blinky
	GPIO_SetDir(1, 1<<19, 1); // led blinky
	GPIO_OutputValue(3, 1<<24, v);
	GPIO_OutputValue(1, 1<<19, !v);
	return 0;
}
int main(void)
{	
	thread_init();
	uart03_timer_init();
	uart2_nCCS_init();
	uart1_debug_init();
	creatTimerTickFun(led_blink, TIMER_TICK_THREAD_LED);
	timeTickStart(0, 20); // ÉÁµÆÆô¶¯
	initDummyEv();
	while (1) {
		threadScheduler();
	}
}
