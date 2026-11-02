/* OWNER: A. Section 12: cron templates (-every), crons, cancel-cron. */
#include "scheduler/hooks.h"
#include "scheduler/handlers.h"
#include <stdio.h>

void cron_tick(void) { }
int cron_add_template(const JobSpec *spec, const char *user,
                      uint32_t *template_id, char *err, size_t errlen) {
    UNUSED(spec); UNUSED(user); UNUSED(template_id);
    snprintf(err, errlen, "cron not implemented yet");
    return -1;
}
HANDLER(on_cron_list_req) { UNUSED(c); UNUSED(h); UNUSED(payload); NOT_IMPLEMENTED("on_cron_list_req"); }
HANDLER(on_cancel_cron_req) { UNUSED(c); UNUSED(h); UNUSED(payload); NOT_IMPLEMENTED("on_cancel_cron_req"); }
