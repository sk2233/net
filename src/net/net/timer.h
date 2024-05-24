#ifndef TIMER_H
#define TIMER_H
#include "type.h"

typedef struct timer timer_t;

struct timer{
    uint32_t time;
    void (*func)(void *data);
    void *data;
    timer_t *next;
};

void add_event(void (*func)(void *data), void *data, uint32_t ms);
void remove_event(void (*func)(void *data));
err_code timer_init();

#endif