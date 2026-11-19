#ifndef WORKER_CAPACITY_H
#define WORKER_CAPACITY_H

#include "worker/config.h"

int       machine_cores(void);     // online CPU cores, or -1 if unknown
long long machine_ram_mb(void);    // physical RAM in MB, or -1 if unknown

// Lowers cfg->cores and cfg->ram_mb to the real machine values if they ask for
// more, and logs a warning for each one. -gpu is never touched.
void clamp_capacity(WorkerConfig *cfg);

#endif
