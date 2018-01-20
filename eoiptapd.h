#ifndef EOIPTAPD_H_
#define EOIPTAPD_H_

#include "eoip.h"

/*** program internal ***/

struct environment_t;

typedef void (*socket_forward_func)(struct environment_t *, struct eoip_pkt_t *pkt, size_t payload_size);

typedef void (*tap_forward_func)(struct environment_t *, const char *frame, size_t frame_size);

struct environment_t {
    uint8_t tid;

    int sock_fd, tap_fd;

    socket_forward_func socket_forward;
    tap_forward_func tap_forward;

    struct sockaddr_storage *raddr;
    socklen_t raddr_len;
};

#endif //EOIPTAPD_H_
