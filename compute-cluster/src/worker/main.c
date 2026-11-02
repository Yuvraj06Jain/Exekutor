/* OWNER: A. Section 2B.
 * Usage: ./worker [<scheduler_ip> <scheduler_port>] -cores <num> -ram <MB> [-gpu <num>] [-discovery-port <port>]
 * TODO(A): parse args, clamp cores/ram (with warning), discovery_find_scheduler()
 *          then fallback, register, heartbeat every HEARTBEAT_INTERVAL_SEC,
 *          read stdin for `drain`, main message loop. */
#include <signal.h>
#include <stdio.h>
#include "worker/worker.h"

int main(int argc, char **argv) {
    UNUSED(argc); UNUSED(argv);
    signal(SIGPIPE, SIG_IGN);
    fprintf(stderr, "usage: ./worker [<scheduler_ip> <scheduler_port>] -cores <num> -ram <MB> "
                    "[-gpu <num>] [-discovery-port <port>]\n"
                    "(skeleton: worker not implemented yet)\n");
    return 1;
}
