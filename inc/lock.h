/*
 *
 *  thread lock header
 *
 *  author  : sunjianping    (ifindex@163.com)
 *  date    : 2018/12/01
 *  
 *  Copyright All Reserved 2018-2020 by sunjianping (ifindex@163.com)
 *  
 *
 */
#ifndef __LOCK_H__
#define __LOCK_H__

void lock_init();
void lock_uninit();
uint close_interrupt();
void open_interrupt(uint lock_status);


#endif  /* __LOCK_H__ */
