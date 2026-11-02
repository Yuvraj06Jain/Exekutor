/* OWNER: C. Section 11: fair-share quota check. */
#include "scheduler/hooks.h"
#include "scheduler/handlers.h"

int quota_should_skip(const Job *j) { UNUSED(j); return 0; }   /* stub: never skip */
