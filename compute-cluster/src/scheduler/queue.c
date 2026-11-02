/* OWNER: B. Sections 4A/4B (queue) and 3 (submit/status/cancel handling).
 * TODO(B): bounded queue (QUEUE_CAPACITY), QUEUE_FULL, job ids, cancel logic. */
#include "scheduler/hooks.h"
#include "scheduler/handlers.h"

Job *job_find(uint32_t id) { UNUSED(id); NOT_IMPLEMENTED("job_find"); return NULL; }
Job *job_create(const JobSpec *spec, const char *user) {
    UNUSED(spec); UNUSED(user); NOT_IMPLEMENTED("job_create"); return NULL;
}
int  queue_submit(Job *j) { UNUSED(j); NOT_IMPLEMENTED("queue_submit"); return -1; }
void job_cancel(Job *j, const char *requested_by) {
    UNUSED(j); UNUSED(requested_by); NOT_IMPLEMENTED("job_cancel");
}
HANDLER(on_submit) { UNUSED(c); UNUSED(h); UNUSED(payload); NOT_IMPLEMENTED("on_submit"); }
HANDLER(on_status_req) { UNUSED(c); UNUSED(h); UNUSED(payload); NOT_IMPLEMENTED("on_status_req"); }
HANDLER(on_cancel_req) { UNUSED(c); UNUSED(h); UNUSED(payload); NOT_IMPLEMENTED("on_cancel_req"); }
