#include "queue.h"

void queue_init(queue_t *queue){
    queue->read=queue->write=queue->count=0;
    queue->read_sem= sys_sem_create(0);
    queue->write_sem= sys_sem_create(QUEUE_SIZE);
}

// 只有单线程读写 还好
err_code queue_write(queue_t *queue,void *data,int32_t ms){
    if(sys_sem_wait(queue->write_sem,ms)<0){
        return ERR_CODE_TMO;
    }
    queue->buff[queue->write%QUEUE_SIZE]=data;
    queue->write++;
    queue->count++;
    sys_sem_notify(queue->read_sem);
    return ERR_CODE_OK;
}

void *queue_read(queue_t *queue,int32_t ms){
    if(sys_sem_wait(queue->read_sem,ms)<0){
        return 0;
    }
    void *data=queue->buff[queue->read%QUEUE_SIZE];
    queue->read++;
    queue->count--;
    sys_sem_notify(queue->write_sem);
    return data;
}
