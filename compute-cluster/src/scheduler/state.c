/* OWNER: B. Job state transitions + handlers for worker reports. */
#include "scheduler/hooks.h"
#include "scheduler/handlers.h"

#include <stdio.h>

/* Minimal working version so everyone can use it from day one.
 * TODO(B): also emit_event() to the job owner, update timestamps, call
 *          deps_on_finish() on final states. */
void job_set_state(Job *j, JobState s, const char *reason) {
    JobState old = j->state;
    j->state = s;
    if (reason && reason[0])
        log_line("SCHEDULER", "Job %u %s -> %s (%s)", j->id, job_state_name(old), job_state_name(s), reason);
    else
        log_line("SCHEDULER", "Job %u %s -> %s", j->id, job_state_name(old), job_state_name(s));
}
HANDLER(on_job_update) { UNUSED(c); UNUSED(h); UNUSED(payload); NOT_IMPLEMENTED("on_job_update"); }
HANDLER(on_job_output) { UNUSED(c); UNUSED(h); UNUSED(payload); NOT_IMPLEMENTED("on_job_output"); }
