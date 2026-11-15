#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/types.h"
#include "src/scheduler/config.h"

char *sched_algo_name(SchedAlgo a){
    return (a == ALGO_PRIORITY) ? "PRIORITY" : "FIFO";
}

void sched_usage(char *prog){
    fprintf(stderr,
            "Usage: %s <port> <FIFO|PRIORITY> [-discovery-port <port>]\n",
            prog);
}

int parse_port(char* s, int* res){
    if(s == NULL || *s == '\0')
        return -1;

    char* end = NULL;
    errno = 0;
    long v = strtoll(s, &end, 10);

    if(errno !=0 || end == s || *end != '\0'){
        return -1;
    }
    if(v<1 || v>65535){
        return -1;
    }

    *res = (int)v;
    return 0;
}

int scheduler_config_parse(int argc, char** argv, SchedulerConfig *cfg){
    cfd->port = 0;
    cfg->algo = ALGO_FIFO;
    cfd->discovery_port = DEFAULT_DISCOVERY_PORT;

    if(argc < 3){
        fprintf(stderr, "scheduler: expected <port> and <scheduling_algorithm>\n");
        return -1;
    }

     if(parse_port(argv[1], &cfg->port) < 0){
        fprintf(stderr, "scheduler: invalid port '%s' (expected 1-65535)\n", argv[1]);
        return -1;
    }

    if(!strcmp(argv[2], "FIFO"))
        cfg->algo = FIFO;
    else if(!strcmp(argv[2], "PRIORITY"))
        cfg->algo = PRIORITY;
    else{
        fprintf(stderr, "scheduler: unknown scheduling algorithm '%s' (expected FIFO or PRIORITY)\n", argv[2]);
        return -1;
    }

    int seen_discovery = 0;
    for(int i = 3; i < argc; i++){
        if(!strcmp(argv[i], "-discovery-port")){
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
        i++;
        if(parse_port(argv[i], &cfg->discovery_port) < 0){
            fprintf(stderr, "scheduler: invalid discovery port '%s' (expected 1-65535)\n", argv[i]);
            return -1;
        }
        seen_discovery = 1;
    }

    return 0;
}