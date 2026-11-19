#include <signal.h>
#include <stdio.h>

#include "common/log.h"
#include "worker/config.h"
#include "worker/capacity.h"

int main(int argc, char **argv){
    WorkerConfig cfg;
    if(worker_config_parse(argc, argv, &cfg) < 0){
        worker_usage(argv[0]);
        return 1;
    }

    signal(SIGPIPE, SIG_IGN); // writing to a closed socket returns an error instead of killing us

    clamp_capacity(&cfg); // never advertise more than the machine really has

    // TEMPORARY: replaced in the next section (clamping, discovery, registration)
    log_line("WORKER", "Parsed: cores=%d ram=%dMB gpus=%d discovery-port=%d fallback=%s:%d",
             cfg.cores, cfg.ram_mb, cfg.gpus, cfg.discovery_port,
             cfg.has_fallback ? cfg.fallback_ip : "none", cfg.fallback_port);
    return 0;
}
