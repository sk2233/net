#ifndef ETHER_H
#define ETHER_H
#include "type.h"
#include "package.h"

#define ETHER_ARP 0x0806
#define ETHER_IPV4 0x0800

#pragma pack(1)
typedef struct ether_hdr{
    hw_addr_t desc;
    hw_addr_t src;
    uint16_t protocol;
}ether_hdr_t;
#pragma pack()

err_code ether_init();
err_code ether_in(package_t *package);
err_code ether_out(package_t *package,uint16_t protocol,hw_addr_t addr);

#endif