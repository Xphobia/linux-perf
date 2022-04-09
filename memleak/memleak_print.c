
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