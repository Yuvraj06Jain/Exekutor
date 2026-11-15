/* log.h
 *
 * The ONE logging function the spec asks for (Section 7). Every state change,
 * scheduling decision and discovery outcome in the scheduler and worker should
 * be printed through it, so all lines share the same format.
 */
#ifndef COMMON_LOG_H
#define COMMON_LOG_H

/* Prints one line to stdout as   [HH:MM:SS] [TAG] message
 * Used like printf, with the tag in front:
 *      log_line("SCHEDULER", "Job %u RUNNING -> COMPLETED", id);
 *      -> [14:03:21] [SCHEDULER] Job 7 RUNNING -> COMPLETED
 * Tags are uppercase: "SCHEDULER", "WORKER 1002", "CLIENT", ...
 * Don't put "\n" at the end of fmt; log_line adds it.
 * Safe to call from several threads at once (see log.c). */
void log_line(char *tag, char *fmt, ...);

#endif
