/* types.c
 *
 * Turns state enums into printable names, for log lines and the
 * `status` / `nodes` output.
 */
#include "common/types.h"

/* Names in the SAME order as the enums in types.h: an enum value is just its
 * position (QUEUED = 0, RUNNING = 1, ...), so job_names[RUNNING] is "RUNNING".
 * If you add a state to an enum, add its name here at the same position. */
char *job_names[JOB_STATE_COUNT] = {
    "QUEUED", "RUNNING", "COMPLETED", "FAILED", "CANCELLED",
    "WAITING", "LOST", "FAILED_PERMANENT"
};
char *worker_names[WORKER_STATE_COUNT] = {
    "ALIVE", "SUSPECTED", "DEAD", "DRAINING"
};


/* Range check first, so a garbage value (e.g. from a corrupted message) prints
 * "UNKNOWN" instead of reading past the end of the array. */
char *job_state_name(JobState s) {
    return (s >= 0 && s < JOB_STATE_COUNT) ? job_names[s] : "UNKNOWN";
}

char *worker_state_name(WorkerState s) {
    return (s >= 0 && s < WORKER_STATE_COUNT) ? worker_names[s] : "UNKNOWN";
}
