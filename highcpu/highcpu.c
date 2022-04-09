
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

int a = 0;
int b = 0;

void *threadA(void *arg)
{
    pthread_t tid = pthread_self();

    printf("%lu : threadA starts working...\n", tid);

    while (1) {
        a++;
    };

    printf("threadA exit...\n");
    return NULL;
}

void *threadB(void *arg)
{
    pthread_t tid = pthread_self();

    printf("%lu : threadB starts working...\n", tid);
    while (1) {
        b++;
        sleep(1);
    };

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
