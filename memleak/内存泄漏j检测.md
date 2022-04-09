
视频：https://www.bilibili.com/video/BV19r4y1z7Y2?p=3

# 什么是内存泄漏
内存泄漏是指，在进程运行过程中，通过调用内存分配函数（如malloc或new）分配内存，但没有调用释放内存函数（如free或delete）释放，导致进程内存占用持续增长，最终可能耗尽堆内存。
 
1. 内存泄漏的原因：malloc和free调用次数不匹配
2. 需要解决的两个问题：
   （1）如何判断是否存在内存泄漏
   （2）如何判断内存泄漏具体在哪块代码
3. 如何实现：
   （1）实现malloc和free的hook
   （2）如果是线上版本，配置文件memleak.conf开关memleak = 1


# 方法或工具
1. valgrind/mtrace
2. hook方法,
3. addr2line工具： addr2line -fe ./memleak -a 0x400ab8



# valgrind工具静态检测

1. 安装valgrind：yum install -y valgrind
2. 准备程序代码：memleak_valgrind.c

```
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    void *p1 = malloc(10);
    free(p1);

    void *p2 = malloc(20);
    
    void *p3 = malloc(30);
    free(p3);

    return 0;
}
```
3. 编译：gcc memleak_valgrind.c -o memleak_valgrind
4. 执行：valgrind ./memleak_valgrind
```
[root@xphobia ~]# valgrind ./memleak_valgrind
==28711== Memcheck, a memory error detector
==28711== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==28711== Using Valgrind-3.15.0 and LibVEX; rerun with -h for copyright info
==28711== Command: ./memleak_valgrind
==28711== 
==28711== 
==28711== HEAP SUMMARY:
==28711==     in use at exit: 20 bytes in 1 blocks
==28711==   total heap usage: 3 allocs, 2 frees, 60 bytes allocated
==28711== 
==28711== LEAK SUMMARY:
==28711==    definitely lost: 20 bytes in 1 blocks
==28711==    indirectly lost: 0 bytes in 0 blocks
==28711==      possibly lost: 0 bytes in 0 blocks
==28711==    still reachable: 0 bytes in 0 blocks
==28711==         suppressed: 0 bytes in 0 blocks
==28711== Rerun with --leak-check=full to see details of leaked memory
==28711== 
==28711== For lists of detected and suppressed errors, rerun with: -s
==28711== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```
从输出结果LEAK SUMMARY可以看出，存在20字节的内存泄漏。


# hook方法检测：打印
原理：通过dlsym函数实现malloc和free的hook函数，通过打印malloc和free调用次数，如果次数不一致，则存在内存泄漏。

1. 准备程序代码：memleak_print.c
```
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>


// hook定义
typedef void *(*malloc_t)(size_t size);
malloc_t malloc_f = NULL;

typedef void *(*free_t)(void *ptr);
free_t free_f = NULL;

// 需置0防止malloc递归调用（例如：printf里面有调用malloc）
int enable_malloc_hook = 1;
int enable_free_hook = 1;

void* malloc(size_t size)
{
    if (enable_malloc_hook)
    {
        enable_malloc_hook = 0;
        printf("malloc\n");
        void *ptr = malloc_f(size);
        enable_malloc_hook = 1;
        return ptr;
    }
    else
    {
        return malloc_f(size);
    }
}

void free(void *ptr)
{
    if (enable_free_hook)
    {
        enable_free_hook = 0;
        printf("free\n");
        free_f(ptr);
        enable_free_hook = 1;
    }
    else
    {
        free_f(ptr);
    }
}

static int init_hook()
{
    malloc_f = dlsym(RTLD_NEXT, "malloc");
    free_f = dlsym(RTLD_NEXT, "free");
}


int main(void)
{
    init_hook();

    void *p1 = malloc(10);
    free(p1);

    void *p2 = malloc(20);
    
    void *p3 = malloc(30);
    free(p3);

    return 0;
}
```
2. 编译：gcc memleak_print.c -o memleak_print -ldl 
3. 执行：./memleak_print
```
[root@xphobia memleak]# ./memleak_print
malloc
free
malloc
malloc
free
```
从输出结果可以看出，malloc和free的调用次数不等，存在内存泄漏。
该方法缺点：只能判断是否存在内存泄漏，不能判断具体代码位置。

# hook方法检测：文件记录

1. 准备程序代码：memleak_file.c
```
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MEM_CONTENT_LENGTH  128

// hook定义
typedef void *(*malloc_t)(size_t size);
malloc_t malloc_f = NULL;

typedef void *(*free_t)(void *ptr);
free_t free_f;

// 需置0防止malloc递归调用（例如：printf里面有调用malloc）
int enable_malloc_hook = 1;
int enable_free_hook = 1;

void* malloc(size_t size)
{
    if (enable_malloc_hook)
    {
        enable_malloc_hook = 0;
        void *ptr = malloc_f(size);
        // malloc被调用时保存信息到文件
        {
            // 0 == f() --> malloc()
            // 1 == a() --> f() --> malloc()
            // 2 == a() --> b() --> f() --> malloc()
            void *caller = (void *)__builtin_return_address(0);
            char buff[MEM_CONTENT_LENGTH] = {0};
            sprintf(buff, "/tmp/%p.mem", ptr);
            FILE *fp = fopen(buff, "w");
            fprintf(fp, "[+%p]malloc --> addr: %p, size: %lu\n", caller, ptr, size);
            fflush(fp);
        }
        enable_malloc_hook = 1;
        return ptr;
    }
    else
    {
        return malloc_f(size);
    }
}

void free(void *ptr)
{
    if (enable_free_hook)
    {
        enable_free_hook = 0;
        // free被调用时删除文件
        {
            char buff[MEM_CONTENT_LENGTH] = {0};
            sprintf(buff, "/tmp/%p.mem", ptr);
            if (unlink(buff) < 0)
            {
                printf("free: %p\n");
            }
        }
        free_f(ptr);
        enable_free_hook = 1;
    }
    else
    {
        free_f(ptr);
    }
}

static int init_hook()
{
    malloc_f = dlsym(RTLD_NEXT, "malloc");
    free_f = dlsym(RTLD_NEXT, "free");
}

```
2. 编译：gcc memleak_file.c -o memleak_file -ldl -g
3. 执行：./memleak_file
```
[root@xphobia memleak]# ls /tmp/*.mem
/tmp/0x133e010.mem
[root@xphobia memleak]# cat /tmp/0x133e010.mem 
[+0x400947]malloc --> addr: 0x133e010, size: 20
```
4. 确定代码行号
```
[root@xphobia memleak]# addr2line -fe ./memleak_file -a 0x133e010
```

# 宏定义

1. 准备程序源代码：memleak_macro.c
2. 编译、执行memleak_macro
```
[root@xphobia memleak]# gcc -o memleak_macro memleak_macro.c -ldl -g
[root@xphobia memleak]# ./memleak_macro
[root@xphobia memleak]# ls
0x24b8010.mem  memleak_macro  memleak_macro.c
[root@xphobia memleak]# cat 0x24b8010.mem 
[+memleak_macro.c:51]malloc --> addr: 0x24b8010, size: 20
```

