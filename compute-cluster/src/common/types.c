#include "common/types.h"

char *job_names[JOB_STATE_COUNT] = {
    "QUEUED", "RUNNING", "COMPLETED", "FAILED", "CANCELLED",
    "WAITING", "LOST", "FAILED_PERMANENT"
};
char *worker_names[WORKER_STATE_COUNT] = {
    "ALIVE", "SUSPECTED", "DEAD", "DRAINING"
};


char *job_state_name(JobState s) {
    return (s >= 0 && s < JOB_STATE_COUNT) ? job_names[s] : "UNKNOWN";
}

char *worker_state_name(WorkerState s) {
    return (s >= 0 && s < WORKER_STATE_COUNT) ? worker_names[s] : "UNKNOWN";
}
