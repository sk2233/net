#ifndef PING_H
#define PING_H
#include "type.h"

typedef struct ping{

}ping_t;

void do_ping(ping_t *ping,ipv4_t ip,uint32_t count,uint32_t size,uint32_t interval);

#endif