﻿#ifndef SYS_PLAT_H
#define SYS_PLAT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <pcap.h>
#include "type.h"

// 系统硬件配置
// 不同网卡配置，共2块网卡
#if 1
static ipv4_t netdev_ip = {192, 168, 74, 2}; // 本地网络
static ipv4_t netdev_gw = {192, 168, 74, 1}; // 网卡地址，本地网络，虚拟网络链接的桥梁
static ipv4_t friend_ip = {192, 168, 74, 3}; // 虚拟机网络
//static const ipv4_t netdev0_phy_ip[] = "192.168.74.1";    // 用于收发包的真实网卡ip地址，在qemu上不需要使用
static ipv4_t netdev_mask = {255,255,255,0};
static hw_addr_t netdev_hwaddr = {0x00, 0x50, 0x56, 0xc0, 0x00, 0x11 };
static hw_addr_t broadcast_hwaddr={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
#else
static const char netdev_ip[] = "192.168.74.2";
static const char netdev_gw[] = "192.168.74.3";
static const char friend_ip[] = "192.168.74.3";
static const char netdev0_phy_ip[] = "192.168.74.1";    // 用于收发包的真实网卡ip地址
static const char netdev_mask[] = "255.255.255.0";
#endif

static const char netdev1_ip[] = "10.0.2.200";
static const char netdev1_gw[] = "10.0.2.2";
static const char netdev1_phy_ip[] = "192.168.254.1";
static const char friend1_ip[] = "10.0.2.2";
static const char netdev1_mask[] = "255.255.255.0";
static const uint8_t netdev1_hwaddr[] = { 0x00, 0x50, 0x56, 0xc0, 0x00, 0x22 };

typedef struct timeval net_time_t;      // 时间类型
typedef struct pcap_pkthdr pcap_header_t;

#define SYS_THREAD_INVALID          (sys_thread_t*)0
#define SYS_SEM_INVALID             (sys_sem_t*)0
#define SYS_MUTEx_INVALID           (sys_mutex_t*)0

#define plat_strlen         strlen
#define plat_strcpy         strcpy
#define plat_strncpy        strncpy
#define plat_strcmp         strcmp
#define plat_stricmp        strcasecmp
#define plat_memset         memset
#define plat_memcpy         memcpy
#define plat_memcmp         memcmp
#define plat_sprintf        sprintf
#define plat_vsprintf       vsprintf
#define plat_printf         printf

typedef struct _xsys_sem_t {
    int count;                          // 信号量计数
    pthread_cond_t cond;                // 条件变量
    pthread_mutex_t locker;             // 访问C的互斥锁
}sys_sem_t;

typedef struct _opaque_pthread_t sys_thread_t;           // 线程重定义
typedef pthread_mutex_t sys_mutex_t;      // 互斥信号量

// PCAP网卡驱动相关函数
int pcap_find_device(const char* ip, char* name_buf);
int pcap_show_list(void);
pcap_t * pcap_device_open(const char* ip, const uint8_t* mac_addr);

sys_sem_t *sys_sem_create(int init_count);
void sys_sem_free(sys_sem_t *sem);
int sys_sem_wait(sys_sem_t *sem, int32_t ms);
void sys_sem_notify(sys_sem_t *sem);

// 互斥信号量：由具体平台实现
sys_mutex_t *sys_mutex_create(void);
void sys_mutex_free(sys_mutex_t *mutex);
void sys_mutex_lock(sys_mutex_t *mutex);
void sys_mutex_unlock(sys_mutex_t *mutex);
int sys_mutex_is_valid(sys_mutex_t *mutex);

// 线程相关：由具体平台实现
typedef void (*sys_thread_func_t)(void * arg);
sys_thread_t *sys_thread_create(sys_thread_func_t entry, void* arg);
void sys_thread_exit (int error);
void sys_sleep(uint32_t ms);
sys_thread_t *sys_thread_self (void);

void sys_plat_init(void);

uint32_t min(uint32_t val1,uint32_t val2);

#endif // SYS_PLAT_H
