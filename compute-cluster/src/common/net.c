/* net.c
 *
 * TCP helpers used for all scheduler <-> worker and scheduler <-> client
 * communication. See net.h for what each function promises, and protocol.h
 * for the message format (8-byte header + payload).
 */
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "common/net.h"


/* write() can return after writing only part of the buffer (common with big
 * buffers on sockets). Keep writing the rest until all n bytes are out. */
int write_all(int fd, void *buf, size_t n){
    char *p = buf;                  /* char * so we can move forward byte by byte */
    while(n > 0){
        ssize_t w = write(fd, p, n);
        if(w < 0){
            if (errno == EINTR)     /* a signal interrupted the call before anything was written: just retry */
                continue;
            return -1;              /* a real error, e.g. the other side is gone */
        }

        p += w;                     /* skip the part that was written */
        n -= (size_t)w;
    }

    return 0;
}

/* Same idea for reading: read() may return fewer bytes than asked, because
 * data arrives over the network in pieces. Keep reading until n bytes are in. */
int read_all(int fd, void *buf, size_t n) {
    char *p = buf;
    while(n > 0){
        ssize_t r = read(fd, p, n);
        if(r < 0){
            if (errno == EINTR)     /* interrupted by a signal: retry */
                continue;
            return -1;
        }

        if(r == 0)                  /* 0 = the other side closed the connection; the message will never complete */
            return -1;

        p += r;
        n -= (size_t)r;
    }

    return 0;
}


/* Glue header + payload into one buffer and send it with one write_all, so
 * the message goes out as one continuous block of bytes.
 * Note: the result of malloc isn't checked; if it ever failed, this would crash. */
int send_msg(int fd, uint32_t type, void *payload, uint32_t len){
    size_t total = sizeof(MsgHeader) + len;
    char *buf = malloc(total);

    MsgHeader h = { type, len };
    memcpy(buf, &h, sizeof h);                      /* first 8 bytes: the header */
    if(len)
        memcpy(buf + sizeof h, payload, len);       /* then the payload right after it */

    int rc = write_all(fd, buf, total);
    free(buf);

    return rc;
}


/* Read the fixed 8-byte header first; it tells us how many payload bytes follow. */
int recv_msg(int fd, MsgHeader *h, void **payload){
    MsgHeader raw;
    if(read_all(fd, &raw, sizeof raw) < 0)
        return -1;
    if(raw.length > MAX_PAYLOAD)        /* nonsense length (corrupt data or not our protocol): give up */
        return -1;

    void *p = NULL;                     /* stays NULL for messages with no payload */
    if(raw.length){
        p = malloc(raw.length);

        if(read_all(fd, p, raw.length) < 0){
            free(p);                    /* don't leak the buffer if the read fails halfway */
            return -1;
        }
    }

    /* Only fill the caller's variables once the whole message has arrived. */
    *h = raw;
    *payload = p;                       /* the caller now owns this and must free() it */

    return 0;
}

/* Create a server socket: socket() -> bind() to the port -> listen(). */
int tcp_listen(int port){
    int fd = socket(AF_INET, SOCK_STREAM, 0);       /* AF_INET = IPv4, SOCK_STREAM = TCP */
    if(fd < 0)
        return -1;

    /* SO_REUSEADDR: lets the scheduler restart right away on the same port.
     * Without it, bind() can fail with "address already in use" for a while
     * after the previous run closed. */
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    struct sockaddr_in a;
    memset(&a, 0, sizeof a);
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_ANY);          /* accept connections arriving on any network interface */
    a.sin_port = htons((uint16_t)port);             /* socket addresses must be in network byte order, hence htons/htonl */

    /* 128 = how many connections may wait to be accept()ed at once */
    if(bind(fd, (struct sockaddr *)&a, sizeof a) < 0 || listen(fd, 128) < 0){
        close(fd);
        return -1;
    }

    return fd;
}

/* Create a client socket and connect it to ip:port. */
int tcp_connect(char *ip, int port){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0)
        return -1;

    struct sockaddr_in a;
    memset(&a, 0, sizeof a);
    a.sin_family = AF_INET;
    a.sin_port = htons((uint16_t)port);

    /* inet_pton turns the text "192.168.1.20" into the 4-byte binary address.
     * It returns 1 on success; anything else means the text wasn't a valid IPv4 address. */
    if (inet_pton(AF_INET, ip, &a.sin_addr) != 1 || connect(fd, (struct sockaddr *)&a, sizeof a) < 0){
        close(fd);
        return -1;
    }

    return fd;
}
