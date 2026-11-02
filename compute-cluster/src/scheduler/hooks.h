/* CONTRACT FILE: functions that one person writes and another calls.
 * Until the owner finishes, a stub in the owner's file keeps everything compiling. */
#ifndef SCHEDULER_HOOKS_H
#define SCHEDULER_HOOKS_H

#include "scheduler/sched.h"

/* ---- C (events.c) ---- */
void emit_event(const char *user, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
void events_conn_closed(Conn *c);          /* B calls this whenever a connection closes */

/* ---- C (deps.c) : Section 8 ---- */
/* On submit. Return 0 to accept (job may become WAITING), -1 to reject (fill err). */
int  deps_on_submit(Job *j, char *err, size_t errlen);
void deps_on_finish(Job *j);               /* B calls when any job reaches a final state */

/* ---- C (quota.c) : Section 11 ---- */
int  quota_should_skip(const Job *j);      /* 1 = skip this job in this pass */

/* ---- C (logbuf.c) : Section 6 ---- */
void logbuf_append(Job *j, const char *data, size_t len);

/* ---- C (filesend.c) : Section 13 ---- */
int  cache_worker_has(const Worker *w, const char *hash);   /* 1 = yes */
int  send_binary(Job *j, Worker *w);       /* FILE_DATA or FILE_ALREADY_CACHED; 0 ok */

/* ---- A (migration.c) : Section 14 ---- */
int  send_checkpoint(Job *j, Worker *w);   /* MSG_CHECKPOINT if job has one; 0 ok */
/* B's pass calls send_checkpoint FIRST, then send_binary, then sends JOB_ASSIGN. */

/* ---- A (cron.c) : Section 12 ---- */
void cron_tick(void);                      /* B calls about once per second */
/* Register an -every template. 0 ok (id in *template_id), -1 error (fill err). */
int  cron_add_template(const JobSpec *spec, const char *user,
                       uint32_t *template_id, char *err, size_t errlen);

#endif
