#ifndef LOG_H
#define LOG_H

#include <stdlib.h>

#define LOG_INFO 0
#define LOG_WARN 1
#define LOG_ERR 2

#define LOG_LEVEL LOG_INFO

#define LOG_INFO_COL "\033[0m"
#define LOG_WARN_COL "\033[33m"
#define LOG_ERR_COL  "\033[31m"

// 外面不要直接使用这个
void log_msg(char *file,uint32_t line,uint32_t level,char *fmt,...);

#define log_info(fmt,...) log_msg(__FILE_NAME__,__LINE__,LOG_INFO,fmt,##__VA_ARGS__)
#define log_warn(fmt,...) log_msg(__FILE_NAME__,__LINE__,LOG_WARN,fmt,##__VA_ARGS__)
#define log_err(fmt,...) log_msg(__FILE_NAME__,__LINE__,LOG_ERR,fmt,##__VA_ARGS__)

#define assert(expr) \
if(!(expr)){         \
    log_err("assert fail %s",#expr); \
    while(1);                     \
}

#endif