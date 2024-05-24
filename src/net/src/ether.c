#include "ether.h"
#include "tool.h"
#include "loop.h"
#include "arp.h"
#include "sys_plat.h"
#include "log.h"
#include "ipv4.h"

err_code ether_init(){
    return ERR_CODE_OK;
}

err_code ether_in(package_t *package){
    if(package->size< sizeof(ether_hdr_t)){
        log_warn("ether size err");
        return ERR_CODE_SYS;
    }
    ether_hdr_t *hdr= (ether_hdr_t *) package->ptr;
    switch (swap16(hdr->protocol)) {
        case ETHER_ARP:
            remove_header(package, sizeof(ether_hdr_t));
            return arp_in(package);
        case ETHER_IPV4:
            remove_header(package, sizeof(ether_hdr_t));
            return ipv4_in(package);
        default:
            log_info("ether not support");
            return ERR_CODE_SYS;
    }
}

err_code ether_out(package_t *package,uint16_t protocol,hw_addr_t addr){
    ether_hdr_t *hdr= add_header(package, sizeof(ether_hdr_t));
    hdr->protocol= swap16(protocol);
    hw_addr_set(hdr->desc,addr);
    hw_addr_set(hdr->src, netdev_hwaddr);
    return write_package(package,-1);
}