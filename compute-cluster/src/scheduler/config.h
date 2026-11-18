/* config.h  (OWNER: B, Section 2A)
 *
 * Everything the scheduler learns from its command line:
 *
 *      ./scheduler <port> <scheduling_algorithm> [-discovery-port <port>]
 *
 *   <port>                  TCP port that workers and clients connect to
 *   <scheduling_algorithm>  FIFO or PRIORITY (decides the ORDER queued jobs are tried in, Sec 4B)
 *   -discovery-port <port>  UDP port for discovery (Sec 2C); optional, defaults to 9999
 *
 * main.c calls sched_config_parse() once at startup and keeps the result.
 */
// [S] Scheduler command-line settings (Section 2A)
#ifndef SCHEDULER_CONFIG_H
#define SCHEDULER_CONFIG_H

/* The two scheduling algorithms the spec allows. FIFO is 0, so a zeroed
 * config means FIFO. */
// [S] Scheduling algorithm chosen on the command line
typedef enum{
    ALGO_FIFO,
    ALGO_PRIORITY
}SchedulerAlgo;

/* The parsed command line. Filled in by sched_config_parse(). */
// [S] Parsed command-line settings
typedef struct{
    int port;               /* TCP port to listen on (1..65535) */            // [S] TCP port
    SchedulerAlgo algo;     /* FIFO or PRIORITY */                            // [S] algorithm
    int discovery_port;     /* UDP discovery port (1..65535, default 9999) */ // [S] UDP discovery port
}SchedulerConfig;

/* Parses argv into *cfg. Returns 0 if the command line is valid.
 * On any problem it prints ONE line to stderr saying what is wrong and
 * returns -1; the caller then prints the usage line and exits. */
// [S] Parse argv into cfg; 0 = ok, -1 = invalid (error already printed)
int sched_config_parse(int argc, char **argv, SchedulerConfig *cfg);

/* Prints the usage line to stderr, e.g.
 *   Usage: ./scheduler <port> <FIFO|PRIORITY> [-discovery-port <port>] */
// [S] Print the usage line
void sched_usage(char *prog);

/* "FIFO" or "PRIORITY", for log lines like "... Algorithm: PRIORITY". */
// [S] Algorithm name as text
char *sched_algo_name(SchedulerAlgo a);

#endif
