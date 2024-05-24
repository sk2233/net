#include "tcp.h"
#include "tool.h"
#include "sys_plat.h"
#include "ipv4.h"
#include "log.h"
#include "timer.h"

uint32_t seq_len(tcp_hdr_t *hdr, package_t *package) {
    // 每个数据占用一位 特殊的 syn fin 也占用一位
    return package->size + hdr->flag_syn + hdr->flag_fin;
}

// 根据已经创建的tcp头进行数据发送
err_code tcp_out(package_t *package, tcp_hdr_t *hdr, ipv4_t desc) {
    uint16_t pre = check_sum_pre(netdev_ip, desc, IPV4_PROTOCOL_TCP, package->size);
    hdr->check_sum = check_sum(package->ptr, package->size, pre, TRUE);
    return ipv4_out(package, IPV4_PROTOCOL_TCP, desc);
}

// 进行tcp reset操作
err_code tcp_reset(tcp_hdr_t *hdr, ipv4_t desc) {
    package_t *package = alloc_package(sizeof(tcp_hdr_t));
    // 设置请求头信息
    tcp_hdr_t *temp = (tcp_hdr_t *) package->ptr;
    // 如果之前有交互的话使用其期望的序号
    if (hdr->ack) {
        temp->seq = hdr->ack;
    }
    temp->src_port = hdr->desc_port;
    temp->desc_port = hdr->src_port;
    temp->flag_rst = 1;
    temp->hdr_size = sizeof(tcp_hdr_t) / 4;
    return tcp_out(package, temp, desc);
}

static tcp_state state;
static uint16_t local_port;
static uint16_t remote_port;
// 发送信息相关变量
static uint32_t next_seq; // 将要发送的数据序号，最开始从 0 开始
// 接受信息相关变量
static uint32_t next_ack; // 下一个存放数据的位置

err_code tcp_init() {
    state = TCP_CLOSE;
    return ERR_CODE_OK;
}

err_code tcp_send_ack(package_t *package, tcp_hdr_t *hdr, ipv4_t ip) {
    // 校验数据
    if (swap16(hdr->src_port) != remote_port || swap16(hdr->desc_port) != local_port ||
        swap32(hdr->ack) != next_seq) { // 这里只发了一个数据应该不会再少了， 数据传输时因为丢包可能导致 swap32(hdr->ack)<next_seq 此时要调整next_seq
        log_warn("tcp_send_ack err");
        return ERR_CODE_SYS;
    }
    if (!hdr->flag_ack || !hdr->flag_syn || hdr->flag_rst) { // 第二次握手数据不对
        return ERR_CODE_SYS;
    }
    // 记录对方第一次发数据的序号值  syn 占用 1
    next_ack = swap32(hdr->seq) + 1;
    package_t *send = alloc_package(sizeof(tcp_hdr_t));
    tcp_hdr_t *temp = (tcp_hdr_t *) send->ptr;
    temp->desc_port = hdr->src_port;
    temp->src_port = hdr->desc_port;
    temp->seq = swap32(next_seq);
    temp->ack = swap32(next_ack);
    temp->flag_ack = 1;
    temp->window = swap16(1024);
    temp->hdr_size = sizeof(tcp_hdr_t) / 4;
    free_package(package);
    state = TCP_ESTABLISHED;
    return tcp_out(send, temp, ip);
}

err_code tcp_resp_close(package_t *package, tcp_hdr_t *hdr, ipv4_t ip) {
    if (swap16(hdr->src_port) != remote_port || swap16(hdr->desc_port) != local_port ||
        swap32(hdr->ack) > next_seq) { // 这里只发了一个数据应该不会再少了， 数据传输时因为丢包可能导致 swap32(hdr->ack)<next_seq 此时要调整next_seq
        log_warn("tcp_resp_close err");
        return ERR_CODE_SYS;
    }
    if (swap32(hdr->ack) < next_seq) { // 调整位置
        next_seq = swap16(hdr->ack);
    }
    if (swap32(hdr->seq) < next_ack) { // 序号可能大于 next_ack
        log_warn("tcp_resp_close err");
        return ERR_CODE_SYS;
    }
    if (!hdr->flag_fin || !hdr->flag_ack) { // 非关闭请求
        log_warn("tcp_resp_close err");
        return ERR_CODE_SYS;
    }
    next_ack = swap32(hdr->seq) + 1; // fin 占用一个位置  还可能有数据，暂时不处理
    package_t *send = alloc_package(sizeof(tcp_hdr_t));
    tcp_hdr_t *temp = (tcp_hdr_t *) send->ptr;
    temp->desc_port = hdr->src_port;
    temp->src_port = hdr->desc_port;
    temp->seq = swap32(next_seq);
    temp->ack = swap32(next_ack);
    temp->flag_ack = 1;
    temp->flag_fin = 1; // 直接合并 第二次与第三次挥手   我知道你要关闭了，我也没数据发了，我也要关闭
    next_seq++;
    temp->window = swap16(1024);
    temp->hdr_size = sizeof(tcp_hdr_t) / 4;
    free_package(package);
//    state=TCP_CLOSE_WAIT; // 发送ack响应
    state = TCP_LAST_ACK; // 处理最后的 ack
    return tcp_out(send, temp, ip);
}

err_code tcp_resp_data(package_t *package, tcp_hdr_t *hdr, ipv4_t ip) {
    return 0;
}

err_code tcp_resp_last_ack(package_t *package, tcp_hdr_t *hdr) {
    if (swap16(hdr->src_port) != remote_port || swap16(hdr->desc_port) != local_port ||
        swap32(hdr->ack) > next_seq) { // 这里只发了一个数据应该不会再少了， 数据传输时因为丢包可能导致 swap32(hdr->ack)<next_seq 此时要调整next_seq
        log_warn("tcp_resp_last_ack err");
        return ERR_CODE_SYS;
    }
    if (swap32(hdr->seq) < next_ack) { // 序号可能大于 next_ack
        log_warn("tcp_resp_last_ack err");
        return ERR_CODE_SYS;
    }
    if (!hdr->flag_ack) { // 非关闭请求
        log_warn("tcp_resp_last_ack err");
        return ERR_CODE_SYS;
    }
    free_package(package); // 正常关闭，切换到关闭状态
    state = TCP_CLOSE;
    return ERR_CODE_OK;
}

// 5s后由 time_wait 到 close 一般  2ML 是 2min 这里为了简单
void time_wait_to_close(void *data){
    state=TCP_CLOSE;
    log_info("time_wait_to_close ok");
}

err_code tcp_resp_wait1(package_t *package, tcp_hdr_t *hdr, ipv4_t ip) {
    if (swap16(hdr->src_port) != remote_port || swap16(hdr->desc_port) != local_port ||
        swap32(hdr->ack) > next_seq) { // 这里只发了一个数据应该不会再少了， 数据传输时因为丢包可能导致 swap32(hdr->ack)<next_seq 此时要调整next_seq
        log_warn("tcp_resp_wait1 err");
        return ERR_CODE_SYS;
    }
    if (swap32(hdr->ack) < next_seq) { // 调整位置
        next_seq = swap16(hdr->ack);
    }
    if (swap32(hdr->seq) < next_ack) { // 序号可能大于 next_ack
        log_warn("tcp_resp_wait1 err");
        return ERR_CODE_SYS;
    }
    if (!hdr->flag_ack) { // 非fin_wait1的响应
        log_warn("tcp_resp_wait1 err");
        return ERR_CODE_SYS;
    }
    if(!hdr->flag_fin){ // 只是简单的 ack  对方接受到了关闭请求，但是他还有数据要发
        state=TCP_FIN_WAIT2;
        return ERR_CODE_OK;
    }// 对方直接回应了 ack + fin
    next_ack = swap32(hdr->seq) + 1; // fin 占用一个位置
    package_t *send = alloc_package(sizeof(tcp_hdr_t));
    tcp_hdr_t *temp = (tcp_hdr_t *) send->ptr;
    temp->desc_port = hdr->src_port;
    temp->src_port = hdr->desc_port;
    temp->seq = swap32(next_seq);
    temp->ack = swap32(next_ack);
    temp->flag_ack = 1;
    temp->window = swap16(1024);
    temp->hdr_size = sizeof(tcp_hdr_t) / 4;
    free_package(package);
    state = TCP_TIME_WAIT; // 直接进入time_wait
    add_event(time_wait_to_close,NULL,5*1000);
    return tcp_out(send, temp, ip);
}

err_code tcp_resp_wait2(package_t *package, tcp_hdr_t *hdr, ipv4_t ip) {
    if (swap16(hdr->src_port) != remote_port || swap16(hdr->desc_port) != local_port ||
        swap32(hdr->ack) > next_seq) { // 这里只发了一个数据应该不会再少了， 数据传输时因为丢包可能导致 swap32(hdr->ack)<next_seq 此时要调整next_seq
        log_warn("tcp_resp_wait2 err");
        return ERR_CODE_SYS;
    }
    if (swap32(hdr->ack) < next_seq) { // 调整位置
        next_seq = swap16(hdr->ack);
    }
    if (swap32(hdr->seq) < next_ack) { // 序号可能大于 next_ack
        log_warn("tcp_resp_wait2 err");
        return ERR_CODE_SYS;
    }
    if (!hdr->flag_ack||!hdr->flag_fin) { // 非fin_wait2的响应
        log_warn("tcp_resp_wait2 err");
        return ERR_CODE_SYS;
    }
    next_ack = swap32(hdr->seq) + 1; // fin 占用一个位置
    package_t *send = alloc_package(sizeof(tcp_hdr_t));
    tcp_hdr_t *temp = (tcp_hdr_t *) send->ptr;
    temp->desc_port = hdr->src_port;
    temp->src_port = hdr->desc_port;
    temp->seq = swap32(next_seq);
    temp->ack = swap32(next_ack);
    temp->flag_ack = 1;
    temp->window = swap16(1024);
    temp->hdr_size = sizeof(tcp_hdr_t) / 4;
    free_package(package);
    state = TCP_TIME_WAIT; // 进入time_wait
    add_event(time_wait_to_close,NULL,5*1000);
    return tcp_out(send, temp, ip);
}

err_code tcp_in(package_t *package, ipv4_t src) {
    if (package->size < sizeof(tcp_hdr_t)) {
        return ERR_CODE_SYS;
    }
    tcp_hdr_t *hdr = (tcp_hdr_t *) package->ptr;
    if (package->size < hdr->hdr_size * 4) {
        return ERR_CODE_SYS;
    }
    if (hdr->check_sum) {
        uint16_t pre = check_sum_pre(src, netdev_ip, IPV4_PROTOCOL_TCP, package->size);
        uint16_t temp = hdr->check_sum; // 计算校验和必须原来 check_sum = 0
        hdr->check_sum = 0;
        if (check_sum(package->ptr, package->size, pre, TRUE) != temp) {
            return ERR_CODE_SYS;
        }
    }
    remove_header(package, hdr->hdr_size * 4); // 移除整个头部包含选项数据
    switch (state) {
        case TCP_CLOSE:
            // 改阶段不处理输入
            log_warn("state = close can't in");
            return ERR_CODE_SYS;
        case TCP_SYN_SEND: // 接受到第二次握手，进行第三次握手
            return tcp_send_ack(package, hdr, src);
            // TODO  TEST 主动关闭
//            return tcp_send_fin(src);
        case TCP_FIN_WAIT1:
            return tcp_resp_wait1(package,hdr,src);
        case TCP_FIN_WAIT2:
        case TCP_TIME_WAIT: // fin_wait2 回应完可能丢包对方重发，这里也重新响应 fin_wait2
            return tcp_resp_wait2(package,hdr,src);
        case TCP_ESTABLISHED:
            if (hdr->flag_fin) { // 响应对方的关闭请求
                return tcp_resp_close(package, hdr, src);
            } else {// 响应数据
                return tcp_resp_data(package, hdr, src);
            }
        case TCP_LAST_ACK:
            return tcp_resp_last_ack(package, hdr);
        default:
            return ERR_CODE_SYS;
    }
}

// 主动关闭链接
err_code tcp_send_fin(ipv4_t ip) {
    if (state != TCP_ESTABLISHED) {
        log_warn("err state %d", state);
        return ERR_CODE_SYS;
    }
    package_t *package = alloc_package(sizeof(tcp_hdr_t));
    tcp_hdr_t *hdr = (tcp_hdr_t *) package->ptr;
    hdr->src_port = swap16(local_port);
    hdr->desc_port = swap16(remote_port);
    hdr->seq = swap32(next_seq);
    hdr->ack = swap32(next_ack);
    hdr->flag_fin = 1;
    hdr->flag_ack = 1;
    next_seq++; //  fin占用一个数据
    hdr->window = swap16(1024);
    hdr->hdr_size = sizeof(tcp_hdr_t) / 4;
    state = TCP_FIN_WAIT1; // 切换状态
    return tcp_out(package, hdr, ip);
}

// 创建连接
err_code tcp_send_syn(ipv4_t ip, uint16_t port) {
    if (state != TCP_CLOSE) {
        log_warn("err state %d", state);
        return ERR_CODE_SYS;
    }
    // 当前认为单线程模型 处理连接要使用的数据
    local_port = alloc_port();
    remote_port = port;
    next_seq = 0;
//    next_ack=0; // 这里还不清楚 next_ack
    // 构造数据包
    package_t *package = alloc_package(sizeof(tcp_hdr_t));
    tcp_hdr_t *hdr = (tcp_hdr_t *) package->ptr;
    hdr->src_port = swap16(local_port);
    hdr->desc_port = swap16(port);
    hdr->seq = swap32(next_seq);
//    hdr->ack= swap32(next_ack); // 第一次建立连接对方也会随机序号
    hdr->flag_syn = 1;
    next_seq++; //  syn占用一个数据
    hdr->window = swap16(1024);
    hdr->hdr_size = sizeof(tcp_hdr_t) / 4;
    state = TCP_SYN_SEND; // 切换状态
    return tcp_out(package, hdr, ip);
}