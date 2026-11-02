/* CONTRACT FILE: the wire protocol, Milestone 1 subset.
 *
 * Framing: [MsgHeader][payload of header.length bytes]. All integers are in
 * HOST byte order (every lab machine is little-endian x86/ARM; never mix in htonl).
 * Structs use fixed-width types and NUL-terminated char arrays, and follow the
 * "message struct rule" in types.h (only uint32_t/int32_t and char arrays, so no padding).
 * "(+bytes)" means the fixed struct is followed by raw bytes in the same payload.
 *
 * Starting a job in Milestone 1: the scheduler sends ONE message, MSG_JOB_ASSIGN,
 * for a PATH command (e.g. "ls"). No checkpoint and no file transfer yet.
 *
 * Message numbers are KEPT IDENTICAL to the full contract, so the gaps below are
 * the messages of later milestones. When you add one, give it the number that is
 * reserved for it here (never reuse a number for something else). */
#ifndef COMMON_PROTOCOL_H
#define COMMON_PROTOCOL_H

#include "common/types.h"

typedef struct { uint32_t type; uint32_t length; } MsgHeader;
#define MAX_PAYLOAD (256u * 1024u * 1024u)    /* recv_msg rejects anything bigger */

typedef enum {
    /* worker <-> scheduler */
    MSG_REGISTER     = 1,    /* worker -> sched : RegisterMsg */
    MSG_REGISTER_ACK = 2,    /* sched -> worker : RegisterAck (carries the worker id) */
    MSG_HEARTBEAT    = 3,    /* worker -> sched : HeartbeatMsg */
    MSG_JOB_ASSIGN   = 4,    /* sched -> worker : JobAssignMsg (this starts the job) */
    MSG_JOB_UPDATE   = 5,    /* worker -> sched : JobUpdateMsg (RUNNING/COMPLETED/FAILED + reason) */
    MSG_JOB_OUTPUT   = 6,    /* worker -> sched : JobOutputMsg (+bytes) */
    /* 7..13 reserved: CHECKPOINT, KILL, DRAIN, CACHE_REPORT, FILE_DATA,
     *                 FILE_ALREADY_CACHED, FILE_MISSING */

    /* client -> scheduler */
    MSG_CLIENT_LOGIN = 14,   /* ClientLoginMsg (always the first message) */
    MSG_SUBMIT       = 15,   /* JobSpec; answered with MSG_REPLY */
    MSG_NODES_REQ    = 16,   /* no payload; answered with MSG_REPLY_TEXT */
    MSG_STATUS_REQ   = 17,   /* IdReq; answered with MSG_REPLY_TEXT */
    /* 18..22 reserved: LOGS_REQ, CANCEL_REQ, MIGRATE_REQ, CRON_LIST_REQ, CANCEL_CRON_REQ */

    /* scheduler -> client */
    MSG_REPLY        = 23,   /* ReplyMsg */
    MSG_REPLY_TEXT   = 24,   /* raw text bytes, client prints as is */
    MSG_EVENT        = 25,   /* raw text bytes: one "[Event] ..." line (no newline) */
    /* 26..27 reserved: LOGS_DATA, LOGS_END */

    MSG_TYPE_COUNT   = 28
} MsgType;

/* ---------------- payloads ---------------- */
typedef struct{
    uint32_t cores; 
    uint32_t ram_mb;
    uint32_t  gpus;
}RegisterMsg;


typedef struct{
    uint32_t worker_id; 
}RegisterAck;

/* n_telemetry is always 0 in Milestone 1 (live telemetry arrives in Milestone 2). */
typedef struct{
    uint32_t worker_id;
    uint32_t cpu_load_pct;
    uint32_t running_jobs;
    uint32_t n_telemetry;
}HeartbeatMsg;

#define ASSIGN_PATH_COMMAND 1u   /* run by name found in PATH; no file transfer */
typedef struct{
    uint32_t job_id;
    uint32_t cores;
    uint32_t ram_mb;
    uint32_t time_sec;
    uint32_t gpus;
    uint32_t flags;
    char name[MAX_PATH_LEN];                 /* command name (PATH command) */
}JobAssignMsg;

typedef struct{
    uint32_t job_id;
    uint32_t worker_id;
    uint32_t new_state;       /* new_state is a JobState */
    char reason[MAX_REASON];                 /* human text; "" if none */
}JobUpdateMsg;

typedef struct{
    uint32_t job_id;
    uint32_t len; 
}JobOutputMsg;   /* +len bytes of output */

typedef struct{
    char username[MAX_USERNAME + 1]; 
}ClientLoginMsg;

typedef struct{
    uint32_t id; 
}IdReq;                   /* status <job_id> */

#define REPLY_OK         0
#define REPLY_QUEUE_FULL 1
#define REPLY_ERROR      2

typedef struct{
    int32_t code; 
    uint32_t id; 
    char text[MAX_REASON]; 
}ReplyMsg;

#endif
