#include <time.h>
#include <sys/time.h>
#include <string.h>
#include "tool.h"

void swap(void *ptr,uint32_t len){
    uint8_t *l=ptr;
    uint8_t *r=l+len-1;
    while (l<r){
        uint8_t temp=*l;
        *l=*r;
        *r=temp;
        l++;
        r--;
    }
}

uint16_t swap16(uint16_t data){
    swap(&data,2);
    return data;
}

uint32_t swap32(uint32_t data){
    swap(&data,4);
    return data;
}

uint32_t get_time(){ // 获取当前毫秒
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    return current_time.tv_sec * 1000 + current_time.tv_usec / 1000;
}

void ipv4_set(ipv4_t desc,ipv4_t val){
    memcpy(desc,val,IPV4_SIZE);
}

boot_t ipv4_eq(ipv4_t ip1,ipv4_t ip2){
    return memcmp(ip1,ip2,IPV4_SIZE)==0;
}

void hw_addr_set(hw_addr_t desc,hw_addr_t val){
    memcpy(desc,val,HW_ADDR_SIZE);
}

boot_t hw_addr_eq(hw_addr_t hw1,uint8_t *hw2){
    return memcmp(hw1,hw2,HW_ADDR_SIZE)==0;
}

uint16_t check_sum(void *buff,uint16_t len,uint16_t base,boot_t ok){
    uint16_t *curr=buff;
    uint32_t sum=base;
    while (len>1){
        sum+=*curr++;
        len-=2;
    }
    if(len>0){
        sum+=*(uint8_t *)curr;
    }
    uint16_t high;
    while ((high=sum>>16)!=0){
        sum=high+(sum&0xFFFF);
    }
    if(!ok){
        return sum;
    }
    return ~sum;
}

uint16_t check_sum_pre(ipv4_t src,ipv4_t desc,uint8_t protocol,uint16_t total){
    fake_hdr_t *hdr= malloc(sizeof(fake_hdr_t));
    ipv4_set(hdr->src,src);
    ipv4_set(hdr->desc,desc);
    hdr->protocol=protocol;
    hdr->total= swap16(total);
    uint16_t res= check_sum(hdr, sizeof(fake_hdr_t),0,FALSE);
    free(hdr);
    return res;
}

static uint16_t curr_port=1024;

uint16_t alloc_port(){ // 分配端口 一般发送方端口是随意的
    curr_port++;
    return curr_port;
}