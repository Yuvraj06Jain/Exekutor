#ifndef COMMON_NET_H
#define COMMON_NET_H
#include <stddef.h>
#include <stdint.h>
#include "common/protocol.h"

int write_all(int fd, const void *buf, size_t n);   /* 0 ok, -1 error */
int read_all(int fd, void *buf, size_t n);          /* 0 ok, -1 error or peer closed */

/* Sends header + payload as ONE write. If several threads send on the same fd,
 * serialize them yourself (the scheduler's conn_send does this). */
int send_msg(int fd, uint32_t type, const void *payload, uint32_t len);

/* Blocking. On success *payload is malloc'd (NULL if length==0): caller frees. */
int recv_msg(int fd, MsgHeader *h, void **payload);

int tcp_listen(int port);                      /* listening fd, or -1 */
int tcp_connect(const char *ip, int port);     /* connected fd, or -1 */
#endif
