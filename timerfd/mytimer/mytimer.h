
#ifndef _MYTIMER_H
#define _MYTIMER_H

// int mytimer_init(void);

int mytimer_create(void);

int mytimer_settime(int tfd, int interval);

void mytimer_delete(int tfd);

// void mytimer_destroy(void);

#endif /* _MYTIMER_H */
