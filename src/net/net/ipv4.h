#ifndef IPV4_H
#define IPV4_H
#include "type.h"
#include "package.h"

#define IPV4_VERSION 4

#define IPV4_PROTOCOL_ICMP 1
#define IPV4_PROTOCOL_TCP 6
#define IPV4_PROTOCOL_UDP 17

#define IPV4_TTL 64

#define IP_ROUTE_SIZE 64

#pragma pack(1)
typedef struct ipv4_hdr{
    uint8_t hdr_len:4; // *4  实际与下面是相反的，但是因为大小段转换这样就行了
    uint8_t version:4;
    uint8_t unused1;
    uint16_t total;
    uint16_t id;
    // 分片 大小端转换
    uint16_t offset:13;
    uint16_t more:1;
    uint16_t disable:1;
    uint16_t unused2:1;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t check_sum;
    ipv4_t src;
    ipv4_t desc;
}ipv4_hdr_t;
#pragma pack()

err_code ipv4_init();
err_code ipv4_in(package_t *package);
err_code ipv4_out(package_t *package,uint8_t protocol,ipv4_t desc);

typedef struct route{
    ipv4_t net;
    ipv4_t mask;
    ipv4_t next_hop;
    boot_t used;
}route_t;

err_code route_init();
void add_route(ipv4_t net,ipv4_t mask,ipv4_t next_hop);
void remove_route(ipv4_t net,ipv4_t mask);

#endif