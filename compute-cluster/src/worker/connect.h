#ifndef WORKER_CONNECT_H
#define WORKER_CONNECT_H

#include <stdint.h>

#include "worker/config.h"

// Finds the scheduler (UDP discovery first, command-line fallback second) and
// opens the TCP connection. Returns the socket, or -1 if the worker cannot
// reach a scheduler (the reason has already been logged).
int connect_to_scheduler(WorkerConfig *cfg);

// Sends MSG_REGISTER with the (already clamped) capacity and waits for the
// scheduler's MSG_REGISTER_ACK. On success *worker_id holds the ID the scheduler
// assigned. Returns 0 on success, -1 on failure (reason already logged).
int register_with_scheduler(int sock, WorkerConfig *cfg, uint32_t *worker_id);

#endif
