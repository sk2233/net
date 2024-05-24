#ifndef LIST_H
#define LIST_H

#include "type.h"

typedef struct list_node list_node_t;

struct list_node{
    list_node_t *pre;
    list_node_t *next;
    void *data;
};

list_node_t *list_node_alloc(void *data);
void list_node_free(list_node_t *node);

typedef struct list{
    list_node_t *first;
    list_node_t *last;
    uint32_t count;
}list_t;

void list_init(list_t *list);
boot_t list_empty(list_t *list);
void list_add_first(list_t *list, void *data);
void list_add_last(list_t *list,void *data);
list_node_t *list_remove(list_t *list, void *data);

#endif