/* OWNER: B. Add a row here when you add a message type. */
#include "scheduler/hooks.h"
#include "scheduler/handlers.h"

static const struct { uint32_t type; MsgHandler fn; } table[] = {
    { MSG_REGISTER,       on_register },
    { MSG_HEARTBEAT,      on_heartbeat },
    { MSG_JOB_UPDATE,     on_job_update },
    { MSG_JOB_OUTPUT,     on_job_output },
    { MSG_CHECKPOINT,     on_checkpoint },
    { MSG_DRAIN,          on_worker_drain },
    { MSG_CACHE_REPORT,   on_cache_report },
    { MSG_FILE_MISSING,   on_file_missing },
    { MSG_CLIENT_LOGIN,   on_client_login },
    { MSG_SUBMIT,         on_submit },
    { MSG_NODES_REQ,      on_nodes_req },
    { MSG_STATUS_REQ,     on_status_req },
    { MSG_LOGS_REQ,       on_logs_req },
    { MSG_CANCEL_REQ,     on_cancel_req },
    { MSG_MIGRATE_REQ,    on_migrate_req },
    { MSG_CRON_LIST_REQ,  on_cron_list_req },
    { MSG_CANCEL_CRON_REQ,on_cancel_cron_req },
};

void dispatch_message(Conn *c, const MsgHeader *h, const void *payload) {
    for (size_t i = 0; i < sizeof table / sizeof table[0]; i++) {
        if (table[i].type == h->type) { table[i].fn(c, h, payload); return; }
    }
    log_line("SCHEDULER", "ignoring unknown message type %u", h->type);
}
