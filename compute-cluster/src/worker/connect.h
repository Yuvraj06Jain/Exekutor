#ifndef WORKER_CONNECT_H
#define WORKER_CONNECT_H

#include "worker/config.h"

// Finds the scheduler (UDP discovery first, command-line fallback second) and
// opens the TCP connection. Returns the socket, or -1 if the worker cannot
// reach a scheduler (the reason has already been logged).
int connect_to_scheduler(WorkerConfig *cfg);

#endif
