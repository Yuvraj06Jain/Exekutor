/* OWNER: C. Section 8: dependencies and groups. */
#include "scheduler/hooks.h"
#include "scheduler/handlers.h"

int  deps_on_submit(Job *j, char *err, size_t errlen) {
    UNUSED(j); UNUSED(err); UNUSED(errlen);
    return 0;                       /* stub: accept everything, job has no deps */
}
void deps_on_finish(Job *j) { UNUSED(j); }
