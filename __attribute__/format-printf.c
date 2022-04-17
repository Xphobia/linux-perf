
#include <stdio.h>
#include <stdarg.h>

#ifdef ON
    // 告诉编译器以printf的方式去检查该函数
    // gcc -Wall format-printf.c -D ON
    extern void myprint(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
#else
    // gcc -Wall format-printf.c
    extern void myprint(const char *fmt, ...);
#endif

void myprint(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    (void)printf(fmt, ap);
    va_end(ap);
}

void test(void)
{
    myprint("number = %d\n", 6);
    // warning: format ‘%d’ expects argument of type ‘int’, but argument 2 has type ‘char *’ [-Wformat=]
    myprint("string = %d\n", "hello");
}

int main(void)
{
    test();
    return 0;
}
