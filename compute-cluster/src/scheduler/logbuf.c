/* OWNER: C. Section 6: 64 KB per-job output buffer, log following. */
#include "scheduler/hooks.h"
#include "scheduler/handlers.h"

void logbuf_append(Job *j, const char *data, size_t len) {
    UNUSED(j); UNUSED(data); UNUSED(len);
}
HANDLER(on_logs_req) { UNUSED(c); UNUSED(h); UNUSED(payload); NOT_IMPLEMENTED("on_logs_req"); }
