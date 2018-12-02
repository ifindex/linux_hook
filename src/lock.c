/*
 *
 *  thread lock
 *
 *  author  : sunjianping    (ifindex@163.com)
 *  date    : 2018/12/01
 *  
 *  Copyright All Reserved 2018-2020 by sunjianping (ifindex@163.com)
 *  
 *
 */
#include "import.h"

static bool g_thread_inited = FALSE;
static pthread_mutex_t g_thread_mutex;
static int g_thread_id  = 0;

#define LOCK_STATUS_ON      1
#define LOCK_STATUS_OFF     0

void lock_init()
{
    pthread_mutexattr_t attr;

    if (TRUE == g_thread_inited) {
        return;
    }

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);   /* enable nested mutex */
    pthread_mutex_init(&g_thread_mutex, &attr);
    g_thread_inited = TRUE;
}

void lock_uninit()
{
    if (FALSE == g_thread_inited) {
        return;
    }
    
    pthread_mutex_destroy(&g_thread_mutex);
    g_thread_inited = FALSE;
}

uint close_interrupt()
{
    int self_tid    = pthread_self();

    if (g_thread_id != self_tid) {
        pthread_mutex_lock(&g_thread_mutex);
        g_thread_id = self_tid;
        return LOCK_STATUS_ON;
    }

    return LOCK_STATUS_OFF;
}

void open_interrupt(uint lock_status)
{
    if (LOCK_STATUS_ON == lock_status) {
        g_thread_id = 0;
        pthread_mutex_unlock(&g_thread_mutex);
    }
}

