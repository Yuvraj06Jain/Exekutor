/* CONTRACT FILE: one handler per message type that arrives at the scheduler.
 * The table that connects type -> handler lives in dispatch.c (B).
 * Each handler is written by the person named; stubs exist until then. */
#ifndef SCHEDULER_HANDLERS_H
#define SCHEDULER_HANDLERS_H
#include "scheduler/sched.h"

#define HANDLER(name) void name(Conn *c, const MsgHeader *h, const void *payload)

/* B */
HANDLER(on_register);        /* workers.c   */
HANDLER(on_worker_drain);    /* workers.c   */
HANDLER(on_nodes_req);       /* workers.c   */
HANDLER(on_heartbeat);       /* heartbeat.c */
HANDLER(on_job_update);      /* state.c     */
HANDLER(on_job_output);      /* state.c : thin, calls logbuf_append */
HANDLER(on_submit);          /* queue.c     */
HANDLER(on_status_req);      /* queue.c     */
HANDLER(on_cancel_req);      /* queue.c     */
/* A */
HANDLER(on_checkpoint);      /* migration.c */
HANDLER(on_migrate_req);     /* migration.c */
HANDLER(on_cron_list_req);   /* cron.c      */
HANDLER(on_cancel_cron_req); /* cron.c      */
/* C */
HANDLER(on_client_login);    /* events.c    */
HANDLER(on_logs_req);        /* logbuf.c    */
HANDLER(on_cache_report);    /* filesend.c  */
HANDLER(on_file_missing);    /* filesend.c  */

#endif
