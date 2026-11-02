/* CONTRACT FILE. Changes must be additive (add, never rename/remove) and
 * announced in the group chat. See README.md "Contract rules". */
#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

#define UNUSED(x) ((void)(x))
#define WIRE __attribute__((packed))

/* ---------------- constants (from the brief) ---------------- */
#define DEFAULT_DISCOVERY_PORT 9999
#define DISCOVERY_TIMEOUT_SEC 2
#define DISCOVERY_RETRIES 3
#define HEARTBEAT_INTERVAL_SEC 2
#define SUSPECT_AFTER_MISSED 1
#define DEAD_AFTER_MISSED 3
#define MAX_ATTEMPTS 3
#define QUEUE_CAPACITY 1024
#define LOGBUF_BYTES (64 * 1024)
#define AGING_INTERVAL_SEC 10
#define OUTPUT_POLL_MS 500
#define MIGRATE_KILL_GRACE_SEC 2
#define DEFAULT_CACHE_SIZE_MB 512

#define MAX_USERNAME 31
#define MAX_PATH_LEN 512
#define MAX_REASON 256
#define MAX_DEPS 32
#define HASH_HEX_LEN 64
#define MAX_CACHED_HASHES 256

#define ENV_CACHE_DIR "CACHE_DIR"
#define ENV_CACHE_SIZE_MB "CACHE_SIZE"
#define ENV_CHECKPOINT_DIR "CHECKPOINT_DIR"


// States of a Job
typedef enum {
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

// States of a Worker
typedef enum {
    WORKER_ALIVE, 
    WORKER_SUSPECTED, 
    WORKER_DEAD, 
    WORKER_DRAINING,
    WORKER_STATE_COUNT
}WorkerState;


char *job_state(JobState s);
char *worker_state(WorkerState s);


/* ---------------- wire-level job description (client -> scheduler) ---------------- */
typedef struct WIRE{
    uint8_t is_group;
    uint32_t id;
}Dep;

typedef struct WIRE {
    uint32_t cores, ram_mb, time_sec, gpus;
    uint32_t prio;          /* 1 (highest) .. 3 */
    uint32_t group;         /* -group <id>, 0 = none */
    uint32_t every_sec;     /* -every <sec>, 0 = not a cron template */
    uint8_t  preemptible;   /* -preemptible */
    uint8_t  n_deps;
    Dep      deps[MAX_DEPS];
    char     binary[MAX_PATH_LEN];  /* path or bare PATH command, NUL-terminated */
} JobSpec;

/* Live telemetry for one running job (worker -> scheduler, in heartbeats) */
typedef struct WIRE {
    uint32_t job_id;
    uint64_t cur_ram_kb, peak_ram_kb, page_faults, cpu_time_ms;
} JobTelemetry;

/* ---------------- scheduler-side records ---------------- */
typedef struct Checkpoint {
    uint32_t number;      /* 0 = none yet */
    uint32_t size;
    time_t   saved_at;
    void    *data;        /* malloc'd, small (a few KB) */
} Checkpoint;

typedef struct Job {
    uint32_t id;
    char     user[MAX_USERNAME + 1];
    JobSpec  spec;
    JobState state;
    uint32_t attempts;             /* counts toward MAX_ATTEMPTS (not preempt/migrate) */
    uint32_t worker_id;            /* 0 = none */
    uint32_t excluded_worker_id;   /* migrate: avoid this worker if another qualifies */
    uint32_t held_for_job;         /* preempted for this priority-1 job (0 = none) */
    uint32_t template_id;          /* cron template that spawned it (0 = none) */
    char     hash[HASH_HEX_LEN + 1];
    int      binary_is_path_command;
    Checkpoint   checkpoint;
    JobTelemetry tel;
    time_t   submit_time, queued_since, start_time, end_time;
    struct Job *next;
} Job;

typedef struct Worker {
    uint32_t id;
    int      fd;
    WorkerState state;
    uint32_t cores_total, cores_free;
    uint32_t ram_total_mb, ram_free_mb;
    uint32_t gpus_total, gpus_free;
    uint32_t cpu_load_pct;         /* 0..100, from heartbeat */
    uint32_t running_jobs;
    uint32_t missed_heartbeats;
    time_t   last_heartbeat;
    char     ip[46];
    char     cached[MAX_CACHED_HASHES][HASH_HEX_LEN + 1];
    uint32_t n_cached;
    struct Worker *next;
} Worker;

#endif
