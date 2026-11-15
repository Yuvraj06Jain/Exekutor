/* net.h
 *
 * Small helpers for TCP: sending and receiving whole messages (the framing
 * described at the top of protocol.h), and opening sockets.
 * Used by all three programs. The code is in net.c.
 */
#ifndef COMMON_NET_H
#define COMMON_NET_H

#include <stddef.h>
#include <stdint.h>
#include "common/protocol.h"

/* write()/read() may move FEWER bytes than asked: a "short" write or read.
 * These two keep looping until exactly n bytes have been moved.
 * Both return 0 on success and -1 on error. read_all also returns -1 if the
 * other side closed the connection before n bytes arrived. */
int write_all(int fd, void *buf, size_t n);
int read_all(int fd, void *buf, size_t n);


/* Sends one message: the header {type, len} followed by `len` bytes of payload.
 * Both are glued into one buffer and written together, so a message is never
 * split by something else being written in between.
 * If several threads may send on the SAME socket, they must still take turns
 * (e.g. a mutex per connection), or their messages could get mixed up.
 * Returns 0 on success, -1 on error. */
int send_msg(int fd, uint32_t type, void *payload, uint32_t len);


/* Receives one message. Blocks until a whole message has arrived.
 * On success: returns 0, fills *h, and sets *payload to a malloc'd buffer of
 * h->length bytes (or NULL when length is 0). The CALLER must free(*payload).
 * Returns -1 on error, if the connection closed, or if the header claims more
 * than MAX_PAYLOAD bytes. */
int recv_msg(int fd, MsgHeader *h, void **payload);

/* Scheduler: open a TCP socket listening on `port` on every network interface.
 * Returns the listening socket (pass it to accept()), or -1 on failure. */
int tcp_listen(int port);

/* Worker/client: connect to ip:port (ip as text like "192.168.1.20").
 * Returns the connected socket, or -1 on failure. */
int tcp_connect(char *ip, int port);

#endif
