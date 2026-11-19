#ifndef WORKER_CONFIG_H
#define WORKER_CONFIG_H

typedef struct{
    int  has_fallback;      // 1 if <scheduler_ip> <scheduler_port> were given
    char fallback_ip[46];
    int  fallback_port;
    int  cores;             // requested capacity (clamped later)
    int  ram_mb;            // requested capacity (clamped later)
    int  gpus;              // just a counter, never clamped
    int  discovery_port;    // UDP discovery port
}WorkerConfig;

int  worker_config_parse(int argc, char **argv, WorkerConfig *cfg);
void worker_usage(char *prog);

#endif
