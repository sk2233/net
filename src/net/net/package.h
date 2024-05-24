#ifndef PACKAGE_H
#define PACKAGE_H
#include "type.h"

// 源mac(6) 目的mac(6) 类型(2) 数据(46~1500)
#define PACKAGE_SIZE (6+6+2+1500)
#define MIN_PACKAGE_SIZE (6+6+2+46)

#pragma pack(1)
typedef struct package{
    uint8_t data[PACKAGE_SIZE]; // 使用动态分配的内存值 这里只保留其开始位置 每次不足翻倍分配 使用动态数组
    uint8_t *ptr; // 当前位置
    uint32_t size; // 动态内存的大小
}package_t;
#pragma pack()

package_t *alloc_package(uint32_t size);
void free_package(package_t *package);
void extend_package(package_t *package,uint32_t size);
void reduce_package(package_t *package,uint32_t size);
void *add_header(package_t *package,uint32_t size);
void *remove_header(package_t *package,uint32_t size);

#endif