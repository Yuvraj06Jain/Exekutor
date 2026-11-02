/* OWNER: B. Section 2A + 4A.
 * Usage: ./scheduler <port> <FIFO|PRIORITY> [-discovery-port <port>]
 * TODO(B): parse args, print the startup lines from the brief, call
 *          discovery_responder_start(), pick the concurrency model, accept loop. */
#include <signal.h>
#include <stdio.h>
#include "scheduler/hooks.h"

int conn_send(Conn *c, uint32_t type, const void *payload, uint32_t len) {
    pthread_mutex_lock(&c->write_lock);
    int rc = send_msg(c->fd, type, payload, len);
    pthread_mutex_unlock(&c->write_lock);
    return rc;
}

/* No-ops until B decides the concurrency model (event loop needs none). */
void sched_lock(void)   { }
void sched_unlock(void) { }

int main(int argc, char **argv) {
    UNUSED(argc); UNUSED(argv);
    signal(SIGPIPE, SIG_IGN);
    fprintf(stderr, "usage: ./scheduler <port> <FIFO|PRIORITY> [-discovery-port <port>]\n"
                    "(skeleton: scheduler not implemented yet)\n");
    return 1;
}
