/* OWNER: B. Section 4C: heartbeat bookkeeping, ALIVE/SUSPECTED/DEAD, rescue. */
#include "scheduler/hooks.h"
#include "scheduler/handlers.h"

HANDLER(on_heartbeat) { UNUSED(c); UNUSED(h); UNUSED(payload); NOT_IMPLEMENTED("on_heartbeat"); }
