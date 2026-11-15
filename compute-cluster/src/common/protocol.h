/* protocol.h
 *
 * The wire protocol: every message that travels over TCP between the
 * scheduler and a worker, or between the scheduler and a client.
 * (UDP discovery has its own tiny packet, defined in discovery.c.)
 *
 * HOW A MESSAGE LOOKS ON THE SOCKET
 *
 *      +-----------+-------------+------------------------+
 *      |  type     |  length     |  payload               |
 *      |  4 bytes  |  4 bytes    |  `length` bytes        |
 *      +-----------+-------------+------------------------+
 *       \______ MsgHeader ______/
 *
 * TCP is a byte stream with no message boundaries, so the receiver first reads
 * exactly 8 bytes (the header), learns what kind of message it is and how many
 * bytes follow, then reads exactly that many. send_msg()/recv_msg() in net.c
 * do this. The payload is usually one of the structs below, copied as raw
 * bytes. Some messages carry plain text, or a struct followed by extra bytes
 * (JobOutputMsg is followed by the output itself).
 *
 * Numbers are sent in the machine's own byte order (no htonl). That's fine
 * because every machine we use is little-endian (x86 or ARM).
 *
 * The numbers in MsgType are fixed. The gaps are reserved for messages of
 * later milestones; when you add one, use its reserved number and never reuse
 * a number for something else.
 */
#ifndef COMMON_PROTOCOL_H
#define COMMON_PROTOCOL_H

#include "common/types.h"

/* The 8 bytes in front of every message. */
typedef struct{
    uint32_t type;      /* which message this is: a MsgType value */
    uint32_t length;    /* how many payload bytes follow (0 = no payload) */
}MsgHeader;

/* recv_msg() refuses any message claiming a bigger payload than this (256 MB).
 * Otherwise a corrupted header could make it try to malloc gigabytes. */
#define MAX_PAYLOAD (256u * 1024u * 1024u)

/* Every message type. The arrow says who sends it to whom, then the payload. */
typedef enum{
    /* ---- worker <-> scheduler ---- */
    MSG_REGISTER = 1,       /* worker -> sched : RegisterMsg. First message a worker sends: "here is my capacity" */
    MSG_REGISTER_ACK = 2,   /* sched -> worker : RegisterAck. "You're registered, your id is X" */
    MSG_HEARTBEAT = 3,      /* worker -> sched : HeartbeatMsg. Every HEARTBEAT_INTERVAL seconds: "I'm still alive" */
    MSG_JOB_ASSIGN = 4,     /* sched -> worker : JobAssignMsg. "Run this job" */
    MSG_JOB_UPDATE = 5,     /* worker -> sched : JobUpdateMsg. "Job X is now RUNNING / COMPLETED / FAILED, because ..." */
    MSG_JOB_OUTPUT = 6,     /* worker -> sched : JobOutputMsg followed by `len` bytes of the job's output */

    /* 7..13 reserved for later milestones (checkpoints, kill, drain, caching, file transfer) */

    /* ---- client -> scheduler ---- */
    MSG_CLIENT_LOGIN = 14,  /* ClientLoginMsg. Always the client's first message, and resent after every reconnect */
    MSG_SUBMIT = 15,        /* JobSpec. Answered with MSG_REPLY (code + the new job id) */
    MSG_NODES_REQ = 16,     /* no payload. Answered with MSG_REPLY_TEXT holding the ready-made `nodes` table */
    MSG_STATUS_REQ = 17,    /* IdReq (the job id). Answered with MSG_REPLY_TEXT holding the status text */

    /* 18..22 reserved for later milestones (logs, cancel, migrate, crons, cancel-cron) */

    /* ---- scheduler -> client ---- */
    MSG_REPLY = 23,         /* ReplyMsg: a result code plus an id / short text (e.g. the answer to submit) */
    MSG_REPLY_TEXT = 24,    /* plain text, the client prints it exactly as received (answers to nodes / status) */
    MSG_EVENT = 25,         /* plain text: one "[Event] ..." line about one of this user's jobs. Can arrive at
                               any moment, even while the user sits idle at the prompt */

    /* 26..27 reserved for later milestones (log streaming) */

    MSG_TYPE_COUNT = 28     /* not a message: one past the highest number, handy for range checks */
} MsgType;


/* ---------------- payload structs ---------------- */

/* MSG_REGISTER: the capacity the worker offers (from -cores / -ram / -gpu,
 * with cores and ram already clamped to what the machine really has). */
typedef struct{
    uint32_t cores;
    uint32_t ram_mb;
    uint32_t  gpus;
}RegisterMsg;


/* MSG_REGISTER_ACK: the id the scheduler gave this worker. The worker uses it
 * in all its log lines from then on ("[WORKER 1001] ..."). */
typedef struct{
    uint32_t worker_id;
}RegisterAck;


/* MSG_HEARTBEAT: "still alive", plus the numbers the scheduler needs for
 * load-aware placement (Sec 4D). */
typedef struct{
    uint32_t worker_id;
    uint32_t cpu_load_pct;      /* how busy the machine's CPU is, 0..100 */
    uint32_t running_jobs;      /* how many jobs the worker is running right now */
}HeartbeatMsg;


/* MSG_JOB_ASSIGN: everything the worker needs to start a job.
 * `flags` is a set of bits; ASSIGN_PATH_COMMAND means `name` is a command to
 * look up in PATH (like "ls"), so no executable file is being sent. */
#define ASSIGN_PATH_COMMAND 1u
typedef struct{
    uint32_t job_id;
    uint32_t cores;
    uint32_t ram_mb;            /* the worker turns this into the job's memory limit */
    uint32_t time_sec;          /* the worker kills the job after this many real seconds */
    uint32_t gpus;
    uint32_t flags;             /* ASSIGN_PATH_COMMAND or 0 */
    char name[MAX_PATH_LEN];    /* what to run ('\0'-terminated) */
}JobAssignMsg;


/* MSG_JOB_UPDATE: the worker reports that a job changed state. */
typedef struct{
    uint32_t job_id;
    uint32_t worker_id;
    uint32_t new_state;         /* a JobState value (stored as uint32_t to keep the struct layout fixed) */
    char reason[MAX_REASON];    /* human-readable why, e.g. "Wall-clock timeout exceeded (3s limit)"; "" if none */
}JobUpdateMsg;


/* MSG_JOB_OUTPUT: a chunk of what the job printed. This struct is immediately
 * followed, inside the same payload, by `len` bytes of output text. */
typedef struct{
    uint32_t job_id;
    uint32_t len;
}JobOutputMsg;


/* MSG_CLIENT_LOGIN: the username (1..31 chars, '\0'-terminated). */
typedef struct{
    char username[MAX_USERNAME + 1];
}ClientLoginMsg;


/* A request that only needs one id, e.g. MSG_STATUS_REQ for `status <job_id>`. */
typedef struct{
    uint32_t id;
}IdReq;


/* Possible values of ReplyMsg.code */
#define REPLY_OK         0      /* it worked */
#define REPLY_QUEUE_FULL 1      /* submit refused: the queue already holds JOB_QUEUE_CAPACITY jobs */
#define REPLY_ERROR      2      /* any other failure; the explanation is in `text` */


/* MSG_REPLY: the scheduler's answer to a request such as submit. */
typedef struct{
    int32_t code;               /* one of the REPLY_* values above */
    uint32_t id;                /* e.g. the new job id after a successful submit */
    char text[MAX_REASON];      /* optional message for the user */
}ReplyMsg;


#endif
