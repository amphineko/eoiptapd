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

int socket_open(int *fd, const struct sockaddr_storage *addr, socklen_t size) {
    int buf_size = 2 * sizeof(struct eoip_pkt_t), e, one = 1, ret;

    /* open socket */
    if ((*fd = socket(addr->ss_family, SOCK_RAW, 47)) == -1) {
        fprintf(stderr, "[ERROR] socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* set so_reuseport */
    if (setsockopt(*fd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one)) != 0) {
        fprintf(stderr, "[WARN] setsockopt(SO_REUSEPORT): %s\n", strerror(errno));
        ret = errno == ENOPROTOOPT ? ENOPROTOOPT : -1;
    } else {
        fprintf(stderr, "[INFO] setsockopt(SO_REUSEPORT) succeeded\n");
        ret = 0;
    }

    /* bind socket */
    if (bind(*fd, (const struct sockaddr *) addr, size) != 0) {
        fprintf(stderr, "[ERROR] bind: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* set socket buffer */
    e = setsockopt(*fd, SOL_SOCKET, SO_RCVBUF, &buf_size, sizeof(buf_size)) |
        setsockopt(*fd, SOL_SOCKET, SO_SNDBUF, &buf_size, sizeof(buf_size));
    if (e != 0) {
        fprintf(stderr, "[ERROR] setsockopt(socket): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return ret;
}

int tap_open(int *fd, const char *if_name) {
    struct ifreq ifreq = {};
    strncpy(ifreq.ifr_name, if_name, IFNAMSIZ);

    /* open /dev/net/tun */
    if ((*fd = open("/dev/net/tun", O_RDWR)) < 0) {
        fprintf(stderr, "[ERROR] open(/dev/net/tun): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    ifreq.ifr_flags = IFF_TAP | IFF_NO_PI | IFF_MULTI_QUEUE;
    if (ioctl(*fd, TUNSETIFF, &ifreq) != 0) {
        if (errno == EINVAL) {
            ifreq.ifr_flags = IFF_TAP | IFF_NO_PI;
            if (ioctl(*fd, TUNSETIFF, &ifreq) == 0) {
                fprintf(stderr, "[WARN] multi-queue tap not supported\n");
                return EINVAL;
            } else; // exit w/ error
        } else; // exit w/ error
    } else
        return 0;

    fprintf(stderr, "[ERROR] ioctl(TUNSETIFF): %s\n", strerror(errno));
    exit(EXIT_FAILURE);
}