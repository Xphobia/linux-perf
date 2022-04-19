
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <sys/epoll.h>
#include <sys/time.h>

#include "mytimer.h"

typedef void (*task_t)(void *args);

struct event_data {
    int timeout;
    int number;
};

struct timer_event {
    int tfd;
    task_t ev_task;
    struct event_data ev_data;
};

void ev_task(void *args)
{
    struct timer_event *evt = (struct timer_event*)args;
    printf("do ev_task: number = %d\n", evt->ev_data.number);
}

int timerfd_get(struct event_data *ev_data) {
    assert(ev_data);
    int rc = 0;

    // 创建定时器fd
    int tfd = mytimer_create();
    if (tfd < 0) {
        perror("mytimer_create");
        return tfd;
    }

    // 设置定时器
    rc = mytimer_settime(tfd, ev_data->timeout);
    if (rc < 0) {
        perror("mytimer_settime");
        return rc;
    }

    return tfd;
}

void epoll_loop(struct timer_event *evt)
{
    // 创建epoll fd，同时监听timer fd
    int efd = epoll_create(1);
    struct epoll_event events[5];
    struct epoll_event ev;
    ev.data.ptr = (void*)evt;
    ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(efd, EPOLL_CTL_ADD, evt->tfd, &ev);

    while (1) {
        int i = 0;
        int n = epoll_wait(efd, events, 5, 0);
        for (i = 0; i < n; i++) {
            // 收到超时信号，读取
            uint64_t buf;
            struct timer_event *data = (struct timer_event*)events[i].data.ptr;
            int r = read(data->tfd, &buf, sizeof(buf));
            if (r > 0) {
                time_t t;
                char buffer[32];
                time(&t);
                ctime_r(&t, buffer);
                printf("\n timeout: %s", buffer);
                data->ev_task(&data->ev_data);
            }
        }
    };
}

int main(void)
{
    struct event_data ev_data = {
            .timeout = 3,
            .number = 8
    };

    int tfd = timerfd_get(&ev_data);
    if (tfd < 0) {
        printf("timerfd_get failed");
        return tfd;
    }
    printf("timerfd read = %d\n", tfd);

    struct timer_event evt = {
            .tfd = tfd,
            .ev_data = ev_data,
            .ev_task = ev_task
    };

    epoll_loop(&evt);

    return 0;
}
