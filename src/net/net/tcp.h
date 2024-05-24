#ifndef NET_TCP_H
#define NET_TCP_H
#include "type.h"
#include "package.h"

#pragma pack(1)
typedef struct tcp_hdr{
    uint16_t src_port;
    uint16_t desc_port;
    // 双工的协议，可以同时声明自己的数据发到哪里了，对方的数据接收到哪里了
    uint32_t seq; // 本次发送的数据是从哪里开始的
    uint32_t ack; // 通知对方希望下次从哪里开始发
    union {
        struct {
            uint16_t reserved:4;
            uint16_t hdr_size:4;
            uint16_t flag_fin:1;
            uint16_t flag_syn:1;
            uint16_t flag_rst:1;
            uint16_t flag_psh:1;
            uint16_t flag_ack:1;
            uint16_t flag_urg:1;
            uint16_t unused:2;
        };
        uint16_t all;
    };
    uint16_t window;
    uint16_t check_sum;
    uint16_t urg_ptr;
}tcp_hdr_t;
#pragma pack()

typedef enum {
    TCP_CLOSE =1, // CLOSE
    TCP_LISTEN, // LISTEN
    TCP_SYN_SEND, // SYN_SEND
    TCP_SYN_RECEIVE, // SYN_RECEIVE
    TCP_ESTABLISHED, // ESTABLISHED
    TCP_FIN_WAIT1, // FIN_WAIT1
    TCP_FIN_WAIT2, // FIN_WAIT2
    TCP_CLOSING, // CLOSING
    TCP_TIME_WAIT, // TIME_WAIT
    TCP_CLOSE_WAIT, // CLOSE_WAIT
    TCP_LAST_ACK // LAST_ACK
}tcp_state;

err_code tcp_init();

err_code tcp_in(package_t *package,ipv4_t src);

err_code tcp_send_syn(ipv4_t ip, uint16_t port);
err_code tcp_send_fin(ipv4_t ip);

#endif
