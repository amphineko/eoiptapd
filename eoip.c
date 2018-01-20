#include <netinet/in.h>

#include "eoip.h"

void eoip_pkt_hdr_set_size(struct eoip_pkt_hdr_t *hdr, uint16_t size) {
    hdr->size = htons(size);
}
