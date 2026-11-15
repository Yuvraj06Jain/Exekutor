#ifndef CONFIG
#define CONFIG

typedef enum{
    FIFO,
    PRIORITY
}SchedulerAlgo;

typedef struct{
    int port;
    SchedulerAlgo algo;
    int discoveryPort;
}SchedulerConfig;

int sched_config_parse(int argc, char **argv, SchedulerConfig *cfg);
void sched_usage(char *prog);
char *sched_algo_name(SchedulerAlgo a);

#endif