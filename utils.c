#include <errno.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include "eoip.h"
#include "utils.h"

void socket_open(int *fd, const struct sockaddr_storage *addr, socklen_t size) {
    int buf_size = 2 * sizeof(struct eoip_pkt_t), e;

    if ((*fd = socket(AF_INET, SOCK_RAW, 47)) == -1) {
        fprintf(stderr, "[ERROR] socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (bind(*fd, (const struct sockaddr *) addr, size) != 0) {
        fprintf(stderr, "[ERROR] bind: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    e = setsockopt(*fd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size)) |
        setsockopt(*fd, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size));
    if (e != 0) {
        fprintf(stderr, "[ERROR] setsockopt(socket): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void tap_open(int *fd, const char *if_name) {
    struct ifreq ifreq = {
            .ifr_flags = IFF_NO_PI | IFF_TAP
    };
    strncpy(ifreq.ifr_name, if_name, IFNAMSIZ);

    *fd = open("/dev/net/tun", O_RDWR);

    if (ioctl(*fd, TUNSETIFF, &ifreq) != 0) {
        fprintf(stderr, "[ERROR] ioctl(TUNSETIFF): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}