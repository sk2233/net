#ifndef TOOL_H
#define TOOL_H
#include "type.h"

uint16_t swap16(uint16_t data);
uint32_t swap32(uint32_t data);
uint32_t get_time();

void ipv4_set(ipv4_t desc,ipv4_t val);
boot_t ipv4_eq(ipv4_t ip1,ipv4_t ip2);

void hw_addr_set(hw_addr_t hw_addr,hw_addr_t hw);
boot_t hw_addr_eq(hw_addr_t hw1,uint8_t *hw2);

uint16_t check_sum(void *buff,uint16_t len,uint16_t base,boot_t ok);

typedef struct fake_hdr{
    ipv4_t src;
    ipv4_t desc;
    uint8_t unused;
    uint8_t  protocol;
    uint16_t total;
}fake_hdr_t;

uint16_t check_sum_pre(ipv4_t src,ipv4_t desc,uint8_t protocol,uint16_t total);

uint16_t alloc_port();

#endif