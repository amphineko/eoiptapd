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
#include "utils.h"

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

int create_fork(struct environment_t *env, struct sockaddr_storage *laddr, socklen_t laddr_size, char *if_name) {
    bool cap = true;

    if (socket_open(&(env->sock_fd), laddr, laddr_size) == ENOPROTOOPT)
        cap = false;    // multiple sockets not supported
    if (tap_open(&(env->tap_fd), if_name))
        cap = false;    // multiple tap queue not supported

    env->sock_pid = fork();
    if (env->sock_pid == 0) {
        socket_listen(env);
        exit(EXIT_SUCCESS);
    }
    if (env->sock_pid == -1) {
        fprintf(stderr, "[ERROR] fork(socket): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    env->tap_pid = fork();
    if (env->tap_pid == 0) {
        tap_listen(env);
        exit(EXIT_SUCCESS);
    }
    if (env->tap_pid == -1) {
        fprintf(stderr, "[ERROR] fork(tap): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return cap;
}

int main(int argc, char *argv[]) {
    struct sockaddr_storage *laddr = malloc(sizeof(struct sockaddr_storage));
    socklen_t laddr_size;
    long cpu_count = sysconf(_SC_NPROCESSORS_ONLN);

    char *if_name;

    struct environment_t env = {
            .socket_forward = socket_send,
            .tap_forward = tap_send,
    };

    struct children_t *children = malloc(sizeof(struct children_t) + sizeof(void *) * cpu_count);

    resolve_args(argc, argv, &if_name, &laddr, &laddr_size, &(env.raddr), &(env.raddr_len), (uint16_t *) &(env.tid));

    fprintf(stderr, "[INFO] %ld processors found\n", cpu_count);

    for (long i = 0; i < cpu_count; i++) {
        children->count++;

        children->children[i] = malloc(sizeof(struct environment_t));
        memcpy(children->children[i], &env, sizeof(struct environment_t));

        if (!create_fork(children->children[i], laddr, laddr_size, if_name))
            break;
    }

    waitpid(-1, NULL, 0);
}
