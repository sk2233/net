#include <stdio.h>
#include "log.h"
#include <stdarg.h>

void log_msg(char *file,uint32_t line,uint32_t level,char *fmt,...){
    if(level<LOG_LEVEL){
        return;
    }
    char buff[128];
    va_list args;
    va_start(args,fmt);
    vsprintf(buff,fmt,args);
    va_end(args);
    char *col=LOG_INFO_COL;
    if(level==LOG_WARN){
        col=LOG_WARN_COL;
    } else if(level==LOG_ERR){
        col=LOG_ERR_COL;
    }
    printf("%s[%s:%d] %s%s\n",col,file,line,buff,LOG_INFO_COL);
}