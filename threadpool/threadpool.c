#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


#define LL_ADD(item, list)  do {         \
    item->prev = NULL;                   \
    item->next = list;                   \
    if (list != NULL) list->prev = item; \
    list = item;                         \
} while(0)

#define LL_REMOVE(item, list)  do {                         \
    if (item->prev != NULL) item->prev->next = item->next;  \
    if (item->next != NULL) item->next->prev = item->prev;  \
    if (list == item) list = item->next;                    \
    item->prev = item->next = NULL;                         \
} while(0)

// 任务队列
struct NJOB {
    void (*func)(void *arg);
    void *user_data;

    struct NJOB *prev;
    struct NJOB *next;
};

// 执行队列
struct NWORKER {
    pthread_t threadid;
    struct NMANAGER *pool;
    int terminate;

    struct NWORKER *prev;
    struct NWORKER *next;
};

// 管理者
struct NMANAGER {
    struct NJOB *jobs;
    struct NWORKER *workers;

    pthread_cond_t jobs_cond;
    pthread_mutex_t jobs_mutex;
};

typedef struct NMANAGER nThreadPool;

// 线程启动函数
static void *nThreadCallback(void *arg)
{
    struct NWORKER *worker = (struct NWORKER*)arg;
    
    while (1)
    {
        pthread_mutex_lock(&worker->pool->jobs_mutex);
        
        // 如果队列任务为空，则等待
        while (worker->pool->jobs == NULL)
        {
            if (worker->terminate) break;
            pthread_cond_wait(&worker->pool->jobs_cond, &worker->pool->jobs_mutex);
        }
        // 线程终止
        if (worker->terminate)
        {
            pthread_mutex_unlock(&worker->pool->jobs_mutex);
            break;
        }
        // 取出任务
        struct NJOB *job = worker->pool->jobs;
        LL_REMOVE(job, worker->pool->jobs);

        pthread_mutex_unlock(&worker->pool->jobs_mutex);

        // 执行任务
        job->func(job->user_data);
    };
    
    free(worker);
    pthread_exit(NULL);
    // pthread_cancel(worker->threadid);
}

// 线程池初始化
int thread_pool_create(nThreadPool *pool, int nWorkers)
{
    if (pool == NULL) return -1;
    memset(pool, 0, sizeof(nThreadPool));

    if (nWorkers < 1) nWorkers = 1;

    pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;
    memcpy(&pool->jobs_mutex, &blank_mutex, sizeof(pthread_mutex_t));

    pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER;
    memcpy(&pool->jobs_cond, &blank_cond, sizeof(pthread_cond_t));
    
    int i = 0;
    for (i = 0; i < nWorkers; i++)
    {
        struct NWORKER *worker = (struct NWORKER*)malloc(sizeof(struct NWORKER));
        if (worker == NULL)
        {
            perror("malloc");
            return -2;
        }
        memset(worker, 0, sizeof(struct NWORKER));

        int ret = pthread_create(&worker->threadid, NULL, nThreadCallback, worker);
        if (ret)
        {
            perror("pthread_create");
            free(worker);
            return -3;
        }

        LL_ADD(worker, pool->workers);
    }

    return 0;
}
// 线程池销毁
void nThreadDestroy(nThreadPool *pool)
{
    struct NWORKER *worker = NULL;
    for (worker = pool->workers; worker != NULL; worker = worker->next)
    {
        worker->terminate = 1;
    }
    pthread_mutex_lock(&pool->jobs_mutex);
    pthread_cond_broadcast(&pool->jobs_cond);
    pthread_mutex_unlock(&pool->jobs_mutex);
}

// 线程池添加任务
void nThreadPoolPush(nThreadPool *pool, struct NJOB *job)
{
    pthread_mutex_lock(&pool->jobs_mutex);
    LL_ADD(job, pool->jobs);
    pthread_cond_signal(&pool->jobs_cond);
    pthread_mutex_unlock(&pool->jobs_mutex);
}

// 创建1000个线程，打印0-1000
int main(void)
{
    return 0;
}