#include "net.h"
#include "log.h"

int main (void) {
    err_code err= net_init();
    if(err<0){
        log_err("net_init err = %d",err);
    }
    net_loop();
}