#include "list.h"

list_node_t *list_node_alloc(void *data){
    list_node_t *note= malloc(sizeof(list_node_t));
    note->pre= note->next=NULL;
    note->data=data;
    return note;
}

void list_node_free(list_node_t *node){
    free(node);
}

void list_init(list_t *list){
    list->first= list->last=NULL;
    list->count=0;
}

boot_t list_empty(list_t *list){
    return list->count==0;
}

void list_add_first(list_t *list, void *data){
    list_node_t *node= list_node_alloc(data);
    if(list_empty(list)){
        list->first=list->last=node;
        node->pre= node->next=NULL;
    } else{
        node->next=list->first;
        list->first->pre=node;
        list->first=node;
        node->pre=NULL;
    }
    list->count++;
}

void list_add_last(list_t *list,void *data){
    list_node_t *node= list_node_alloc(data);
    if(list_empty(list)){
        list->first=list->last=node;
        node->pre= node->next=NULL;
    } else{
        list->last->next=node;
        node->pre=list->last;
        list->last=node;
        node->next=NULL;
    }
    list->count++;
}

list_node_t *find_list_node(list_t *list,void *data) {
    list_node_t *node=list->first;
    while (node!=list->last){
        if(node->data==data){
            return node;
        }
        node=node->next;
    }
    if(node->data==data){
        return node;
    }
    return NULL;
}

list_node_t *list_remove(list_t *list, void *data){
    list_node_t *node= find_list_node(list,data);
    if(node == list->first){
        list->first=node->next;
    }
    if(node == list->last){
        list->last=node->pre;
    }
    if(node->pre){
        node->pre->next=node->next;
    }
    if(node->next){
        node->next->pre=node->pre;
    }
    node->next= node->pre=NULL;
    list->count--;
    return node;
}
