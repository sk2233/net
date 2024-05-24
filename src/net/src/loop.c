#include "loop.h"
#include "log.h"
#include "sys_plat.h"
#include "queue.h"

static queue_t read_queue;
static queue_t write_queue;
static pcap_t *pcap;

err_code write_package(package_t *package,int32_t ms){
    return queue_write(&read_queue,package,ms);
}

package_t *read_package(int32_t ms){
    return queue_read(&write_queue,ms);
}

void read_task(void *args){
    // 不停的读数据并写入read_queue
    log_info("read_task ...");
    pcap_header_t *hdr;
    const uint8_t *data;
    while (1){
        if(pcap_next_ex(pcap,&hdr,&data)!=1){
            continue;
        }
        package_t *package= alloc_package(hdr->len);
        memcpy(package->ptr,data,hdr->len);
        err_code code = queue_write(&write_queue,package,-1);
        if(code<0){
            log_warn("write_package err code = %d", code);
            free_package(package);
        }
    }
}

void send_task(void *args){
    log_info("send_task ...");
    while (1){
        package_t *package= queue_read(&read_queue,100);
        if(package==NULL){
            continue;
        }
        extend_package(package,MIN_PACKAGE_SIZE);
        if(pcap_inject(pcap,package->ptr,package->size)==-1){
            log_err("pcap_inject err %s", pcap_geterr(pcap));
        }
        free_package(package);
    }
}

err_code loop_init(){
    pcap= pcap_device_open("192.168.74.1", netdev_hwaddr);
    queue_init(&read_queue);
    queue_init(&write_queue);
    sys_thread_create(read_task,0);
    sys_thread_create(send_task,0);
    return ERR_CODE_OK;
}

