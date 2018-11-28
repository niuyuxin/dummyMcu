/********************************************************************************************************
* Filename   : FrameWork.C
* Programmer : 
* 描述：　基于优先级的多线程
*         只能定义8个优先级，多个线程同时就绪时，先执行最高优先级。
*********************************************************************************************************
*/
#define  FRAMEWORK_GLOBALS
#include "stdint.h"
#include "framework.h"
#include "lpc177x_8x_timer.h"

#define OS_ENTER_CRITICAL()	NVIC_DisableIRQ(TIMER0_IRQn)
#define OS_EXIT_CRITICAL() NVIC_EnableIRQ(TIMER0_IRQn)

uint8_t ThreadReadyList; 

static const uint8_t PRIORITY_TABLE[]= {0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4};
static const uint8_t READY_CLR[] = {0xFF, 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F};
uint16_t timerTickCount[TIMER_TICK_COUNT_MAX];
timerTickFun_t timerTickFun[TIMER_TICK_COUNT_MAX];

thread_t thread_fun[] = {
	NULLTASK,
	NULLTASK,
	NULLTASK,
	NULLTASK,
	NULLTASK,
	NULLTASK,
	NULLTASK,
	NULLTASK
};
int timeTickThread(void)
{	int i;
	for (i = 0; i < sizeof(timerTickCount)/sizeof(uint16_t); i++) {
		if (timerTickCount[i] != 0) {
			if (--timerTickCount[i] == 0) {
				if (timerTickFun[i] != NULLTASK) {
					timerTickFun[i]();
					return 0;
				}
			}
		}
	}
	return -1;
}
void threadScheduler(void)
{
    uint8_t prionum, ready;
	prionum = 0;
	ready = ThreadReadyList;
    if (ready != 0) {
        if ((ready & 0xF0) != 0U) { // 找出就绪线程中的最高优先级的线程
            prionum = PRIORITY_TABLE[ready >> 4] + 4;
        }else{
            prionum = PRIORITY_TABLE[ready];
        }
        ready = READY_CLR[prionum];
        OS_ENTER_CRITICAL();
        ThreadReadyList &= ready;//执行完成，清就绪位
        OS_EXIT_CRITICAL();
		if (thread_fun[prionum-1] != NULLTASK) {
			thread_fun[prionum-1]();
		}
    }
}
void creatTimerTickFun(timerTickFun_t f, int prio)
{
	if (prio < sizeof(timerTickFun)/sizeof(timerTickFun_t)) {
		timerTickFun[prio] = f;
	}
}
void creatThread(thread_t f, int prio)
{
	if (prio <= 8) {
		thread_fun[prio-1] = f;
	}
}
void timer0Init() 
{

	TIM_TIMERCFG_Type TIM_ConfigStruct;
	TIM_MATCHCFG_Type TIM_MatchConfigStruct ;
	
	TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
	TIM_ConfigStruct.PrescaleValue	= 100;
	// use channel 0, MR0
	TIM_MatchConfigStruct.MatchChannel = 0;
	// Enable interrupt when MR0 matches the value in TC register
	TIM_MatchConfigStruct.IntOnMatch   = TRUE;
	//Enable reset on MR0: TIMER will reset if MR0 matches it
	TIM_MatchConfigStruct.ResetOnMatch = TRUE;
	//Stop on MR0 if MR0 matches it
	TIM_MatchConfigStruct.StopOnMatch  = FALSE;
	//Toggle MR0.0 pin if MR0 matches it
	TIM_MatchConfigStruct.ExtMatchOutputType =TIM_EXTMATCH_NOTHING;
	// Set Match value, count value of 10000 (10000 * 100uS = 1000000us = 1s --> 1 Hz)
	TIM_MatchConfigStruct.MatchValue   = 100;

	// Set configuration for Tim_config and Tim_MatchConfig
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &TIM_ConfigStruct);
	TIM_ConfigMatch(LPC_TIM0, &TIM_MatchConfigStruct);
		/* preemption = 1, sub-priority = 1 */
	NVIC_SetPriority(TIMER0_IRQn, ((0x01<<3)|0x01));
	NVIC_EnableIRQ(TIMER0_IRQn);
	TIM_Cmd(LPC_TIM0, ENABLE);
}
/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief		TIMER0 interrupt handler sub-routine
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void TIMER0_IRQHandler(void)
{
	if (TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)== SET)
	{
		threadSet(THREAD_TICK_SEC);
		threadSet(THREAD_UART);
	}
	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
}

void thread_init()
{
	int i = 0;
	timer0Init();
	for (i = 0; i < sizeof(timerTickCount)/sizeof(uint16_t); i++) {
		timerTickCount[i] = 0;
	}
	for (i = 0; i < sizeof(timerTickFun)/sizeof(timerTickFun_t); i++) {
		timerTickFun[i] = NULLTASK;
	}
	creatThread(timeTickThread, THREAD_TICK_SEC);
}
