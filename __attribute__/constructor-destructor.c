
#include <stdio.h>
#include <stdarg.h>

#ifdef ON
// gcc -Wall constructor-destructor.c -D ON
// 函数在main函数开始执行之前执行
static void start(void) __attribute__ ((constructor));
// 函数在main函数执行结束之后执行
static void stop(void) __attribute__ ((destructor));
#else
// gcc -Wall constructor-destructor.c
static void start(void);
static void stop(void);
#endif
void start(void)
{
    printf("hello\n");
}
void stop(void)
{
    printf("goodbye\n");
}

int main(int argc, char *argv[])
{
    printf("start == %p\n", start);
    printf("stop == %p\n", stop);
    return 0;
}