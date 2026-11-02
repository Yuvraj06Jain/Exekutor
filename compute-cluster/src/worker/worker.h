/* CONTRACT FILE (worker side). */
#ifndef WORKER_WORKER_H
#define WORKER_WORKER_H
#include <stddef.h>
#include "common/types.h"
#include "common/protocol.h"
#include "common/log.h"
#include "common/net.h"

/* ---- C (cache.c) : Section 13. A's worker loop calls these. ---- */

/* Called before a job starts. Fills out_path with the file to execvp:
 * the cached copy (cache/ folder) or, for PATH commands, just the name.
 * Returns 0 ok, -1 if the file is missing (then send MSG_FILE_MISSING). */
int  cache_prepare(const char *name, const char *hash, int is_path_command,
                   char *out_path, size_t out_len);

/* On MSG_FILE_DATA: store the bytes in the cache (and evict if over the limit). 0 ok. */
int  cache_store(const char *hash, const char *name, const void *data, size_t len);

/* Tell the scheduler which hashes this worker has (on connect, and after evictions). */
void cache_report(int sock);

#endif
