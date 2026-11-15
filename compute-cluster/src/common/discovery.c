/* discovery.c
 *
 * UDP discovery (spec Section 2C), written by C. See discovery.h for the big
 * picture and the packet format.
 *
 * The file has two halves:
 *   1. FINDER    - run by the worker and the client: broadcast a question,
 *                  wait for an answer, try again a few times.
 *   2. RESPONDER - run by the scheduler in a background thread: wait for
 *                  questions forever and answer each one.
 *
 * WHY UDP? TCP needs to know the address before connecting, and that is
 * exactly what we don't know yet. UDP can send one packet to a broadcast
 * address, which every machine on the local network receives.
 */

/* Without these two, Linux's headers hide some of what this file uses
 * (getifaddrs, the IFF_* flags). They must come before any #include.
 * Harmless on macOS. */
#define _GNU_SOURCE
#define _DEFAULT_SOURCE

#include <arpa/inet.h>      /* inet_ntop (binary IP -> text), htons/htonl/ntohl (byte-order conversion) */
#include <errno.h>
#include <ifaddrs.h>        /* getifaddrs: list this machine's network interfaces (wifi, ethernet, ...) */
#include <net/if.h>         /* IFF_UP, IFF_BROADCAST, IFF_LOOPBACK: flags describing an interface */
#include <netinet/in.h>
#include <poll.h>           /* poll: wait for a packet, but only up to a time limit */
#include <pthread.h>        /* the responder runs in its own thread */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>           /* clock_gettime, for measuring the timeout */
#include <unistd.h>

#include "common/discovery.h"
#include "common/log.h"
#include "common/types.h"   /* DEFAULT_DISCOVERY_PORT, DISCOVERY_TIMEOUT, DISCOVERY_RETRIES */


/* 8-letter markers at the start of every packet: one for questions, one for
 * answers. Any packet on the port that doesn't start with the right one
 * (another program, another team) is ignored. They're compared with memcmp
 * over exactly 8 bytes, so the string's trailing '\0' never matters. */
#define FIND_MAGIC "DCC_FIND"
#define HERE_MAGIC "DCC_HERE"

/* The one packet format, used both ways:
 *   question: magic = "DCC_FIND", tcp_port = 0
 *   answer:   magic = "DCC_HERE", tcp_port = scheduler's TCP port
 * 8 + 4 = 12 bytes with no padding, so the struct can be sent as it is.
 * Unlike the TCP messages in protocol.h, the port here IS converted to
 * network byte order (htonl when sending, ntohl when receiving). */
typedef struct {
    char magic[8];
    uint32_t tcp_port;
} DiscoveryPacket;

/* Tag for the finder's log lines ("WORKER", "CLIENT", ...).
 * `static` means it's only visible inside this file. */
static char log_tag[32] = "DISCOVERY";

void discovery_set_log_tag(char *tag){
    if(tag == NULL)
        return;
    snprintf(log_tag, sizeof log_tag, "%s", tag);    /* copies safely: never writes past the 32 bytes */
}


/* ======================================================================
 *  1. FINDER (worker and client side)
 * ====================================================================== */

/* Current time in milliseconds. CLOCK_MONOTONIC only ever moves forward,
 * even if someone changes the system clock, so it's safe for timeouts. */
static long long now_ms(void){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

/* Sends one question packet to one IPv4 address.
 * `ip` is in host byte order (like the constants INADDR_BROADCAST and
 * INADDR_LOOPBACK); it's converted with htonl when filling in the address. */
static void send_request_to(int fd, uint32_t ip, int port){
    DiscoveryPacket req;
    memset(&req, 0, sizeof req);            /* tcp_port = 0 */
    memcpy(req.magic, FIND_MAGIC, 8);

    struct sockaddr_in to;
    memset(&to, 0, sizeof to);
    to.sin_family = AF_INET;
    to.sin_port = htons((uint16_t)port);
    to.sin_addr.s_addr = htonl(ip);

    /* The result is deliberately not checked: some interfaces can't send a
     * broadcast, or aren't connected. That's fine as long as one of the
     * destinations works; if none does, we simply get no answer and time out. */
    sendto(fd, &req, sizeof req, 0, (struct sockaddr *)&to, sizeof to);
}

/* Sends the question to every place the scheduler might hear it:
 *   1. 255.255.255.255  - the general "everyone on this network" broadcast.
 *   2. 127.0.0.1        - this same machine, for when the scheduler runs on
 *                         the same laptop (a broadcast doesn't always reach
 *                         our own machine).
 *   3. each interface's own broadcast address (e.g. 192.168.1.255), because
 *      255.255.255.255 only leaves through ONE interface, which may be the
 *      wrong one on a laptop with several (wifi + hotspot, say).
 * So the scheduler may get the same question several times and answer each;
 * the finder just uses the first answer. */
static void send_to_all(int fd, int port){
    send_request_to(fd, INADDR_BROADCAST, port);
    send_request_to(fd, INADDR_LOOPBACK, port);

    struct ifaddrs *list = NULL;
    if(getifaddrs(&list) < 0)       /* can't list the interfaces: the two sends above will have to do */
        return;

    /* getifaddrs gives a linked list with one entry per interface address. */
    for(struct ifaddrs *a = list; a != NULL; a = a->ifa_next){
        /* keep only IPv4 interfaces that are up, can broadcast, and aren't loopback */
        if(a->ifa_addr == NULL || a->ifa_addr->sa_family != AF_INET)
            continue;
        if(!(a->ifa_flags & IFF_UP) || !(a->ifa_flags & IFF_BROADCAST))
            continue;
        if(a->ifa_flags & IFF_LOOPBACK)
            continue;
        if(a->ifa_broadaddr == NULL)
            continue;

        struct sockaddr_in *b = (struct sockaddr_in *)a->ifa_broadaddr;
        send_request_to(fd, ntohl(b->sin_addr.s_addr), port);   /* ntohl because send_request_to expects host order */
    }

    freeifaddrs(list);              /* getifaddrs allocated the list; give it back */
}

/* Waits up to timeout_ms for a valid answer.
 * Returns 0 and fills ip_out / *port_out when one arrives, -1 on timeout.
 *
 * Invalid packets (wrong size, wrong marker, impossible port) are skipped.
 * The remaining time is recomputed on every loop, so a stream of junk packets
 * can't stretch the wait past timeout_ms. */
static int wait_for_reply(int fd, int timeout_ms, char *ip_out, size_t ip_len, int *port_out){
    long long start = now_ms();

    while(1){
        long long left = timeout_ms - (now_ms() - start);
        if(left <= 0)
            return -1;

        /* poll() sleeps until the socket has a packet to read, or until
         * `left` milliseconds pass, whichever comes first. */
        struct pollfd p;
        p.fd = fd;
        p.events = POLLIN;          /* "tell me when there's something to read" */
        p.revents = 0;

        int ready = poll(&p, 1, (int)left);
        if(ready < 0){
            if(errno == EINTR)      /* a signal interrupted the wait: wait again for the time that's left */
                continue;
            return -1;
        }
        if(ready == 0)              /* time's up, nothing arrived */
            return -1;

        /* recvfrom also tells us WHO sent the packet (`from`). */
        DiscoveryPacket rep;
        struct sockaddr_in from;
        socklen_t from_len = sizeof from;
        ssize_t n = recvfrom(fd, &rep, sizeof rep, 0, (struct sockaddr *)&from, &from_len);

        if(n != (ssize_t)sizeof rep)                    /* not exactly 12 bytes: not ours */
            continue;
        if(memcmp(rep.magic, HERE_MAGIC, 8) != 0)       /* not an answer */
            continue;

        uint32_t port = ntohl(rep.tcp_port);
        if(port == 0 || port > 65535)                   /* not a usable port number */
            continue;

        /* The scheduler's IP is the address the answer came from, so it
         * doesn't need to be inside the packet. inet_ntop turns it into text
         * and fails if ip_out is too small. */
        if(inet_ntop(AF_INET, &from.sin_addr, ip_out, (socklen_t)ip_len) == NULL)
            continue;

        *port_out = (int)port;
        return 0;
    }
}

int discovery_find_scheduler(int udp_port, char *ip_out, size_t ip_len, int *tcp_port_out){
    if(udp_port <= 0)
        udp_port = DEFAULT_DISCOVERY_PORT;

    log_line(log_tag, "Looking for scheduler via UDP discovery (port %d)...", udp_port);

    /* SOCK_DGRAM = UDP. The socket isn't bound to a port: the OS picks a
     * free one, and the scheduler answers to whatever port that is. */
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0){
        log_line(log_tag, "Discovery failed: cannot create UDP socket (%s).", strerror(errno));
        return -1;
    }

    /* The OS refuses to send to a broadcast address unless SO_BROADCAST is on. */
    int yes = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof yes) < 0){
        log_line(log_tag, "Discovery failed: cannot enable broadcast (%s).", strerror(errno));
        close(fd);
        return -1;
    }

    /* UDP packets can get lost, so: send, wait, and if nobody answers, try
     * again, up to DISCOVERY_RETRIES times (3 tries x 2 s by default). */
    for(int attempt = 1; attempt <= DISCOVERY_RETRIES; attempt++){
        send_to_all(fd, udp_port);

        if(wait_for_reply(fd, DISCOVERY_TIMEOUT * 1000, ip_out, ip_len, tcp_port_out) == 0){
            log_line(log_tag, "Scheduler found at %s:%d (UDP discovery, attempt %d).",
                     ip_out, *tcp_port_out, attempt);
            close(fd);
            return 0;
        }

        log_line(log_tag, "No discovery reply (attempt %d/%d).", attempt, DISCOVERY_RETRIES);
    }

    /* Out of attempts. The caller now uses the command-line fallback address
     * (and logs that it did), or exits if there isn't one. */
    log_line(log_tag, "Discovery failed after %d attempts.", DISCOVERY_RETRIES);
    close(fd);
    return -1;
}


/* ======================================================================
 *  2. RESPONDER (scheduler side)
 * ====================================================================== */

/* pthread_create can pass only ONE pointer to the new thread, so the two
 * values the thread needs are packed into this struct. */
typedef struct {
    int fd;             /* the UDP socket, already bound to the discovery port */
    int tcp_port;       /* the scheduler's TCP port, sent back in every answer */
} ResponderArgs;

/* The responder thread. Loops for as long as the scheduler runs:
 * wait for a packet -> check it's a real question -> send the answer back. */
static void *responder_thread(void *arg){
    /* Copy the settings onto this thread's stack, then free the heap copy
     * that discovery_responder_start() allocated, so it isn't leaked. */
    ResponderArgs a = *(ResponderArgs *)arg;
    free(arg);

    while(1){
        DiscoveryPacket req;
        struct sockaddr_in from;
        socklen_t from_len = sizeof from;

        /* Sleeps here until a packet arrives on the discovery port.
         * `from` gets the asker's address so we know where to answer. */
        ssize_t n = recvfrom(a.fd, &req, sizeof req, 0, (struct sockaddr *)&from, &from_len);
        if(n < 0){
            if(errno == EINTR)      /* interrupted by a signal: keep listening */
                continue;
            /* A real socket error: stop answering. The rest of the scheduler keeps running. */
            log_line("SCHEDULER", "Discovery responder stopped: %s", strerror(errno));
            close(a.fd);
            return NULL;
        }

        /* Ignore anything that isn't exactly a 12-byte question. */
        if(n != (ssize_t)sizeof req || memcmp(req.magic, FIND_MAGIC, 8) != 0)
            continue;

        /* Build the answer: the HERE marker + our TCP port in network byte order. */
        DiscoveryPacket rep;
        memset(&rep, 0, sizeof rep);
        memcpy(rep.magic, HERE_MAGIC, 8);
        rep.tcp_port = htonl((uint32_t)a.tcp_port);

        /* Send it straight back to the address the question came from. */
        if(sendto(a.fd, &rep, sizeof rep, 0, (struct sockaddr *)&from, from_len) < 0){
            log_line("SCHEDULER", "Discovery reply failed: %s", strerror(errno));
            continue;
        }

        /* Log who asked (Section 7 wants every discovery outcome logged). */
        char ip[INET_ADDRSTRLEN] = "?";
        inet_ntop(AF_INET, &from.sin_addr, ip, sizeof ip);
        log_line("SCHEDULER", "Discovery request from %s answered (TCP port %d).", ip, a.tcp_port);
    }
}

int discovery_responder_start(int udp_port, int tcp_port){
    if(udp_port <= 0)
        udp_port = DEFAULT_DISCOVERY_PORT;

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0){
        log_line("SCHEDULER", "Discovery responder: cannot create socket (%s).", strerror(errno));
        return -1;
    }

    /* The spec requires that restarting the scheduler right away doesn't fail
     * with "address already in use", so allow reusing the address/port.
     * SO_REUSEPORT doesn't exist on every system, hence the #ifdef. */
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
#ifdef SO_REUSEPORT
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof yes);
#endif

    /* Bind = claim the discovery port on every interface (INADDR_ANY), so
     * broadcasts arriving from any network reach us. */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)udp_port);

    if(bind(fd, (struct sockaddr *)&addr, sizeof addr) < 0){
        log_line("SCHEDULER", "Discovery responder: cannot bind UDP port %d (%s).",
                 udp_port, strerror(errno));
        close(fd);
        return -1;
    }

    /* Allocated on the heap, not the stack: this function returns right away,
     * and the thread still needs the values afterwards. The thread frees it. */
    ResponderArgs *args = malloc(sizeof *args);
    if(args == NULL){
        close(fd);
        return -1;
    }
    args->fd = fd;
    args->tcp_port = tcp_port;

    /* Start the answering loop in its own thread, so the scheduler's main
     * thread can carry on. "Detached" means nobody will pthread_join it: its
     * resources are released automatically, and it lives until the program exits. */
    pthread_t tid;
    if(pthread_create(&tid, NULL, responder_thread, args) != 0){
        log_line("SCHEDULER", "Discovery responder: cannot start thread.");
        free(args);
        close(fd);
        return -1;
    }
    pthread_detach(tid);

    log_line("SCHEDULER", "Discovery responder listening on UDP port %d.", udp_port);
    return 0;
}
