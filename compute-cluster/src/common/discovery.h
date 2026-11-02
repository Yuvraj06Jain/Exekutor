#ifndef COMMON_DISCOVERY_H
#define COMMON_DISCOVERY_H
#include <stddef.h>
/* Owner: C (Section 2C).
 * Worker/client side: broadcast on udp_port (2 s wait, 3 tries).
 * Returns 0 and fills ip_out and *tcp_port_out if a scheduler replied, -1 otherwise
 * (caller then uses the command-line fallback). */
int discovery_find_scheduler(int udp_port, char *ip_out, size_t ip_len, int *tcp_port_out);

/* Scheduler side: start answering discovery requests on udp_port, replying
 * with this scheduler's tcp_port. Must not block the caller. 0 ok, -1 error. */
int discovery_responder_start(int udp_port, int tcp_port);
#endif
