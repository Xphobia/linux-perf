
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MEM_CONTENT_LENGTH  128

void *malloc_hook(size_t size, const char *file, int line)
{
    void *ptr = malloc(size);
    // malloc被调用时保存信息到文件
    {
        // 0 == f() --> malloc()
        // 1 == a() --> f() --> malloc()
        // 2 == a() --> b() --> f() --> malloc()
        void *caller = __builtin_return_address(0);
        char buff[MEM_CONTENT_LENGTH] = {0};
        sprintf(buff, "./%p.mem", ptr);
        FILE *fp = fopen(buff, "w");
        fprintf(fp, "[+%s:%d]malloc --> addr: %p, size: %lu\n", file, line, ptr, size);
        fflush(fp);
    }
    return ptr;
}

void free_hook(void *ptr, const char *file, int line)
{
    // free被调用时删除文件
    {
        char buff[MEM_CONTENT_LENGTH] = {0};
        sprintf(buff, "./%p.mem", ptr);
        if (unlink(buff) < 0)
        {
            printf("free: %p\n", ptr);
        }
    }
    free(ptr);
}

#define malloc(size)    malloc_hook(size, __FILE__, __LINE__)
#define free(ptr)       free_hook(ptr, __FILE__, __LINE__)


int main(void)
{
    void *p1 = malloc(10);
    free(p1);

    void *p2 = malloc(20);
    
    void *p3 = malloc(30);
    free(p3);

    return 0;
}