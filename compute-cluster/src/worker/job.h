#ifndef WORKER_JOB_H
#define WORKER_JOB_H

#include <stdint.h>

#include "common/protocol.h"

// All three return 0 normally and -1 only if the scheduler connection broke
// while reporting (the caller then treats it as a lost connection).

// Starts the assigned job in a child process and reports RUNNING.
int job_start(int sock, uint32_t worker_id, char *tag, JobAssignMsg *a);

// Called every loop tick: reaps finished children and reports COMPLETED or FAILED.
int jobs_reap(int sock, uint32_t worker_id, char *tag);

// Number of jobs currently running (sent in every heartbeat).
uint32_t jobs_running(void);

#endif
