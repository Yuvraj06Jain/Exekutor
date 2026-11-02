/* CONTRACT FILE, Milestone 1 subset (spec Sections 2, 3, 4A-B, 5A-B, 7).
 * Everything that only Milestone 2+ needs was left out on purpose.
 * Later additions must be additive: append new struct fields at the END of a
 * struct, add new constants, and announce it in the group chat (README rules). */
#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

#define UNUSED(x) ((void)(x))

/* MESSAGE STRUCT RULE (read this before adding a field to anything in protocol.h
 * or to JobSpec): we send these structs over the socket as raw bytes, so the
 * scheduler, worker and client must all see the same size and layout. The
 * compiler only adds padding when a small field is followed by a bigger one.
 * So in a message struct use ONLY uint32_t / int32_t fields and char arrays.
 * Do not add uint8_t, uint16_t or uint64_t fields without discussing it first. */

/* ---------------- constants ---------------- */
#define DEFAULT_DISCOVERY_PORT 9999   /* 2C: UDP discovery port */
#define DISCOVERY_TIMEOUT_SEC  2      /* 2C: wait per discovery attempt */
#define DISCOVERY_RETRIES      3      /* 2C: attempts before using the fallback address */
#define HEARTBEAT_INTERVAL_SEC 2      /* worker -> scheduler heartbeat period */
#define QUEUE_CAPACITY         1024   /* 4A: bounded queue; when full, submit => QUEUE_FULL */
#define AGING_INTERVAL_SEC     10     /* 4B: PRIORITY mode improves priority by 1 per 10 s queued */
#define OUTPUT_POLL_MS         500    /* 5B: worker tails output.log this often */

#define MAX_USERNAME 31               /* 3: usernames are 1..31 chars; buffers are +1 for NUL */
#define MAX_PATH_LEN 512
#define MAX_REASON   256

/* ---------------- state machines ----------------
 * All states exist from day one (spec Section 1, "State Machines": adding them
 * later means going back into frozen code). Milestone 1 only uses
 * QUEUED / RUNNING / COMPLETED / FAILED and WORKER_ALIVE. */
typedef enum{
    JOB_QUEUED, 
    JOB_RUNNING, 
    JOB_COMPLETED, 
    JOB_FAILED,
    JOB_CANCELLED,
    JOB_WAITING,
    JOB_LOST,
    JOB_FAILED_PERMANENT,
    JOB_STATE_COUNT
}JobState;

typedef enum{
    WORKER_ALIVE,
    WORKER_SUSPECTED,
    WORKER_DEAD,
    WORKER_DRAINING,
    WORKER_STATE_COUNT
}WorkerState;

const char *job_state_name(JobState s);       /* "QUEUED", ... */
const char *worker_state_name(WorkerState s); /* "ALIVE", ...  */

/* ---------------- wire-level job description (client -> scheduler) ---------------- */
typedef struct{
    uint32_t cores;
    uint32_t ram_mb;
    uint32_t time_sec;
    uint32_t gpus;   /* -cores -ram -time -gpu (gpu defaults to 0) */
    uint32_t prio;                            /* -prio: 1 (highest) .. 3 */
    char binary[MAX_PATH_LEN];            /* path or bare PATH command, NUL-terminated */
} JobSpec;

/* ---------------- scheduler-side records (never sent on the wire) ---------------- */
typedef struct Job{
    uint32_t id;
    char user[MAX_USERNAME + 1];          /* owner: receives this job's [Event] lines */
    JobSpec spec;
    JobState state;
    uint32_t worker_id;                       /* 0 = not assigned */
    int binary_is_path_command;          /* 1: run by name from PATH, nothing to transfer */
    time_t submit_time;                     /* FIFO order */
    time_t queued_since;                    /* PRIORITY aging: effective prio = prio - waited/10, min 1 */
    time_t start_time;
    uint32_t end_time;
    struct Job *next;                         /* linked list: B's queue */
}Job;

typedef struct Worker{
    uint32_t id;                              /* assigned by the scheduler on REGISTER */
    int fd;
    WorkerState state;
    uint32_t cores_total;
    uint32_t cores_free;
    uint32_t ram_total_mb;
    uint32_t ram_free_mb;
    uint32_t gpus_total;
    uint32_t gpus_free;
    uint32_t running_jobs;
    time_t last_heartbeat;
    char ip[46];                          /* big enough for an IPv6 string */
    struct Worker *next;                      /* linked list: B's worker table */
}Worker;

#endif
