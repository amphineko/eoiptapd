#include <errno.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <wait.h>

#include "eoip.h"
#include "eoiptapd.h"
#include "socket.h"
#include "tap.h"

void resolve_addr(const char *addr, struct sockaddr_storage **out, socklen_t *out_size) {
    struct addrinfo hints = {
            .ai_family = AF_INET,
            .ai_socktype = SOCK_RAW,
            .ai_protocol = 47,
    }, *result;
    int e;

    if ((e = getaddrinfo(addr, NULL, &hints, &result)) != 0) {
        fprintf(stderr, "[ERROR] getaddrinfo: %s\n", gai_strerror(e));
        exit(EXIT_FAILURE);
    }

    for (; result != NULL; result = result->ai_next)
        if (result->ai_family == AF_INET) {
            *out = malloc(result->ai_addrlen);
            memcpy(*out, result->ai_addr, result->ai_addrlen);
            *out_size = result->ai_addrlen;

            freeaddrinfo(result);
            return;
        }

    fprintf(stderr, "[ERROR] unable to bind specified address");
    exit(EXIT_FAILURE);
}

void resolve_args(int argc, char *argv[], char **if_name, struct sockaddr_storage **laddr_out,
                  socklen_t *laddr_size, struct sockaddr_storage **raddr_out, socklen_t *raddr_size, uint16_t *tid) {
    int opt;
    bool if_name_set = false;
    bool laddr_set = false;
    bool raddr_set = false;

    while ((opt = getopt(argc, argv, "i:l:r:t:")) != -1) {
        switch (opt) {
            case 'i':
                *if_name = malloc(strlen(optarg));
                strcpy(*if_name, optarg);
                if_name_set = true;
                break;
            case 'l':
                resolve_addr(optarg, laddr_out, laddr_size);
                laddr_set = true;
                break;
            case 'r':
                resolve_addr(optarg, raddr_out, raddr_size);
                raddr_set = true;
                break;
            case 't':
                *tid = (uint16_t) atoi(optarg);
                break;
            default:
                fprintf(stderr, "usage: %s -l local -r remote\n", argv[0]);
                break;
        }
    }

    if (if_name_set)
        if (laddr_set)
            if (raddr_set)
                return;
            else
                fprintf(stderr, "[ERROR] remote address is required\n");
        else
            fprintf(stderr, "[ERROR] local address is required\n");
    else
        fprintf(stderr, "[ERROR] interface name is required\n");

    exit(EXIT_FAILURE);
}

void socket_open(int *fd, const struct sockaddr_storage *addr, socklen_t size) {
    if ((*fd = socket(AF_INET, SOCK_RAW, 47)) == -1) {
        fprintf(stderr, "[ERROR] socket: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (bind(*fd, (const struct sockaddr *) addr, size) != 0) {
        fprintf(stderr, "[ERROR] bind: %s\n", strerror(errno));
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

int main(int argc, char *argv[]) {
    struct sockaddr_storage *laddr = malloc(sizeof(struct sockaddr_storage));
    socklen_t laddr_size;

    char *if_name;

    struct environment_t env = {
            .socket_forward = socket_send,
            .tap_forward = tap_send,
    };

    resolve_args(argc, argv, &if_name, &laddr, &laddr_size, &(env.raddr), &(env.raddr_len), (uint16_t *) &(env.tid));

    socket_open(&(env.sock_fd), laddr, laddr_size);
    tap_open(&(env.tap_fd), if_name);

    int sock = fork();
    if (sock == 0) {
        socket_listen(&env);
        exit(EXIT_SUCCESS);
    }
    if (sock == -1) {
        fprintf(stderr, "[ERROR] fork(socket): %s\n", strerror(errno));
    }

    int tap = fork();
    if (tap == 0) {
        tap_listen(&env);
        exit(EXIT_SUCCESS);
    }
    if (sock == -1) {
        fprintf(stderr, "[ERROR] fork(tap): %s\n", strerror(errno));
    }

    waitpid(-1, NULL, 0);
}
