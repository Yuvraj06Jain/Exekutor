/* OWNER: C. Section 3.
 * Usage: ./client [<scheduler_ip> <scheduler_port>] [-user <username>] [-discovery-port <port>]
 * TODO(C): discovery then fallback, login, client> prompt using poll() on stdin
 *          AND the socket so [Event] lines show up while idle. */
#include <signal.h>
#include <stdio.h>
#include "client/client.h"

int main(int argc, char **argv) {
    UNUSED(argc); UNUSED(argv);
    signal(SIGPIPE, SIG_IGN);
    commands_init();
    fprintf(stderr, "usage: ./client [<scheduler_ip> <scheduler_port>] [-user <username>] "
                    "[-discovery-port <port>]\n(skeleton: client not implemented yet)\n");
    return 1;
}
