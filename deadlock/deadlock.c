#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t lockA = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockB = PTHREAD_MUTEX_INITIALIZER;

void *threadA(void *arg)
{
    pthread_t tid = pthread_self();

    printf("%lu : Acquire lockA...\n", tid);
    pthread_mutex_lock(&lockA);

    printf("%lu : lockA acquired, Acquire lockB...\n", tid);
    sleep(3);

    pthread_mutex_lock(&lockB);
    printf("%lu : lockB acquired\n", tid);

    while (1) {
        sleep(1);
    };

    pthread_mutex_unlock(&lockB);
    pthread_mutex_unlock(&lockA);

    printf("threadA exit...\n");
    return NULL;
}

void *threadB(void *arg)
{
    pthread_t tid = pthread_self();

    printf("%lu : Acquire lockB...\n", tid);
    pthread_mutex_lock(&lockB);

    printf("%lu : lockB acquired, Acquire lockA...\n", tid);

    pthread_mutex_lock(&lockA);
    printf("%lu : lockA acquired\n", tid);

    while (1) {
        sleep(1);
    };

    pthread_mutex_unlock(&lockA);
    pthread_mutex_unlock(&lockB);

    printf("threadB exit...\n");
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t tidA;
    pthread_t tidB;

    pthread_create(&tidA, 0, threadA, 0);
    pthread_create(&tidB, 0, threadB, 0);

    pthread_join(tidA, 0);
    pthread_join(tidB, 0);

    printf("main thread exit...\n");

    return 0;
}
