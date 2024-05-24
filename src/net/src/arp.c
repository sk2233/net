#include "arp.h"
#include "package.h"
#include "tool.h"
#include "ether.h"
#include "log.h"
#include "sys_plat.h"
#include "timer.h"
#include "tcp.h"

static arp_item_t arp_items[ARP_ITEM_SIZE];

arp_item_t *alloc_arp_item(){
    for (int i = 0; i < ARP_ITEM_SIZE; ++i) {
        if(arp_items[i].type==ARP_TYPE_FREE){
            return &arp_items[i];
        }
    }
    return 0;
}

arp_item_t *find_arp_item(ipv4_t ipv4){
    for (int i = 0; i < ARP_ITEM_SIZE; ++i) {
        if(ipv4_eq(arp_items[i].ipv4,ipv4)){
            return &arp_items[i];// 找到了
        }
    }
    return 0;
}

hw_addr_t *get_hw_addr(ipv4_t ip){
    arp_item_t *item= find_arp_item(ip);
    if(!item){// 没有发请求创建
        item=alloc_arp_item();
        ipv4_set(item->ipv4,ip);
        item->type=ARP_TYPE_WAIT;
        item->tmo=ARP_WAIT_TMO;
        item->retry=ARP_RETRY;
        return 0;
    }
    if(item->type==ARP_TYPE_WAIT){
        return 0;
    }
    return &item->hw_addr;
}

void set_hw_addr(ipv4_t ip,hw_addr_t hw_addr){
    arp_item_t *item= find_arp_item(ip);
    if(!item){
        item=alloc_arp_item();
        ipv4_set(item->ipv4,ip);
    }
    hw_addr_set(item->hw_addr,hw_addr);
    item->type=ARP_TYPE_VALID;
    item->tmo=ARP_VALID_TMO;
}

void arp_refresh(void *data){
    add_event(arp_refresh,0,1000);
    for (int i = 0; i < ARP_ITEM_SIZE; ++i) {
        arp_item_t *item=arp_items+i;
        if(item->type==ARP_TYPE_FREE){
            continue;
        }
        if(item->tmo>0){
            item->tmo--;
            continue;
        }
        if(item->type==ARP_TYPE_VALID){// 有效期到了
            err_code code= arp_req(item->ipv4);
            if(code<0){
                log_err("arp_req err = %d",code);
                continue;
            }
            item->type=ARP_TYPE_WAIT;
            item->tmo=ARP_WAIT_TMO;
            item->retry=ARP_RETRY;
        } else if(item->type==ARP_TYPE_WAIT){
            if(item->retry>0){
                item->retry--;
                err_code code= arp_req(item->ipv4);
                if(code<0){
                    log_err("arp_req err = %d",code);
                    continue;
                }
                item->tmo=ARP_WAIT_TMO;
            } else{// 没有重试机会了 放弃
                memset(item,0, sizeof(arp_item_t));
            }
        }
    }
}

err_code arp_init(){
    arp_req(netdev_ip); // 查询自己的mac地址将没有人回复，收到的人都更新一下 ip->mac的映射
    arp_req(friend_ip);
    add_event(arp_refresh,0,1000);
    return ERR_CODE_OK;
}

err_code arp_req(ipv4_t ip){
    package_t *package= alloc_package(sizeof(arp_hdr_t));
    arp_hdr_t *hdr= (arp_hdr_t *) package->ptr;
    hdr->hw_type= swap16(ARP_HW_ETHER);
    hdr->protocol_type= swap16(ARP_PROTOCOL_IPV4);
    hdr->hw_len=HW_ADDR_SIZE;
    hdr->protocol_len=IPV4_SIZE;
    hdr->opcode= swap16(ARP_OPCODE_REQ);
    hw_addr_set(hdr->src_hw, netdev_hwaddr);
    ipv4_set(hdr->src_ip, netdev_ip);
    ipv4_set(hdr->tar_ip,ip);
    err_code code=   ether_out(package,ETHER_ARP,broadcast_hwaddr);
    if(code<0){
        free_package(package);
        return code;
    }
    return ERR_CODE_OK;
}

err_code arp_resp(package_t *package){
    arp_hdr_t *hdr= (arp_hdr_t *) package->ptr;
    hdr->opcode= swap16(ARP_OPCODE_RESP);
    hw_addr_set(hdr->tar_hw,hdr->src_hw);
    ipv4_set(hdr->tar_ip,hdr->src_ip);
    hw_addr_set(hdr->src_hw, netdev_hwaddr);
    ipv4_set(hdr->src_ip, netdev_ip);
    return ether_out(package,ETHER_ARP,hdr->tar_hw);
}

err_code arp_in(package_t *package){
    if(package->size< sizeof(arp_hdr_t)){
        log_warn("arp size err");
        return ERR_CODE_SYS;
    }
    arp_hdr_t *hdr= (arp_hdr_t *) package->ptr;
    if(hdr->hw_type!= swap16(ARP_HW_ETHER)||hdr->protocol_type!= swap16(ARP_PROTOCOL_IPV4)||
    hdr->hw_len!=HW_ADDR_SIZE||hdr->protocol_len!=IPV4_SIZE){
        log_warn("arp format err");
        return ERR_CODE_SYS;
    }
    switch (swap16(hdr->opcode)) {
        case ARP_OPCODE_REQ:
            set_hw_addr(hdr->src_ip,hdr->src_hw);
            return arp_resp(package);
        case ARP_OPCODE_RESP:
            set_hw_addr(hdr->src_ip,hdr->src_hw);
            free_package(package);
            // TODO TEST
            tcp_send_syn(friend_ip, 2233);
            return ERR_CODE_OK;
        default:
            log_info("arp not support");
            return ERR_CODE_SYS;
    }
}