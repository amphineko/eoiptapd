#ifndef BASE_H_
#define BASE_H_

#include <stdint.h>

/*** eoip protocol ***/

#define EOIP_GRE_FLAGS 0x2001
#define EOIP_PROTO_TYPE 0x6400

#define MAX_PAYLOAD_SIZE UINT16_MAX

struct eoip_pkt_hdr_t {
    uint16_t gre_flags;     // gre flags    = 0x2001
    uint16_t proto;         // protocol     = 0x6400
    uint16_t size;          // ethernet frame length    [BE]
    uint16_t tid;           // tunnel id                [LE]
};

struct eoip_pkt_t {
    struct eoip_pkt_hdr_t hdr;
    char payload[MAX_PAYLOAD_SIZE];
};

void eoip_pkt_hdr_set_size(struct eoip_pkt_hdr_t *hdr, uint16_t size);

#endif
