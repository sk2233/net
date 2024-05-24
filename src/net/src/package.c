#include <string.h>
#include "package.h"

package_t *alloc_package(uint32_t size){
    package_t *package= malloc(sizeof(package_t));
    memset(package,0, sizeof(package_t));
    package->size=size;
    package->ptr=package->data+PACKAGE_SIZE-size;
    return package;
}

void free_package(package_t *package){
    free(package);
}

void extend_package(package_t *package,uint32_t size){// 若比最小还小，调整包为最小
    if(package->size>=size){
        return;
    }
    uint32_t offset = size-package->size;
    uint8_t *ptr=package->ptr-offset;
    for (int i = 0; i < package->size; ++i) {
        *ptr++=*package->ptr++;
    }
    for (int i = 0; i < offset; ++i) {
        *ptr++=0;
    }
    package->size=size;
    package->ptr=package->data+PACKAGE_SIZE-size;
}

void reduce_package(package_t *package,uint32_t size){
    if(package->size<=size){
        return;
    }
    uint8_t *desc=package->data+PACKAGE_SIZE;
    uint8_t *src=desc-(package->size-size);
    for (int i = 0; i < size; ++i) {
        *--desc=*--src;
    }
    package->size=size;
    package->ptr=package->data+PACKAGE_SIZE-size;
}

void *add_header(package_t *package,uint32_t size){
    package->size+=size;
    package->ptr-=size;
    memset(package->ptr,0,size);
    return package->ptr;
}

void *remove_header(package_t *package,uint32_t size){
    package->size-=size;
    package->ptr+=size;
    return package->ptr;
}