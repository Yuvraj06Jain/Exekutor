#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/types.h"
#include "worker/config.h"

void worker_usage(char *prog){
    fprintf(stderr, "Usage: %s [<scheduler_ip> <scheduler_port>] -cores <num> -ram <MB> "
    "[-gpu <num>] [-discovery-port <port>]\n", prog);
}

// Parses a whole string as an integer in [min, max]. Returns 0 on success.
static int parse_int(char *s, long long min, long long max, int *out){
    if(s == NULL || *s == '\0')
        return -1;

    char *end = NULL;
    errno = 0;
    long long v = strtoll(s, &end, 10);

    if(errno != 0 || end == s || *end != '\0')
        return -1;
    if(v < min || v > max)
        return -1;

    *out = (int)v;
    return 0;
}

// Reads the value that follows a flag (argv[*i]) and moves *i onto it.
static int flag_value(int argc, char **argv, int *i, long long min, long long max, int *out){
    char *flag = argv[*i];
    if(*i + 1 >= argc){
        fprintf(stderr, "worker: %s needs a value\n", flag);
        return -1;
    }
    (*i)++;
    if(parse_int(argv[*i], min, max, out) < 0){
        fprintf(stderr, "worker: invalid value '%s' for %s (expected %lld-%lld)\n",
                argv[*i], flag, min, max);
        return -1;
    }
    return 0;
}

// ASSUMPTION: the fallback <ip> <port> pair, if present, comes first and is IPv4
// (tcp_connect uses inet_pton with AF_INET). Repeating a flag is an error.
int worker_config_parse(int argc, char **argv, WorkerConfig *cfg){
    memset(cfg, 0, sizeof *cfg);
    cfg->discovery_port = DEFAULT_DISCOVERY_PORT;
    cfg->gpus = 0;

    int i = 1;

    // Optional fallback address: first argument does not start with '-'
    if(i < argc && argv[i][0] != '-'){
        if(i + 1 >= argc){
            fprintf(stderr, "worker: scheduler_ip given without scheduler_port\n");
            return -1;
        }
        struct in_addr tmp;
        if(inet_pton(AF_INET, argv[i], &tmp) != 1){
            fprintf(stderr, "worker: invalid scheduler ip '%s'\n", argv[i]);
            return -1;
        }
        snprintf(cfg->fallback_ip, sizeof cfg->fallback_ip, "%s", argv[i]);
        if(parse_int(argv[i + 1], 1, 65535, &cfg->fallback_port) < 0){
            fprintf(stderr, "worker: invalid scheduler port '%s' (expected 1-65535)\n", argv[i + 1]);
            return -1;
        }
        cfg->has_fallback = 1;
        i += 2;
    }

    int seen_cores = 0, seen_ram = 0, seen_gpu = 0, seen_disc = 0;

    for(; i < argc; i++){
        if(strcmp(argv[i], "-cores") == 0){
            if(seen_cores){ fprintf(stderr, "worker: -cores given more than once\n"); return -1; }
            if(flag_value(argc, argv, &i, 1, 65535, &cfg->cores) < 0) return -1;
            seen_cores = 1;
        }
        else if(strcmp(argv[i], "-ram") == 0){
            if(seen_ram){ fprintf(stderr, "worker: -ram given more than once\n"); return -1; }
            if(flag_value(argc, argv, &i, 1, 16777216, &cfg->ram_mb) < 0) return -1;
            seen_ram = 1;
        }
        else if(strcmp(argv[i], "-gpu") == 0){
            if(seen_gpu){ fprintf(stderr, "worker: -gpu given more than once\n"); return -1; }
            if(flag_value(argc, argv, &i, 0, 65535, &cfg->gpus) < 0) return -1;
            seen_gpu = 1;
        }
        else if(strcmp(argv[i], "-discovery-port") == 0){
            if(seen_disc){ fprintf(stderr, "worker: -discovery-port given more than once\n"); return -1; }
            if(flag_value(argc, argv, &i, 1, 65535, &cfg->discovery_port) < 0) return -1;
            seen_disc = 1;
        }
        else{
            fprintf(stderr, "worker: unknown argument '%s'\n", argv[i]);
            return -1;
        }
    }

    if(!seen_cores || !seen_ram){
        fprintf(stderr, "worker: -cores and -ram are required\n");
        return -1;
    }
    return 0;
}
