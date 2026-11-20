#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "common/discovery.h"
#include "common/log.h"
#include "common/net.h"
#include "common/protocol.h"
#include "worker/connect.h"
#define REGISTER_TIMEOUT_SEC 5

static int try_connect(char *ip, int port){
    log_line("WORKER", "Connecting to scheduler at %s:%d...", ip, port);
    int fd = tcp_connect(ip, port);
    if(fd < 0)
        log_line("WORKER", "Could not connect to scheduler at %s:%d.", ip, port);
    return fd;
}

// ASSUMPTION: if discovery finds a scheduler but the TCP connection to it fails,
// we still try the fallback address (when there is one) before giving up.
int connect_to_scheduler(WorkerConfig *cfg){
    char ip[64];
    int port = 0;

    discovery_set_log_tag("WORKER"); // discovery logs the lookup and every attempt itself

    if(discovery_find_scheduler(cfg->discovery_port, ip, sizeof ip, &port) == 0){
        int fd = try_connect(ip, port);
        if(fd >= 0)
            return fd;
        if(!cfg->has_fallback)
            return -1;
        log_line("WORKER", "Discovered scheduler unreachable, trying fallback %s:%d.",
                 cfg->fallback_ip, cfg->fallback_port);
    }
    else{
        if(!cfg->has_fallback){
            log_line("WORKER", "Error: no discovery reply and no fallback address given. Exiting.");
            return -1;
        }
        log_line("WORKER", "No discovery reply, using fallback %s:%d",
                 cfg->fallback_ip, cfg->fallback_port);
    }

    return try_connect(cfg->fallback_ip, cfg->fallback_port);
}

// ASSUMPTION: if the scheduler does not answer the registration within
// REGISTER_TIMEOUT_SEC seconds we give up instead of waiting forever.
int register_with_scheduler(int sock, WorkerConfig *cfg, uint32_t *worker_id){
    RegisterMsg reg;
    reg.cores = (uint32_t)cfg->cores;
    reg.ram_mb = (uint32_t)cfg->ram_mb;
    reg.gpus = (uint32_t)cfg->gpus;

    if(send_msg(sock, MSG_REGISTER, &reg, sizeof reg) < 0){
        log_line("WORKER", "Error: could not send registration to the scheduler.");
        return -1;
    }

    // Do not block forever waiting for the ack; undone right after the reply
    struct timeval tv = { REGISTER_TIMEOUT_SEC, 0 };
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);

    MsgHeader h;
    void *payload = NULL;
    int rc = recv_msg(sock, &h, &payload);

    struct timeval none = { 0, 0 };
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &none, sizeof none);

    if(rc < 0){
        log_line("WORKER", "Error: no registration reply from the scheduler.");
        return -1;
    }
    if(h.type != MSG_REGISTER_ACK || h.length != sizeof(RegisterAck)){
        log_line("WORKER", "Error: unexpected reply (type %u) to registration.", h.type);
        free(payload);
        return -1;
    }

    RegisterAck *ack = payload;
    *worker_id = ack->worker_id;
    free(payload);
    return 0;
}
