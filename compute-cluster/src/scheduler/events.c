/* OWNER: C. Section 3 (events) + login registry on the scheduler side.
 * TODO(C): keep a registry of client connections by user name; emit_event pushes MSG_EVENT. */
#include "scheduler/hooks.h"
#include "scheduler/handlers.h"

#include <stdarg.h>
#include <stdio.h>

/* Temporary behaviour: just log the event. Replace with a real push to the user's client(s). */
void emit_event(const char *user, const char *fmt, ...) {
    char text[MAX_REASON * 2];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(text, sizeof text, fmt, ap);
    va_end(ap);
    log_line("EVENT-STUB", "to %s: %s", user, text);
}
void events_conn_closed(Conn *c) { UNUSED(c); }
HANDLER(on_client_login) { UNUSED(c); UNUSED(h); UNUSED(payload); NOT_IMPLEMENTED("on_client_login"); }
