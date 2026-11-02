/* OWNER: C. Command registry + core commands (nodes, submit, status, cancel).
 * TODO(C): implement the four core commands in cmd_core_register(). */
#include <string.h>
#include "client/client.h"

#define MAX_COMMANDS 32
static struct { const char *name; CmdFn fn; const char *help; } cmds[MAX_COMMANDS];
static int n_cmds;

void register_command(const char *name, CmdFn fn, const char *help) {
    if (n_cmds < MAX_COMMANDS) {
        cmds[n_cmds].name = name; cmds[n_cmds].fn = fn; cmds[n_cmds].help = help;
        n_cmds++;
    }
}

CmdFn find_command(const char *name) {
    for (int i = 0; i < n_cmds; i++)
        if (strcmp(cmds[i].name, name) == 0) return cmds[i].fn;
    return NULL;
}

void cmd_core_register(void) { }

void commands_init(void) {
    cmd_core_register();
    cmd_logs_register();
    cmd_cron_register();
    cmd_migrate_register();
}
