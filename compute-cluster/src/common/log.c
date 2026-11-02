#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "common/log.h"

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void log_line(char *tag, char *fmt, ...){
    char ts[16];
    time_t now = time(NULL);
    struct tm tm;
    localtime_r(&now, &tm);
    strftime(ts, sizeof ts, "%H:%M:%S", &tm);

    pthread_mutex_lock(&log_mutex);
    printf("[%s] [%s] ", ts, tag);
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    putchar('\n');
    fflush(stdout);
    pthread_mutex_unlock(&log_mutex);
}
