#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <time.h>

#include "common/log.h"
#include "common/net.h"
#include "common/protocol.h"
#include "common/types.h"
#include "worker/job.h"
#include "worker/loop.h"

#define LOOP_TICK_MS 100 // the loop wakes up this often even when nothing arrives

long long now_ms(void){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static int send_heartbeat(int sock, uint32_t worker_id){
    HeartbeatMsg hb;
    hb.worker_id = worker_id;
    hb.cpu_load_pct = 0; // placeholder: real CPU load comes in Section 4D (later)
    hb.running_jobs = jobs_running();
    return send_msg(sock, MSG_HEARTBEAT, &hb, sizeof hb);
}

// Returns -1 only if the scheduler connection broke while replying.
static int handle_message(int sock, uint32_t worker_id, char *tag, MsgHeader *h, void *payload){
    if(h->type == MSG_JOB_ASSIGN && h->length == sizeof(JobAssignMsg))
        return job_start(sock, worker_id, tag, payload);

    log_line(tag, "Ignoring message type %u (length %u)", h->type, h->length);
    return 0;
}

// ASSUMPTION: one poll() loop instead of threads, so only one place ever writes to
// the socket and no mutex is needed. Later sections add their periodic work
// (output tailing, wall-clock timers, reaping finished jobs) to each tick.
void run_loop(int sock, uint32_t worker_id, char *tag){
    log_line(tag, "Heartbeat active. Waiting for jobs...");

    struct pollfd pfd = { sock, POLLIN, 0 };
    long long next_heartbeat = now_ms(); // the first heartbeat goes out immediately

    for(;;){
        int n = poll(&pfd, 1, LOOP_TICK_MS);
        if(n < 0){
            if(errno == EINTR)
                continue;
            log_line(tag, "Error: poll failed.");
            return;
        }

        if(n > 0){
            MsgHeader h;
            void *payload = NULL;
            if(recv_msg(sock, &h, &payload) < 0){
                log_line(tag, "Lost connection to the scheduler.");
                return;
            }
            int rc = handle_message(sock, worker_id, tag, &h, payload);
            free(payload);
            if(rc < 0){
                log_line(tag, "Lost connection to the scheduler.");
                return;
            }
        }

        if(jobs_reap(sock, worker_id, tag) < 0){ // report jobs that finished since the last tick
            log_line(tag, "Lost connection to the scheduler.");
            return;
        }

        if(now_ms() >= next_heartbeat){
            if(send_heartbeat(sock, worker_id) < 0){
                log_line(tag, "Lost connection to the scheduler.");
                return;
            }
            next_heartbeat += HEARTBEAT_INTERVAL * 1000;
        }
    }
}
