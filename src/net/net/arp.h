#ifndef ARP_H
#define ARP_H
#include "net.h"

#define ARP_TYPE_FREE 0
#define ARP_TYPE_WAIT 1
#define ARP_TYPE_VALID 2

#define ARP_ITEM_SIZE 32

#define ARP_HW_ETHER 1
#define ARP_PROTOCOL_IPV4 0x0800
#define ARP_OPCODE_REQ 1
#define ARP_OPCODE_RESP 2

#define ARP_VALID_TMO 30
#define ARP_WAIT_TMO 3
#define ARP_RETRY 5

typedef struct arp_item{
    ipv4_t ipv4;
    hw_addr_t hw_addr;
    uint8_t type;
    int8_t tmo;
    int8_t retry;
}arp_item_t;

#pragma pack(1)
typedef struct arp_hdr{
    uint16_t hw_type;
    uint16_t protocol_type;
    uint8_t hw_len;
    uint8_t protocol_len;
    uint16_t opcode;
    hw_addr_t src_hw;
    ipv4_t src_ip;
    hw_addr_t tar_hw;
    ipv4_t tar_ip;
}arp_hdr_t;
#pragma pack()

err_code arp_init();
err_code arp_in(package_t *package);
err_code arp_req(ipv4_t ip);
hw_addr_t *get_hw_addr(ipv4_t ip);
void set_hw_addr(ipv4_t ip,hw_addr_t hw_addr);

#endif