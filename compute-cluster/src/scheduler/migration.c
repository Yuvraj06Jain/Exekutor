/* OWNER: A. Section 14 (scheduler side): store latest checkpoint, migrate command. */
#include "scheduler/hooks.h"
#include "scheduler/handlers.h"

int send_checkpoint(Job *j, Worker *w) { UNUSED(j); UNUSED(w); return 0; }   /* stub: nothing to send */
HANDLER(on_checkpoint) { UNUSED(c); UNUSED(h); UNUSED(payload); NOT_IMPLEMENTED("on_checkpoint"); }
HANDLER(on_migrate_req) { UNUSED(c); UNUSED(h); UNUSED(payload); NOT_IMPLEMENTED("on_migrate_req"); }
