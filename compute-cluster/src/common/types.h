/* types.h
 *
 * The basic vocabulary shared by all three programs (scheduler, worker, client):
 *   - the tuning constants from the spec (ports, timeouts, queue size, ...)
 *   - the job and worker state machines (spec Section 1)
 *   - JobSpec, the job description the client sends when you type `submit`
 *   - Job and Worker, the records the scheduler keeps in memory
 *
 * This is a CONTRACT FILE: all three programs are compiled against it, so a
 * change here changes everyone's code. Rules for changing it:
 *   - only ever ADD things (new constants, new enum values before the *_COUNT
 *     entry, new struct fields at the END of a struct)
 *   - never rename or reorder what is already here
 *   - tell the rest of the team before pushing a change
 */
#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <stdint.h>     /* uint32_t, int32_t: integers with a fixed size on every machine */
#include <stddef.h>     /* size_t */
#include <time.h>       /* time_t, used for the timestamps in Job and Worker */

/* WHY ONLY uint32_t / int32_t AND char ARRAYS IN MESSAGE STRUCTS
 * Structs like JobSpec (and everything in protocol.h) are sent over TCP as raw
 * bytes: the sender does write(&s, sizeof s) and the receiver reads the same
 * number of bytes straight into its own copy of the struct. That only works if
 * all three programs agree on the exact size and layout of the struct.
 * The compiler inserts invisible "padding" bytes when a small field is followed
 * by a bigger one (e.g. a uint8_t then a uint32_t). Using only 4-byte integers
 * and char arrays avoids padding, so the layout is the same everywhere.
 * Don't add uint8_t / uint16_t / uint64_t fields without talking about it first. */

/* ---------------- constants (values come straight from the spec) ---------------- */
#define DEFAULT_DISCOVERY_PORT 9999   /* 2C: UDP port used for discovery when -discovery-port isn't given */
#define DISCOVERY_TIMEOUT      2      /* 2C: seconds to wait for a reply on each discovery attempt */
#define DISCOVERY_RETRIES      3      /* 2C: discovery attempts before falling back to the command-line address */
#define HEARTBEAT_INTERVAL     2      /* 4C: worker sends a heartbeat every 2 seconds */
#define JOB_QUEUE_CAPACITY     1024   /* 4A: max jobs in the queue; when full, submit answers QUEUE_FULL */
#define AGING_INTERVAL         10     /* 4B: in PRIORITY mode a job's priority improves by 1 for every 10 s it waits */
#define JOB_OUTPUT_POLL         500    /* 5B: worker reads new output from output.log every 500 ms */

#define MAX_USERNAME 31               /* 3: usernames are 1..31 chars; buffers are MAX_USERNAME + 1 to fit the '\0' */
#define MAX_PATH_LEN 512              /* longest binary path / command name we accept */
#define MAX_REASON   256              /* longest human-readable text (failure reason, reply text) */

/* ---------------- state machines ----------------
 * Every state of the full project is listed from day one, even the ones only
 * later milestones use. The spec (Section 1, "State Machines") warns that
 * adding states later means reopening code that is already finished.
 * Milestone 1 only uses QUEUED / RUNNING / COMPLETED / FAILED and ALIVE.
 *
 * The *_COUNT entry at the end is not a real state: since enum values count
 * up from 0, it equals the number of states. types.c uses it to size the
 * arrays of state names, so new states must go BEFORE it. */
typedef enum{
    QUEUED,             /* waiting in the queue for a worker with enough free resources */
    RUNNING,            /* running on a worker right now */
    COMPLETED,          /* finished with exit code 0 */
    FAILED,             /* finished badly: non-zero exit, killed by a signal, timeout, ... */
    CANCELLED,          /* cancelled by the user, or because a job it depended on failed (Sec 8) */
    WAITING,            /* blocked until the jobs it depends on complete (Sec 8) */
    LOST,               /* was RUNNING on a worker that died; short-lived, goes back to QUEUED or to FAILED_PERMANENT (Sec 4C) */
    FAILED_PERMANENT,   /* used up all its retry attempts after repeated worker deaths (Sec 4C) */
    JOB_STATE_COUNT
}JobState;

typedef enum{
    ALIVE,              /* heartbeats arriving normally; the only state that can receive new jobs */
    SUSPECTED,          /* missed 1 or 2 heartbeats in a row; gets no new jobs, back to ALIVE if a heartbeat arrives */
    DEAD,               /* missed 3 heartbeats in a row (~6 s); its running jobs are rescued (Sec 4C) */
    DRAINING,           /* finishing its current jobs before leaving; gets no new jobs (Sec 9) */
    WORKER_STATE_COUNT
}WorkerState;

/* Turn a state into its printable name, e.g. job_state_name(RUNNING) -> "RUNNING".
 * An out-of-range value gives "UNKNOWN" instead of crashing. Defined in types.c. */
char *job_state_name(JobState s);
char *worker_state_name(WorkerState s);

/* ---------------- JobSpec: what the client sends for `submit` ----------------
 * One field per submit flag. The client fills it from the command line and
 * sends it as the payload of MSG_SUBMIT (see protocol.h). */
typedef struct{
    uint32_t cores;                 /* -cores: CPU cores the job counts as using (bookkeeping only) */
    uint32_t ram_mb;                /* -ram: MB counted as used, and also the memory limit the worker enforces */
    uint32_t time_sec;              /* -time: wall-clock limit in seconds; the job is killed after this */
    uint32_t gpus;                  /* -gpu: GPUs the job needs (0 if the flag isn't given) */
    uint32_t prio;                  /* -prio: 1 (most important) .. 3 (least important) */
    char binary[MAX_PATH_LEN];      /* path to the executable, or a bare command name found in PATH (like "ls"); '\0'-terminated */
} JobSpec;

/* ---------------- Job: the scheduler's record of one job ----------------
 * Lives only inside the scheduler and is never sent over the network, so the
 * "only uint32_t" rule above doesn't apply (time_t, int and pointers are fine). */
typedef struct Job{
    uint32_t id;                        /* job id handed back to the client after submit */
    char user[MAX_USERNAME + 1];        /* who submitted it; only this user receives the job's [Event] lines */
    JobSpec spec;                       /* the resources and command exactly as submitted */
    JobState state;                     /* where the job is in its state machine right now */
    uint32_t worker_id;                 /* worker running it; 0 means "not assigned to any worker" */
    int binary_is_path_command;         /* 1 = a command found in PATH (e.g. "ls"): the worker runs it by name, no file is sent */
    time_t submit_time;                 /* when it was submitted; FIFO order and "older first" tie-breaks use this */
    time_t queued_since;                /* when it (last) entered the queue; PRIORITY aging uses this:
                                           effective prio = prio - (seconds waited / AGING_INTERVAL), never below 1 */
    time_t start_time;                  /* when it started RUNNING */
    time_t end_time;                    /* when it reached a final state */
    struct Job *next;                   /* next job in the scheduler's linked list */
}Job;

/* ---------------- Worker: the scheduler's record of one connected worker ----------------
 * Also lives only inside the scheduler. The *_free fields go down when a job
 * is placed on the worker and back up when that job ends. */
typedef struct Worker{
    uint32_t id;                        /* unique id the scheduler gives the worker when it registers */
    int fd;                             /* the TCP socket connected to this worker */
    WorkerState state;
    uint32_t cores_total;               /* capacity the worker advertised when registering */
    uint32_t cores_free;                /* capacity not yet taken by jobs running on it */
    uint32_t ram_total_mb;
    uint32_t ram_free_mb;
    uint32_t gpus_total;
    uint32_t gpus_free;
    uint32_t running_jobs;              /* number of jobs running on it right now */
    time_t last_heartbeat;              /* when its last heartbeat arrived; used to spot missed heartbeats */
    char ip[46];                        /* worker's address as text; 46 bytes fits even an IPv6 address */
    struct Worker *next;                /* next worker in the scheduler's worker list */
}Worker;

#endif
