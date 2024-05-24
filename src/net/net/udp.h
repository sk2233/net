#ifndef NET_UDP_H
#define NET_UDP_H
#include "type.h"
#include "package.h"

#define UDP_HANDLER_SIZE 64

#pragma pack(1)
typedef struct udp_hdr{
    uint16_t src_port;
    uint16_t desc_port;
    uint16_t total;
    uint16_t check_sum;
}udp_hdr_t;
#pragma pack()

typedef err_code (*udp_func)(package_t *package,ipv4_t src_ip,uint16_t src_port);

typedef struct udp_handler{
    uint16_t port;
    udp_func func;
    boot_t used;
}udp_handler_t;

err_code udp_init();
err_code udp_out(package_t *package,ipv4_t desc,uint16_t src_port,uint16_t desc_port);
err_code udp_in(package_t *package,ipv4_t src);

#endif
