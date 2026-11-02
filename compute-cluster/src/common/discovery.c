/* OWNER: C. STUB until Section 2C is implemented. */
#include "common/discovery.h"
#include "common/log.h"
#include "common/types.h"

int discovery_find_scheduler(int udp_port, char *ip_out, size_t ip_len, int *tcp_port_out) {
    UNUSED(udp_port); UNUSED(ip_out); UNUSED(ip_len); UNUSED(tcp_port_out);
    NOT_IMPLEMENTED("discovery_find_scheduler");
    return -1;               /* -1 = not found: callers use the fallback address */
}

int discovery_responder_start(int udp_port, int tcp_port) {
    UNUSED(udp_port); UNUSED(tcp_port);
    NOT_IMPLEMENTED("discovery_responder_start");
    return 0;
}
