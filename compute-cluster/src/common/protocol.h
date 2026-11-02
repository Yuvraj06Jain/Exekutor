/* CONTRACT FILE: the wire protocol.
 *
 * Framing: [MsgHeader][payload of header.length bytes]. All integers are in
 * HOST byte order (every lab machine is little-endian x86/ARM; never mix).
 * Structs are packed, fixed-width, NUL-terminated char arrays.
 * "(+bytes)" means the fixed struct is followed by raw bytes in the same payload.
 *
 * Order the scheduler sends when it starts a job on a worker:
 *   1. MSG_CHECKPOINT      (only if the job has a saved checkpoint)
 *   2. MSG_FILE_DATA or MSG_FILE_ALREADY_CACHED   (skipped for PATH commands)
 *   3. MSG_JOB_ASSIGN      (last; this is what makes the worker start the job)
 */
#ifndef COMMON_PROTOCOL_H
#define COMMON_PROTOCOL_H

#include "common/types.h"

typedef struct WIRE { uint32_t type; uint32_t length; } MsgHeader;
#define MAX_PAYLOAD (256u * 1024u * 1024u)

typedef enum {
    /* worker <-> scheduler */
    MSG_REGISTER = 1,       /* worker -> sched : RegisterMsg */
    MSG_REGISTER_ACK,       /* sched -> worker : RegisterAck */
    MSG_HEARTBEAT,          /* worker -> sched : HeartbeatMsg (+JobTelemetry[n]) */
    MSG_JOB_ASSIGN,         /* sched -> worker : JobAssignMsg */
    MSG_JOB_UPDATE,         /* worker -> sched : JobUpdateMsg (RUNNING/COMPLETED/FAILED + reason) */
    MSG_JOB_OUTPUT,         /* worker -> sched : JobOutputMsg (+bytes) */
    MSG_CHECKPOINT,         /* both ways       : CheckpointMsg (+bytes) */
    MSG_KILL,               /* sched -> worker : KillMsg */
    MSG_DRAIN,              /* worker -> sched : DrainMsg ("I am draining") */
    MSG_CACHE_REPORT,       /* worker -> sched : CacheReportMsg (+hashes) */
    MSG_FILE_DATA,          /* sched -> worker : FileDataMsg (+file bytes) */
    MSG_FILE_ALREADY_CACHED,/* sched -> worker : FileRefMsg */
    MSG_FILE_MISSING,       /* worker -> sched : FileRefMsg (cache file vanished) */
    /* client -> scheduler */
    MSG_CLIENT_LOGIN,       /* ClientLoginMsg (always the first message) */
    MSG_SUBMIT,             /* JobSpec; answered with MSG_REPLY */
    MSG_NODES_REQ,          /* no payload; answered with MSG_REPLY_TEXT */
    MSG_STATUS_REQ,         /* IdReq; answered with MSG_REPLY_TEXT */
    MSG_LOGS_REQ,           /* LogsReq; answered with MSG_LOGS_DATA... MSG_LOGS_END */
    MSG_CANCEL_REQ,         /* IdReq; answered with MSG_REPLY */
    MSG_MIGRATE_REQ,        /* IdReq; answered with MSG_REPLY */
    MSG_CRON_LIST_REQ,      /* no payload; answered with MSG_REPLY_TEXT */
    MSG_CANCEL_CRON_REQ,    /* IdReq (template id); answered with MSG_REPLY */
    /* scheduler -> client */
    MSG_REPLY,              /* ReplyMsg */
    MSG_REPLY_TEXT,         /* raw text bytes, client prints as is */
    MSG_EVENT,              /* raw text bytes: one "[Event] ..." line (no newline) */
    MSG_LOGS_DATA,          /* raw output bytes */
    MSG_LOGS_END,           /* IdReq (job id): follow finished */
    MSG_TYPE_COUNT
} MsgType;

/* ---------------- payloads ---------------- */
typedef struct WIRE { uint32_t cores, ram_mb, gpus; } RegisterMsg;
typedef struct WIRE { uint32_t worker_id; } RegisterAck;

typedef struct WIRE {
    uint32_t worker_id, cpu_load_pct, running_jobs, n_telemetry;
} HeartbeatMsg;                                   /* +JobTelemetry[n_telemetry] */

#define ASSIGN_PATH_COMMAND  1u   /* run by name found in PATH; no file transfer */
#define ASSIGN_HAS_CHECKPOINT 2u
typedef struct WIRE {
    uint32_t job_id, cores, ram_mb, time_sec, gpus, attempt, flags;
    char     hash[HASH_HEX_LEN + 1];
    char     name[MAX_PATH_LEN];                  /* file name or command name */
} JobAssignMsg;

typedef struct WIRE {
    uint32_t job_id, worker_id, new_state;        /* JobState */
    uint64_t peak_ram_kb;
    char     reason[MAX_REASON];                  /* human text; "" if none */
} JobUpdateMsg;

typedef struct WIRE { uint32_t job_id, len; } JobOutputMsg;       /* +bytes */
typedef struct WIRE { uint32_t job_id, number, len; } CheckpointMsg; /* +bytes */

#define KILL_CANCEL  1u   /* SIGKILL, report "cancelled" */
#define KILL_PREEMPT 2u   /* SIGKILL, report "preempted" */
#define KILL_MIGRATE 3u   /* SIGTERM, SIGKILL after MIGRATE_KILL_GRACE_SEC */
typedef struct WIRE { uint32_t job_id, reason; } KillMsg;

typedef struct WIRE { uint32_t worker_id; } DrainMsg;
typedef struct WIRE { uint32_t n; } CacheReportMsg;               /* +n*(HASH_HEX_LEN+1) */
typedef struct WIRE {
    uint32_t job_id; char hash[HASH_HEX_LEN + 1];
    char name[MAX_PATH_LEN]; uint64_t len;
} FileDataMsg;                                                    /* +file bytes */
typedef struct WIRE { uint32_t job_id; char hash[HASH_HEX_LEN + 1]; } FileRefMsg;

typedef struct WIRE { char username[MAX_USERNAME + 1]; } ClientLoginMsg;
typedef struct WIRE { uint32_t id; } IdReq;
typedef struct WIRE { uint32_t job_id, follow; } LogsReq;

#define REPLY_OK 0
#define REPLY_QUEUE_FULL 1
#define REPLY_ERROR 2
typedef struct WIRE { int32_t code; uint32_t id; char text[MAX_REASON]; } ReplyMsg;

#endif
