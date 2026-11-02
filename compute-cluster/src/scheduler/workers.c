/* OWNER: B. Worker table, registration, nodes listing, draining state. */
#include "scheduler/hooks.h"
#include "scheduler/handlers.h"

Worker *worker_find(uint32_t id) { UNUSED(id); NOT_IMPLEMENTED("worker_find"); return NULL; }
HANDLER(on_register) { UNUSED(c); UNUSED(h); UNUSED(payload); NOT_IMPLEMENTED("on_register"); }
HANDLER(on_worker_drain) { UNUSED(c); UNUSED(h); UNUSED(payload); NOT_IMPLEMENTED("on_worker_drain"); }
HANDLER(on_nodes_req) { UNUSED(c); UNUSED(h); UNUSED(payload); NOT_IMPLEMENTED("on_nodes_req"); }
