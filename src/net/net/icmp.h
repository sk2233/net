#ifndef ICMP_H
#define ICMP_H
#include "type.h"
#include "package.h"

#define ICMP_ECHO_REQ 8
#define ICMP_ECHO_RESP 0
#define ICMP_UN_REACH 3

#define ICMP_MAX_SIZE 28

#define ICMP_CODE_PORT_UNREACH 3

typedef struct icmp_hdr{
    uint8_t type;
    uint8_t code;
    uint16_t check_sum;
    uint32_t unused;
}icmp_hdr_t;

void icmp_init();
err_code icmp_resp(package_t *package, ipv4_t ip);
err_code icmp_in(package_t *package,ipv4_t ip);
err_code icmp_un_reach(package_t *package,ipv4_t ip,uint8_t code);

#endif