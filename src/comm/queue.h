#ifndef QUEUE_H
#define QUEUE_H

#include "type.h"
#include "sys_plat.h"

#define QUEUE_SIZE 128

typedef struct queue{
    void *buff[QUEUE_SIZE];
    uint32_t read,write,count;
    sys_sem_t *read_sem, *write_sem;
}queue_t;

void queue_init(queue_t *queue);
err_code queue_write(queue_t *queue,void *data,int32_t ms);
void *queue_read(queue_t *queue,int32_t ms);

#endif