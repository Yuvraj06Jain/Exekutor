/* CONTRACT FILE (client side): a command table so each person adds commands in
 * their own file with no merge conflicts. */
#ifndef CLIENT_CLIENT_H
#define CLIENT_CLIENT_H
#include "common/types.h"
#include "common/protocol.h"
#include "common/log.h"
#include "common/net.h"

typedef struct ClientCtx {
    int  sock;                       /* connected to the scheduler */
    char user[MAX_USERNAME + 1];
} ClientCtx;

/* argv[0] is the command word. Return 0 ok, -1 error (print your own message). */
typedef int (*CmdFn)(ClientCtx *ctx, int argc, char **argv);

void register_command(const char *name, CmdFn fn, const char *help);   /* C: commands.c */
CmdFn find_command(const char *name);                                  /* C: commands.c */
void commands_init(void);   /* C: calls every *_register() below once at startup */

void cmd_core_register(void);     /* C: nodes submit status cancel  (commands.c) */
void cmd_logs_register(void);     /* C: logs, logs -f               (cmd_logs.c) */
void cmd_cron_register(void);     /* A: crons, cancel-cron          (cmd_cron.c) */
void cmd_migrate_register(void);  /* A: migrate                     (cmd_migrate.c) */

#endif
