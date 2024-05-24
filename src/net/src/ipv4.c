#include "ipv4.h"
#include "log.h"
#include "tool.h"
#include "sys_plat.h"
#include "ether.h"
#include "arp.h"
#include "icmp.h"
#include "udp.h"
#include "tcp.h"

static uint16_t ipv4_id;

err_code ipv4_init(){
    route_init();
    return ERR_CODE_OK;
}

err_code ipv4_in(package_t *package){
    if(package->size< sizeof(ipv4_hdr_t)){
        log_warn("ipv4 size err");
        return ERR_CODE_SYS;
    }
    ipv4_hdr_t *hdr= (ipv4_hdr_t *) package->ptr;
    if(hdr->hdr_len*4< sizeof(ipv4_hdr_t)||hdr->version!=IPV4_VERSION|| swap16(hdr->total)< sizeof(ipv4_hdr_t)||
    package->size< swap16(hdr->total)){
        log_warn("ipv4 format err");
        return ERR_CODE_SYS;
    }
    if(hdr->check_sum){
        uint16_t temp=hdr->check_sum;
        hdr->check_sum=0;
        if(check_sum(package->ptr,hdr->hdr_len*4,0,TRUE)!=temp){
            log_warn("ipv4 check_sum err");
            return ERR_CODE_SYS;
        }
    }
    if(!ipv4_eq(hdr->desc,netdev_ip)){
        log_warn("ipv4 desc err");
        return ERR_CODE_SYS;
    }

    reduce_package(package,hdr->total); // 移除0填充
    ipv4_t src;
    ipv4_set(src,hdr->src); // 传递的参数必须拷贝 防止修改
    remove_header(package, hdr->hdr_len*4); // 选项数据也一并移除
    err_code code;
    switch (hdr->protocol) {
        case IPV4_PROTOCOL_ICMP:
            return icmp_in(package,src);
        case IPV4_PROTOCOL_TCP:
            return tcp_in(package,src);
        case IPV4_PROTOCOL_UDP:
            code= udp_in(package,src);
            if(code==ERR_CODE_BAD_PORT){
                return icmp_un_reach(package,src,ICMP_CODE_PORT_UNREACH);
            }
            return code;
        default:
            log_info("ipv4 not support");
            return ERR_CODE_SYS;
    }
}

err_code ipv4_out(package_t *package,uint8_t protocol,ipv4_t desc){
    ipv4_hdr_t *hdr= add_header(package, sizeof(ipv4_hdr_t));
    hdr->hdr_len= sizeof(ipv4_hdr_t)/4;
    hdr->version=IPV4_VERSION;
    hdr->total= swap16(package->size);
    hdr->id=ipv4_id++;
    hdr->ttl=IPV4_TTL;
    hdr->protocol=protocol;
    ipv4_set(hdr->src,netdev_ip);
    ipv4_set(hdr->desc,desc);
    hdr->check_sum= check_sum(package->ptr, sizeof(ipv4_hdr_t),0,TRUE);
    hw_addr_t *addr= get_hw_addr(desc);
    if(!addr){
        log_warn("ipv4 no addr of ip");
        return ERR_CODE_SYS;
    }
    // mac地址只有在同一局域网内才直接填写对方的地址，若需要跨路由器，ip依旧是对方的ip，但是mac地址会是路由器的地址
    // 根据路由表一点点传递过去 路由器也是这样传递数据 根据路由表一点点传递
    return ether_out(package,ETHER_IPV4,*addr);
}

static route_t routes[IP_ROUTE_SIZE];

err_code route_init(){
    return ERR_CODE_OK;
}

void add_route(ipv4_t net,ipv4_t mask,ipv4_t next_hop){
    for (int i = 0; i < IP_ROUTE_SIZE; ++i) {
        route_t *route=routes+i;
        if(!route->used){
            ipv4_set(route->net,net);
            ipv4_set(route->mask,mask);
            ipv4_set(route->next_hop,next_hop);
            route->used=TRUE;
            break;
        }
    }
    log_warn("not empty item to add");
}

void remove_route(ipv4_t net,ipv4_t mask){
    for (int i = 0; i < IP_ROUTE_SIZE; ++i) {
        route_t *route=routes+i;
        if(route->used&& ipv4_eq(route->net,net)&& ipv4_eq(route->mask,mask)){
            route->used=FALSE;
        }
    }
}