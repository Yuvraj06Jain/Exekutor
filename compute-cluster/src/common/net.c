#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "common/net.h"

int write_all(int fd, const void *buf, size_t n) {
    const char *p = buf;
    while (n > 0) {
        ssize_t w = write(fd, p, n);
        if (w < 0) { if (errno == EINTR) continue; return -1; }
        p += w; n -= (size_t)w;
    }
    return 0;
}

int read_all(int fd, void *buf, size_t n) {
    char *p = buf;
    while (n > 0) {
        ssize_t r = read(fd, p, n);
        if (r < 0) { if (errno == EINTR) continue; return -1; }
        if (r == 0) return -1;
        p += r; n -= (size_t)r;
    }
    return 0;
}

int send_msg(int fd, uint32_t type, const void *payload, uint32_t len) {
    size_t total = sizeof(MsgHeader) + len;
    char *buf = malloc(total);
    if (!buf) return -1;
    MsgHeader h = { type, len };
    memcpy(buf, &h, sizeof h);
    if (len) memcpy(buf + sizeof h, payload, len);
    int rc = write_all(fd, buf, total);
    free(buf);
    return rc;
}

int recv_msg(int fd, MsgHeader *h, void **payload) {
    MsgHeader raw;
    if (read_all(fd, &raw, sizeof raw) < 0) return -1;
    if (raw.length > MAX_PAYLOAD) return -1;
    void *p = NULL;
    if (raw.length) {
        p = malloc(raw.length);
        if (!p) return -1;
        if (read_all(fd, p, raw.length) < 0) { free(p); return -1; }
    }
    *h = raw;
    *payload = p;
    return 0;
}

int tcp_listen(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
    struct sockaddr_in a;
    memset(&a, 0, sizeof a);
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_ANY);
    a.sin_port = htons((uint16_t)port);
    if (bind(fd, (struct sockaddr *)&a, sizeof a) < 0 || listen(fd, 128) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

int tcp_connect(const char *ip, int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    struct sockaddr_in a;
    memset(&a, 0, sizeof a);
    a.sin_family = AF_INET;
    a.sin_port = htons((uint16_t)port);
    if (inet_pton(AF_INET, ip, &a.sin_addr) != 1 ||
        connect(fd, (struct sockaddr *)&a, sizeof a) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}
