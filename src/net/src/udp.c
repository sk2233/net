#include "udp.h"
#include "tool.h"
#include "ipv4.h"
#include "sys_plat.h"
#include "log.h"

static udp_handler_t handlers[UDP_HANDLER_SIZE];

void register_udp_handler(uint16_t port,udp_func func){
    for (int i = 0; i < UDP_HANDLER_SIZE; ++i) {
        udp_handler_t *handler=handlers+i;
        if(!handler->used){
            handler->used=TRUE;
            handler->port=port;
            handler->func=func;
            return;
        }
    }
    log_warn("no empty handlers");
}

// 暂时直接发回去
err_code default_udp_handler(package_t *package,ipv4_t src_ip,uint16_t src_port){
    return udp_out(package,src_ip,2233,src_port);
}

err_code udp_init(){
    register_udp_handler(2233,default_udp_handler);
    return ERR_CODE_OK;
}

udp_func get_udp_handler(uint16_t port){
    for (int i = 0; i < UDP_HANDLER_SIZE; ++i) {
        udp_handler_t *handler=handlers+i;
        if(handler->used&&handler->port==port){
            return handler->func;
        }
    }
    return NULL;
}

err_code udp_out(package_t *package,ipv4_t desc,uint16_t src_port,uint16_t desc_port){
    udp_hdr_t *hdr= add_header(package, sizeof(udp_hdr_t));
    hdr->src_port= swap16(src_port);
    hdr->desc_port= swap16(desc_port);
    hdr->total= swap16(package->size);
    hdr->check_sum=0;
    uint16_t pre= check_sum_pre(netdev_ip,desc,IPV4_PROTOCOL_UDP,package->size);
    hdr->check_sum= check_sum(package->ptr,package->size,pre,TRUE);
    return ipv4_out(package,IPV4_PROTOCOL_UDP,desc);
}

err_code udp_in(package_t *package,ipv4_t src){
    if(package->size< sizeof(udp_hdr_t)){
        return ERR_CODE_SYS;
    }
    udp_hdr_t *hdr= (udp_hdr_t *)package->ptr;
    if(hdr->check_sum){
        uint16_t pre= check_sum_pre(src,netdev_ip,IPV4_PROTOCOL_UDP,package->size);
        uint16_t temp=hdr->check_sum; // 计算校验和必须原来 check_sum = 0
        hdr->check_sum=0;
        if(check_sum(package->ptr, package->size,pre,TRUE)!=temp){
            return ERR_CODE_SYS;
        }
    }
    udp_func func= get_udp_handler(swap16(hdr->desc_port));
    if(func==NULL){
        return ERR_CODE_BAD_PORT;
    }
    remove_header(package, sizeof(udp_hdr_t));
    return func(package,src, swap16(hdr->src_port));
}