#include "net.h"
#include "log.h"
#include "loop.h"
#include "package.h"
#include "ether.h"
#include "timer.h"
#include "arp.h"
#include "ipv4.h"
#include "icmp.h"
#include "udp.h"
#include "tcp.h"

void net_loop(){
    log_info("net_loop ...");
    while (1){
        package_t *package= read_package(100);
        if(package==NULL){
            continue;
        }
        err_code code= ether_in(package);
        if(code<0){
            log_warn("ether_in err code = %d", code);
            free_package(package);
        }
    }
}

err_code net_init( ){
    timer_init();
    loop_init();
    ether_init();
    arp_init();
    ipv4_init();
    icmp_init();
    udp_init();
    tcp_init();
    return ERR_CODE_OK;
}
