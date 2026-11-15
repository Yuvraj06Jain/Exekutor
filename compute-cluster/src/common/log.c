/* log.c
 *
 * The central log_line() function (spec Section 7). See log.h for how to use it.
 */
#include <pthread.h>
#include <stdarg.h>     /* va_list etc.: lets log_line take a variable number of arguments, like printf */
#include <stdio.h>
#include <time.h>
#include "common/log.h"

/* One lock shared by every thread. The scheduler has several threads logging
 * at once; without the lock, two lines could get mixed up character by
 * character in the terminal. */
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void log_line(char *tag, char *fmt, ...){
    /* Build the "HH:MM:SS" timestamp (24-hour, local time).
     * localtime_r is the thread-safe version of localtime. */
    char ts[16];
    time_t now = time(NULL);
    struct tm tm;
    localtime_r(&now, &tm);
    strftime(ts, sizeof ts, "%H:%M:%S", &tm);

    /* Hold the lock while printing, so this line comes out in one piece. */
    pthread_mutex_lock(&log_mutex);
    printf("[%s] [%s] ", ts, tag);

    /* Hand the "..." arguments to vprintf, which formats fmt with them,
     * just like printf would. */
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    putchar('\n');
    fflush(stdout);     /* show it right away, even when stdout goes to a file or a pipe */
    pthread_mutex_unlock(&log_mutex);
}
