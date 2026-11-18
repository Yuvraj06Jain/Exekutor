/* config.c  (OWNER: B, Section 2A)
 *
 * Reads and checks the scheduler's command line. See config.h for the format.
 * Nothing here touches the network; it only turns strings into a
 * SchedulerConfig, or explains what is wrong with them.
 */
// [S] Command-line parsing for the scheduler (Section 2A)
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/types.h"
#include "scheduler/config.h"

char *sched_algo_name(SchedulerAlgo a){
    return (a == PRIORITY) ? "PRIORITY" : "FIFO";
}

void sched_usage(char *prog){
    fprintf(stderr,
            "Usage: %s <port> <FIFO|PRIORITY> [-discovery-port <port>]\n",
            prog);
}

/* Turns text like "8080" into a port number.
 * strtoll does the conversion; `end` is left pointing at the first character
 * it could NOT convert. So:
 *   - end == s      -> no digits at all ("abc")
 *   - *end != '\0'  -> junk after the number ("80x")
 *   - errno != 0    -> number too big to even fit in a long long
 * Then the value must be a real port, 1..65535 (0 means "any port" to the OS,
 * which makes no sense for a server everyone has to find).
 * Returns 0 and stores the port in *res, or -1 if the text isn't a valid port. */
// [S] Text -> port number in 1..65535; 0 = ok, -1 = invalid
static int parse_port(char *s, int *res){
    if(s == NULL || *s == '\0')
        return -1;

    char *end = NULL;
    errno = 0;
    long long v = strtoll(s, &end, 10);

    if(errno != 0 || end == s || *end != '\0')
        return -1;
    if(v < 1 || v > 65535)
        return -1;

    *res = (int)v;
    return 0;
}

/* Checks the arguments in order:
 *   argv[1] = TCP port, argv[2] = algorithm, then optional "-discovery-port N".
 * Anything else on the line is an error, so typos don't go unnoticed.
 *
 * ASSUMPTION: the algorithm name is case-sensitive: exactly "FIFO" or
 * "PRIORITY", the way the spec writes them. "fifo" is rejected.
 * ASSUMPTION: giving -discovery-port twice is an error rather than
 * "last one wins". */
// [S] Validate <port> <FIFO|PRIORITY> [-discovery-port <port>]
// [S] ASSUMPTION: algorithm is case-sensitive; repeating -discovery-port is an error
int sched_config_parse(int argc, char **argv, SchedulerConfig *cfg){
    /* Start from the defaults; only -discovery-port is optional. */
    // [S] Defaults
    cfg->port = 0;
    cfg->algo = FIFO;
    cfg->discovery_port = DEFAULT_DISCOVERY_PORT;

    if(argc < 3){
        fprintf(stderr, "scheduler: expected <port> and <scheduling_algorithm>\n");
        return -1;
    }

    if(parse_port(argv[1], &cfg->port) < 0){
        fprintf(stderr, "scheduler: invalid port '%s' (expected 1-65535)\n", argv[1]);
        return -1;
    }

    if(strcmp(argv[2], "FIFO") == 0)
        cfg->algo = FIFO;
    else if(strcmp(argv[2], "PRIORITY") == 0)
        cfg->algo = PRIORITY;
    else{
        fprintf(stderr, "scheduler: unknown scheduling algorithm '%s' (expected FIFO or PRIORITY)\n", argv[2]);
        return -1;
    }

    /* Optional flags after the two required arguments. The only one that
     * exists is "-discovery-port <port>", and it needs a value after it. */
    // [S] Optional flags: only -discovery-port <port> is allowed
    int seen_discovery = 0;
    for(int i = 3; i < argc; i++){
        if(strcmp(argv[i], "-discovery-port") != 0){
            fprintf(stderr, "scheduler: unknown argument '%s'\n", argv[i]);
            return -1;
        }
        if(seen_discovery){
            fprintf(stderr, "scheduler: -discovery-port given more than once\n");
            return -1;
        }
        if(i + 1 >= argc){
            fprintf(stderr, "scheduler: -discovery-port needs a port number\n");
            return -1;
        }
        i++;        /* move on to the value */
        if(parse_port(argv[i], &cfg->discovery_port) < 0){
            fprintf(stderr, "scheduler: invalid discovery port '%s' (expected 1-65535)\n", argv[i]);
            return -1;
        }
        seen_discovery = 1;
    }

    return 0;
}
