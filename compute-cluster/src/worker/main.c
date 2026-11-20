#include <signal.h>
#include <stdio.h>

#include "common/log.h"
#include "worker/config.h"
#include "worker/capacity.h"
#include "worker/connect.h"

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

    // TEMPORARY: replaced in the next section (registration)
    log_line("WORKER", "Connected to scheduler.");
    close(sock);

    return 0;
}
