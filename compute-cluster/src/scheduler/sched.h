/* CONTRACT FILE (scheduler side). Owner of the implementations: B unless noted.
 *
 * LOCKING RULE: dispatch_message() and every hook/handler is called with the
 * scheduler's global state protected (trivially true in an event loop; with
 * threads, B takes the global lock around dispatch). Hooks and handlers must
 * therefore be SHORT and must never block (no sleep, no waiting on a socket
 * except conn_send). Do not take the global lock yourself inside them. */
#ifndef SCHEDULER_SCHED_H
#define SCHEDULER_SCHED_H

#include <pthread.h>
#include "common/types.h"
#include "common/protocol.h"
#include "common/log.h"
#include "common/net.h"

enum { CONN_UNKNOWN = 0, CONN_WORKER, CONN_CLIENT };

typedef struct Conn {
    int      fd;
    int      kind;                   /* CONN_* ; set on REGISTER / CLIENT_LOGIN */
    uint32_t worker_id;              /* when kind == CONN_WORKER */
    char     user[MAX_USERNAME + 1]; /* when kind == CONN_CLIENT */
    pthread_mutex_t write_lock;
} Conn;

typedef void (*MsgHandler)(Conn *c, const MsgHeader *h, const void *payload);

/* ---- B: connection + dispatch (main.c, dispatch.c) ---- */
int  conn_send(Conn *c, uint32_t type, const void *payload, uint32_t len); /* thread-safe */
void sched_lock(void);
void sched_unlock(void);
void dispatch_message(Conn *c, const MsgHeader *h, const void *payload);

/* ---- B: state, queue, workers (state.c, queue.c, workers.c, pass.c) ---- */
void   job_set_state(Job *j, JobState s, const char *reason);  /* THE way to change state */
Job   *job_find(uint32_t id);
Worker*worker_find(uint32_t id);
Job   *job_create(const JobSpec *spec, const char *user);      /* assigns id; NULL on failure */
int    queue_submit(Job *j);                                    /* 0 ok, -1 QUEUE_FULL */
void   job_cancel(Job *j, const char *requested_by);            /* QUEUED / RUNNING / WAITING */
void   sched_trigger(void);                                     /* ask for a scheduling pass soon */

#endif
