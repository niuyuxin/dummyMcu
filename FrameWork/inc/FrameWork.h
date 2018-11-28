/********************************************************************************************************
* Filename   : FrameWork.h
* Programmer : 
* 描述：　基于优先级的多线程 头文件。
*********************************************************************************************************
*/

#ifndef FRAMEWORK_H
#define FRAMEWORK_H
#include "stdint.h"

#define NULLTASK	0
#define THREAD_UART	8
#define THREAD_LOGICCTRL	7
#define THREAD_TICK_SEC		1
#define TIMER_TICK_COUNT_MAX 10
#define TIMER_TICK_THREAD_LED 0
#define TIMER_TICK_THREAD_DUMMYEV_CHARGING 1
#define TIMER_TICK_THREAD_DUMMYEV_STOPPED 2
#define TIMER_TICK_THREAD_DUMMYEV_PERIOD 3

extern uint16_t timerTickCount[TIMER_TICK_COUNT_MAX];
extern uint8_t ThreadReadyList;                           //高位优先级高

#define threadSet(prio)  (ThreadReadyList |= (1<<(prio-1))) //置就绪标志
#define	timeTickStart(num, val) (timerTickCount[num] = val)
#define timerTickStop(num) (timerTickCount[num] = 0U)
void threadScheduler(void);                     //调度,这个函数要循环执行

typedef int (*thread_t)(void); 
typedef int (*timerTickFun_t)(void);

void thread_init(void);
void creatTimerTickFun(timerTickFun_t f, int prio);
void creatThread(thread_t f, int prio);

#endif
