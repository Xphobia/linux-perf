
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