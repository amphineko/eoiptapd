#include <assert.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "socket.h"

#define MAX_IP_HDR_SIZE 60

void socket_listen(struct environment_t *env) {
    fd_set *fd_set = malloc(sizeof(fd_set));

    size_t buf_size = sizeof(struct eoip_pkt_t) + MAX_IP_HDR_SIZE;
    char *buf = malloc(buf_size);
    ssize_t buf_read;

    size_t ip_hdr_size, payload_offset;
    struct eoip_pkt_hdr_t pkt_hdr_tpl = {
            .gre_flags = htons(EOIP_GRE_FLAGS),
            .proto = htons(EOIP_PROTO_TYPE),
            .tid = env->tid
    };

    fprintf(stderr, "[INFO] socket listener started\n");

    while (true) {
        FD_ZERO(fd_set);
        FD_SET(env->sock_fd, fd_set);
        select(env->sock_fd + 1, fd_set, NULL, NULL, NULL);
        buf_read = recv(env->sock_fd, buf, buf_size, 0);

        // verify eoip header size
        ip_hdr_size = ((struct ip *) buf)->ip_hl * 4;
        payload_offset = ip_hdr_size + sizeof(struct eoip_pkt_hdr_t);
        if (payload_offset > buf_read)
            continue;

        // verify eoip header content
        eoip_pkt_hdr_set_size(&pkt_hdr_tpl, (uint16_t) (buf_read - payload_offset));
        if (memcmp(buf + ip_hdr_size, &pkt_hdr_tpl, sizeof(struct eoip_pkt_hdr_t)) != 0)
            continue;

        // forward frame
        env->tap_forward(env, buf + payload_offset, pkt_hdr_tpl.size);
    }
}

void socket_send_ex(int fd, const void *buf, size_t buf_size, const struct sockaddr *dst, socklen_t dst_size) {
    ssize_t send_size = -1;
    int se = EAGAIN;

    while ((send_size == -1) && (se == EAGAIN))
        send_size = sendto(fd, buf, buf_size, MSG_DONTWAIT, dst, dst_size), se = errno;

//    if ((send_size == -1) && (errno == EWOULDBLOCK)) {
//        fprintf(stderr, "[WARN] sendto w/ MSG_DONTWAIT failed");
//        send_size = sendto(fd, buf, buf_size, 0, dst, dst_size);
//    }

    if (send_size == -1)
        fprintf(stderr, "[ERROR] sendto: %s\n", strerror(errno));
}

void socket_send(struct environment_t *env, struct eoip_pkt_t *pkt, size_t frame_size) {
    // ensure initialized by callee
    assert(pkt->hdr.gre_flags == htons(EOIP_GRE_FLAGS));
    assert(pkt->hdr.proto == htons(EOIP_PROTO_TYPE));
    assert(pkt->hdr.tid == env->tid);

    // update frame size
    eoip_pkt_hdr_set_size((struct eoip_pkt_hdr_t *) pkt, (uint16_t) frame_size);

    // write to socket
    socket_send_ex(env->sock_fd, pkt, sizeof(struct eoip_pkt_hdr_t) + frame_size, (const struct sockaddr *) env->raddr,
                   env->raddr_len);
}
