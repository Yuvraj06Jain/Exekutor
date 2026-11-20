#ifndef WORKER_LOOP_H
#define WORKER_LOOP_H

#include <stdint.h>

// Monotonic clock in milliseconds (not affected by changes to the system time).
long long now_ms(void);

// Heartbeats, incoming messages and (later) job handling. Returns when the
// scheduler connection is lost; the reason has already been logged.
void run_loop(int sock, uint32_t worker_id, char *tag);

#endif
