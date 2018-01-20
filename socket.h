#ifndef SOCKET_H_
#define SOCKET_H_

#include "eoip.h"
#include "eoiptapd.h"

void socket_listen(struct environment_t *env);

void socket_send(struct environment_t *env, struct eoip_pkt_t *pkt, size_t frame_size);

#endif
