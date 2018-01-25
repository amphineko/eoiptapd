#ifndef UTILS_H_
#define UTILS_H_

int socket_open(int *fd, const struct sockaddr_storage *addr, socklen_t size);

void tap_open(int *fd, const char *if_name);

#endif //UTILS_H_
