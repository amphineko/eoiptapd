#ifndef EOIPTAPD_H_
#define EOIPTAPD_H_

#include "eoip.h"

/*** program internal ***/

struct environment_t;

typedef void (*socket_forward_func)(struct environment_t *, struct eoip_pkt_t *pkt, size_t payload_size);

typedef void (*tap_forward_func)(struct environment_t *, const char *frame, size_t frame_size);

struct environment_t {
    uint16_t tid;

    int sock_fd, tap_fd;

    pid_t sock_pid, tap_pid;

    socket_forward_func socket_forward;
    tap_forward_func tap_forward;

    struct sockaddr_storage *raddr;
    socklen_t raddr_len;
};

struct children_t {
    int count;
    struct environment_t *children[];
};

#endif //EOIPTAPD_H_
