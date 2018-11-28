/********************************************************************************************************
* Filename   : FrameWork.h
* Programmer : 
* ���������������ȼ��Ķ��߳� ͷ�ļ���
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
extern uint8_t ThreadReadyList;                           //��λ���ȼ���

#define threadSet(prio)  (ThreadReadyList |= (1<<(prio-1))) //�þ�����־
#define	timeTickStart(num, val) (timerTickCount[num] = val)
#define timerTickStop(num) (timerTickCount[num] = 0U)
void threadScheduler(void);                     //����,�������Ҫѭ��ִ��

typedef int (*thread_t)(void); 
typedef int (*timerTickFun_t)(void);

void thread_init(void);
void creatTimerTickFun(timerTickFun_t f, int prio);
void creatThread(thread_t f, int prio);

#endif
