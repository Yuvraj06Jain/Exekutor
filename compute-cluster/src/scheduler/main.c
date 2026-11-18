/* main.c  (OWNER: B, Section 2A)
 *
 * Entry point of ./scheduler. Section 2A only covers STARTING the scheduler:
 *
 *   1. read the command line                    (config.c)
 *   2. open the TCP port workers/clients use    (tcp_listen, common/net.c)
 *   3. start answering UDP discovery            (discovery_responder_start, common/discovery.c)
 *   4. print that it is ready
 *
 * What the spec expects to see (timestamps come from log_line, Section 7):
 *   [SCHEDULER] Started on port 8080. Algorithm: PRIORITY
 *   [SCHEDULER] Discovery responder listening on UDP port 9999.
 *   [SCHEDULER] Listening for incoming connections...
 * The middle line is printed by discovery_responder_start() itself.
 *
 * Accepting connections and handling messages is Section 4A (the concurrency
 * model), which gets added at the bottom of main(). Until then main() returns
 * right after the ready line, so the process exits there.
 */
// [S] ./scheduler entry point: parse args, open TCP port, start discovery (Section 2A)
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "common/discovery.h"
#include "common/log.h"
#include "common/net.h"
#include "scheduler/config.h"

int main(int argc, char **argv){
    /* 1. Command line. On a bad line: say what's wrong, show the usage, and
     *    exit with status 1 (non-zero = "failed" to the shell). */
    // [S] Parse the command line; bad input -> usage + exit 1
    SchedulerConfig cfg;
    if(sched_config_parse(argc, argv, &cfg) < 0){
        sched_usage(argv[0]);
        return 1;
    }

    /* Writing to a socket whose other end has closed (a worker that died, a
     * client that quit) makes the OS send the process SIGPIPE, and SIGPIPE's
     * default action is to KILL the process. The scheduler must survive
     * workers vanishing (Section 4C), so ignore SIGPIPE: the write then just
     * fails with an error (EPIPE), which send_msg reports as -1.
     * ASSUMPTION: done here at startup since it applies to the whole process. */
    // [S] Ignore SIGPIPE so writing to a closed socket returns an error instead of killing us
    signal(SIGPIPE, SIG_IGN);

    /* 2. Open the TCP port BEFORE announcing "Started", so we never claim to
     *    be running on a port we couldn't get (e.g. already in use). */
    // [S] Open the TCP listening socket first; exit if the port can't be bound
    int listen_fd = tcp_listen(cfg.port);
    if(listen_fd < 0){
        log_line("SCHEDULER", "Cannot listen on TCP port %d (in use or not allowed).", cfg.port);
        return 1;
    }

    log_line("SCHEDULER", "Started on port %d. Algorithm: %s", cfg.port, sched_algo_name(cfg.algo));

    /* 3. Start answering discovery requests. It runs in its own thread and
     *    prints "Discovery responder listening on UDP port N." itself.
     * ASSUMPTION: if the UDP port can't be bound, the scheduler keeps running
     * instead of exiting. Workers and clients can still reach it through the
     * fallback address on their command line (Section 2B/3), and the failure
     * is logged by discovery_responder_start(). */
    // [S] Start the UDP discovery responder (own thread)
    // [S] ASSUMPTION: if discovery can't start we keep running; the CLI fallback address still works
    if(discovery_responder_start(cfg.discovery_port, cfg.port) < 0)
        log_line("SCHEDULER", "Continuing without discovery; workers/clients must use the fallback address.");

    /* 4. Ready. */
    // [S] Ready message
    log_line("SCHEDULER", "Listening for incoming connections...");

    return 0;
}
