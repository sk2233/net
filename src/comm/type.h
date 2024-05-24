#ifndef TYPE_H
#define TYPE_H

#include <stdlib.h>

// 错误都是小于0的
#define ERR_CODE_OK  0
#define ERR_CODE_SYS  (-1)
#define ERR_CODE_TMO  (-2)
#define ERR_CODE_BAD_PORT   (-3)
#define ERR_CODE_TCP_RST   (-3)

#define TRUE 1
#define FALSE 0

#define IPV4_SIZE 4
#define HW_ADDR_SIZE 6

typedef int32_t err_code;
typedef int boot_t;

typedef uint8_t ipv4_t[IPV4_SIZE];
typedef uint8_t hw_addr_t[HW_ADDR_SIZE];

#endif