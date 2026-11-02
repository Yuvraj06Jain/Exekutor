/* OWNER: C. Section 13: binary transfer, hash registry per worker. */
#include "scheduler/hooks.h"
#include "scheduler/handlers.h"

int cache_worker_has(const Worker *w, const char *hash) { UNUSED(w); UNUSED(hash); return 0; }
int send_binary(Job *j, Worker *w) { UNUSED(j); UNUSED(w); NOT_IMPLEMENTED("send_binary"); return 0; }
HANDLER(on_cache_report) { UNUSED(c); UNUSED(h); UNUSED(payload); NOT_IMPLEMENTED("on_cache_report"); }
HANDLER(on_file_missing) { UNUSED(c); UNUSED(h); UNUSED(payload); NOT_IMPLEMENTED("on_file_missing"); }
