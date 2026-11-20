#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include "common/log.h"
#include "worker/capacity.h"
#include "worker/config.h"
#include "worker/connect.h"
#include "worker/loop.h"

int main(int argc, char **argv){
    WorkerConfig cfg;
    if(worker_config_parse(argc, argv, &cfg) < 0){
        worker_usage(argv[0]);
        return 1;
    }

    signal(SIGPIPE, SIG_IGN); // writing to a closed socket returns an error instead of killing us

    clamp_capacity(&cfg); // never advertise more than the machine really has

    int sock = connect_to_scheduler(&cfg);
    if(sock < 0)
        return 1;

    uint32_t worker_id;
    if(register_with_scheduler(sock, &cfg, &worker_id) < 0){
        close(sock);
        return 1;
    }

    // From now on every log line carries our ID: "WORKER 1001"
    char tag[32];
    snprintf(tag, sizeof tag, "WORKER %u", worker_id);
    log_line(tag, "Registered successfully. ID: %u. Capacity: %d Cores, %dMB RAM, %d GPUs.",
             worker_id, cfg.cores, cfg.ram_mb, cfg.gpus);

    run_loop(sock, worker_id, tag);

    // TEMPORARY: a later section kills running jobs and registers again as a new worker
    close(sock);
    return 1;
}
