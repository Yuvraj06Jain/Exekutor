/* OWNER: C. Section 13 (worker side): cache/ folder, LRU eviction, restart scan.
 * STUBS: cache_prepare lets PATH commands work so A can test the sandbox now. */
#include <stdio.h>
#include "worker/worker.h"

int cache_prepare(const char *name, const char *hash, int is_path_command,
                  char *out_path, size_t out_len) {
    UNUSED(hash);
    if (!is_path_command) { NOT_IMPLEMENTED("cache_prepare (file binaries)"); return -1; }
    snprintf(out_path, out_len, "%s", name);
    return 0;
}
int cache_store(const char *hash, const char *name, const void *data, size_t len) {
    UNUSED(hash); UNUSED(name); UNUSED(data); UNUSED(len);
    NOT_IMPLEMENTED("cache_store");
    return -1;
}
void cache_report(int sock) { UNUSED(sock); }
