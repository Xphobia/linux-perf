
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/socket.h>
#include "mytimer.h"

typedef struct node {
    int tfd;
    union {
        int sp[2];
        struct {
            int r;
            int w;
        } rw;
    };
    int count;
    int timeout;
    struct node *prev, *next;
} mytimer_t;

static mytimer_t head;
static void sighandler(int signum);
void mytimer_init(void) __attribute__((constructor));
void mytimer_destroy(void) __attribute__((destructor));

/**
 * SIGALARM信号处理函数，检测闹钟是否有超时
 */
static void sighandler(int signum)
{
    mytimer_t *i, *n;

    if (signum != SIGALRM) {
        return;
    }

    for (i = head.next; i != &head; i = n)
    {
        // timer还未设置timeout时，跳过不检查
        if (i->timeout == 0)
        {
            continue;
        }
        n = i->next;
        i->count++;
        if (i->count >= i->timeout)
        {
            // printf("timeout writefd = %d\n", i->rw.w);
            write(i->rw.w, "\n", 1);
            i->count = 0;
        }
    }

    alarm(1);
}

void mytimer_init(void)
{
    printf("mytimer_init\n");
    // 初始化timer链表
    head.prev = &head;
    head.next = &head;

    // 注册SIGALARM信号
    signal(SIGALRM, sighandler);

    // 启动闹钟
    alarm(1);
}

int mytimer_create(void)
{
    int ret = -1;

    mytimer_t *timer = (mytimer_t*)malloc(sizeof(mytimer_t));
    if (timer == NULL)
    {
        return -1;
    }

    // 创建socketpair
    ret = socketpair(AF_UNIX, SOCK_STREAM, 0, timer->sp);
    if (ret == -1)
    {
        free(timer);
        return -1;
    }
    // printf("mytimer_create: readfd = %d, wirtefd = %d\n", timer->rw.r, timer->rw.w);
    timer->tfd = timer->rw.r;
    timer->timeout = 0;

    // 把timer添加到链表
    timer->next = &head;
    timer->prev = head.prev;
    timer->next->prev = timer;
    timer->prev->next = timer;

    return timer->tfd;
}

/**
 * 设置超时时间
 */
int mytimer_settime(int tfd, int interval)
{
    mytimer_t *t = head.next;

    while (t != &head)
    {
        if (t->tfd == tfd)
        {
            // printf("mytimer_settime: tfd = %d, timeout = %d\n", t->tfd, t->timeout);
            t->timeout = interval;
            break;
        }
    }
    return 0;
}

/*
 **/
void mytimer_delete(int tfd)
{
    mytimer_t  *i = head.next;

    while (i != &head)
    {
        if (i->tfd == tfd)
        {
            i->prev->next = i->next;
            i->next->prev = i->prev;
            free(i);
            break;
        }
        i = i->next;
    }
}

void mytimer_destroy(void)
{
    printf("mytimer_destroy\n");
    mytimer_t *i, *n;

    alarm(0);
    signal(SIGALRM, SIG_DFL);

    for (i = head.next; i != &head; i = n)
    {
        n = i->next;
        free(i);
    }
}