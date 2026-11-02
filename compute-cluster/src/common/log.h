#ifndef COMMON_LOG_H
#define COMMON_LOG_H
/* The ONE logging function. Prints "[HH:MM:SS] [TAG] message".
 * Tags are uppercase: "SCHEDULER", "WORKER 1002", "CLIENT", "DISCOVERY". */
void log_line(const char *tag, const char *fmt, ...) __attribute__((format(printf, 2, 3)));

/* Put in a stub so it is obvious in the output what is still missing. */
#define NOT_IMPLEMENTED(name) log_line("STUB", "%s is not implemented yet", (name))
#endif
