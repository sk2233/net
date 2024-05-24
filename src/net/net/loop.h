#ifndef OUT_H
#define OUT_H
#include "type.h"
#include "package.h"

err_code loop_init();
err_code write_package(package_t *package,int32_t ms);
package_t *read_package(int32_t ms);

#endif