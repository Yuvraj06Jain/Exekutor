#include "common/types.h"

static const char *const job_names[JOB_STATE_COUNT] = {
    "QUEUED", "RUNNING", "COMPLETED", "FAILED", "CANCELLED",
    "WAITING", "LOST", "FAILED_PERMANENT"
};
static const char *const worker_names[WORKER_STATE_COUNT] = {
    "ALIVE", "SUSPECTED", "DEAD", "DRAINING"
};

const char *job_state_name(JobState s) {
    return (s >= 0 && s < JOB_STATE_COUNT) ? job_names[s] : "UNKNOWN";
}
const char *worker_state_name(WorkerState s) {
    return (s >= 0 && s < WORKER_STATE_COUNT) ? worker_names[s] : "UNKNOWN";
}
