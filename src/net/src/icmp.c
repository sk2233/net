#include "icmp.h"
#include "log.h"
#include "tool.h"
#include "ipv4.h"

void icmp_init(){

}

err_code icmp_in(package_t *package, ipv4_t src){
    if(package->size< sizeof(icmp_hdr_t)){
        log_warn("icmp err size");
        return ERR_CODE_SYS;
    }
    icmp_hdr_t *hdr= (icmp_hdr_t *) package->ptr;
    if(hdr->check_sum){
        uint16_t temp = hdr->check_sum;
        hdr->check_sum=0;// 这个校验和是校验全部数据包的
        if(check_sum(package->ptr, package->size,0,TRUE)!=temp){
            log_warn("icmp check_sum err");
            return ERR_CODE_SYS;
        }
    }

    switch (hdr->type) {
        case ICMP_ECHO_REQ:
            return icmp_resp(package,src);
        default:
            log_info("icmp not support");
            return ERR_CODE_SYS;
    }
}

err_code icmp_resp(package_t *package, ipv4_t desc) {
    icmp_hdr_t *hdr= (icmp_hdr_t *) package->ptr;
    hdr->type=ICMP_ECHO_RESP;
    hdr->check_sum=0;
    hdr->check_sum= check_sum(package->ptr,package->size,0,TRUE);
    return ipv4_out(package,IPV4_PROTOCOL_ICMP,desc);
}

// 从ip头开始拷贝，外面不要移除ip头部
err_code icmp_un_reach(package_t *package,ipv4_t desc,uint8_t code){
    reduce_package(package,ICMP_MAX_SIZE);// icmp body大小有限最多28
    ipv4_hdr_t *temp= (ipv4_hdr_t *) package->ptr;
    if(!temp->check_sum){// ip的check_sum若是缺失补充一下
        temp->check_sum= check_sum(package->ptr,temp->hdr_len*4,0,TRUE);
    }
    icmp_hdr_t *hdr= add_header(package, sizeof(icmp_hdr_t));
    hdr->type=ICMP_UN_REACH;
    hdr->code=code;
    hdr->check_sum= check_sum(package->ptr,package->size,0,TRUE);
    return ipv4_out(package,IPV4_PROTOCOL_ICMP,desc);
}