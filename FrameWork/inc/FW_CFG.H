/***************************************************************************
**Filename   : FW_CFG.h　
* Programmer : Haven
**Date       : 2007.2.27
**DESCRIPTION: 
**************************************************************************
*/
#define FW_THREAD_MAX          8

#define FW_MSG_EN         0
#define FW_MSGQ_EN        0

#if FW_MSGQ_EN          //如果定义了事件队列，因为RAM原因，只能有一个队列存在
#define MSGQ_BUF_SIZE        5 //事件队列的大小
#define MSGQ_PRIO            5 //指定队列对应的优先级线程，这个队列为这个线程所用
#endif
