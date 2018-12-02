/*
 *
 *  test hook and stub function
 *
 *  author  : sunjianping    (ifindex@163.com)
 *  date    : 2018/12/01
 *  
 *  Copyright All Reserved 2018-2020 by sunjianping (ifindex@163.com)
 *  
 *
 */
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <dlfcn.h>

#define COLOR_RED           "\e[0;31m"
#define COLOR_GREEN         "\e[0;32m"
#define COLOR_NONE          "\e[0m"

extern void x86_hk_init();
extern int x86_hook_func(void*, void*, void*);
extern int x86_stub_func(void*, void*);
extern int x86_unhook_func(void*);
extern int x86_unstub_func(void*);

static void test1_pre(int a, int b)
{
    printf(COLOR_GREEN"----> [test1] before hook, "
           "a: %d, b: %d"COLOR_NONE"\n", a, b);
}

static void test1_aft(int ret, int a, int b)
{
    printf(COLOR_GREEN"----> [test1] after hook, "
           "ret: %d, a: %d, b: %d"COLOR_NONE"\n", ret, a, b);
}

static int test1(int a, int b)
{
    int ret =  a + b;
    printf("[test1] real function, "
           "a: %d, b: %d, ret value: %d\n", a, b, ret);
    return ret;
}

static int test2_stub(int a, int b)
{
    int ret = a * b;
    printf(COLOR_GREEN"----> [test2] stub function, "
           "a: %d, b: %d, ret: %d"COLOR_NONE"\n", a, b, ret);
    return ret;
}

static int test2(int a, int b)
{
    int ret =  a + b;
    printf("[test2] real function, "
           "a: %d, b: %d, ret value: %d\n", a, b, ret);
    return ret;
}

static void * thread_sub(void *arg)
{
    int ret;

    printf("\n-----------------------------------------------------------\n");
    printf("**** original function process **** \n");
    ret = test1(11, 25);
    printf("[test1] return value: %d\n", ret);
    ret = test2(11, 25);
    printf("[test2] return value: %d\n", ret);
    printf("-----------------------------------------------------------\n\n");
    
    printf("-----------------------------------------------------------\n");
    printf("**** after hook or stub function **** \n");
    x86_hk_init();
    ret = x86_hook_func(test1, test1_pre, test1_aft);
    if (0 != ret) {
        printf("hook set failed, ret: %d\n", ret);
        pthread_exit(&ret);
    }
    ret = x86_stub_func(test2, test2_stub);
    if (0 != ret) {
        printf("stub set failed, ret: %d\n", ret);
        (void)x86_unhook_func(test1);
        pthread_exit(&ret);
    }

    ret = test1(11, 25);
    printf("[test1] return value: %d\n", ret);
    ret = test2(11, 25);
    printf("[test2] return value: %d\n", ret);
    printf("-----------------------------------------------------------\n\n");
    
    printf("-----------------------------------------------------------\n");
    printf("**** clear hook and stub  **** \n");
    (void)x86_unhook_func(test1);
    (void)x86_unstub_func(test2);
    ret = test1(11, 25);
    printf("[test1] return value: %d\n", ret);
    ret = test2(11, 25);
    printf("[test2] return value: %d\n", ret);
    printf("-----------------------------------------------------------\n");

    return NULL;
}

int main(int argc, char *argv[])
{
    int     ret;
    pthread_t   tid;
    void    * handle;

    ret = pthread_create(&tid, NULL, thread_sub, NULL);
    if (0 != ret) {
        printf("thread create failed, ret: %d\n", ret);
        goto error;
    }

    ret = pthread_join(tid, NULL);
    if (0 != ret) {
        printf("thread join failed, ret: %d\n",ret);
        goto error;
    }

error:

    return 0;
}

