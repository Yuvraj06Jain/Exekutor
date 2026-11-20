#include "common/discovery.h"
#include "common/log.h"
#include "common/net.h"
#include "worker/connect.h"

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
