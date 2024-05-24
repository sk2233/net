#include "timer.h"
#include "tool.h"
#include "sys_plat.h"
#include "log.h"

static timer_t *header;

timer_t *insert_timer(timer_t *curr,timer_t *timer) {
    if(curr==NULL){
        return timer;
    } else if(timer->time>curr->time){
        curr->next= insert_timer(curr->next,timer);
        return curr;
    } else{
        timer->next=curr;
        return timer;
    }
}

void add_event(void (*func)(void *data), void *data, uint32_t ms){
    timer_t *timer= malloc(sizeof(timer_t));
    timer->time=get_time()+ms;
    timer->func=func;
    timer->data=data;
    header=insert_timer(header,timer);
}

timer_t *remove_timer(timer_t *curr, void (*func)(void *)) {
    if(curr==NULL){
        return NULL;
    } else if(curr->func==func){
        timer_t *temp = curr->next;
        free(curr);
        return temp;
    } else{
        curr->next= remove_timer(curr->next,func);
        return curr;
    }
}

void remove_event(void (*func)(void *data)){
    header=remove_timer(header,func);
}

void timer_task(void *data){
    log_info("timer_task ...");
    while (1){
        if(header==NULL){
            sys_sleep(100);
        } else{
            uint32_t curr = get_time();
            if(header->time>curr){
                sys_sleep(header->time-curr);
            } else{
                header->func(header->data);
                timer_t *old=header;
                header=header->next;
                free(old);
            }
        }
    }
}

err_code timer_init(){
    sys_thread_create(timer_task,0);
    return ERR_CODE_OK;
}