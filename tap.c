#include <netinet/in.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "eoiptapd.h"

void tap_listen(struct environment_t *env) {
    fd_set *fd_set = malloc(sizeof(fd_set));

    struct eoip_pkt_t *pkt = malloc(sizeof(struct eoip_pkt_t));
    ssize_t buf_read;

    // make socket_send happy
    pkt->hdr.gre_flags = htons(EOIP_GRE_FLAGS);
    pkt->hdr.proto = htons(EOIP_PROTO_TYPE);
    pkt->hdr.tid = env->tid;

    fprintf(stderr, "[INFO] tap device listener started\n");

    while (true) {
        // read from tap device
        FD_ZERO(fd_set);
        FD_SET(env->tap_fd, fd_set);
        select(env->tap_fd + 1, fd_set, NULL, NULL, NULL);
        buf_read = read(env->tap_fd, pkt->payload, MAX_PAYLOAD_SIZE);   // read to struct, reduce memory copy

        // forward frame
        env->socket_forward(env, pkt, (size_t) buf_read);
    }
}

void tap_send(struct environment_t *env, const char *frame, size_t frame_size) {
    // write to tap device
    write(env->tap_fd, frame, frame_size);
}
