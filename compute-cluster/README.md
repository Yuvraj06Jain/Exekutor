# Distributed Compute Cluster

Three programs in C: `scheduler`, `worker`, `client`. Team of 3. Deadline: 2 December 2026.
This is the **Gate 0 skeleton**: everything compiles, every function someone else will call
exists as a stub, and nobody needs to wait for anyone.

## Build and run

```
make            # builds ./scheduler ./worker ./client at the repo root
make clean
```
The executables must keep these exact names and command lines (see the brief, Sections 2 and 3).
Source lives under `src/` because the root must be free for the executables.
Adding a `.c` file under `src/<dir>/` needs **no Makefile edit**.

## Who owns what (A, B, C: fill in names)

| Person | Name | Role |
|---|---|---|
| A | | Worker and Sandbox |
| B | | Scheduler Core |
| C | | Client and Scheduler Extensions |

Every source file starts with `OWNER: X`. Owners by directory:

| Path | Owner |
|---|---|
| `src/worker/*` except `cache.c` | A |
| `src/scheduler/cron.c`, `migration.c` | A |
| `src/client/cmd_cron.c`, `cmd_migrate.c` | A |
| `src/scheduler/main.c dispatch.c queue.c workers.c heartbeat.c state.c pass.c preempt.c` | B |
| `src/client/*` except A's two command files | C |
| `src/common/*` | C (contract rules below apply) |
| `src/scheduler/events.c logbuf.c deps.c quota.c filesend.c` | C |
| `src/worker/cache.c` | C |

## The contract (frozen after Gate 0)

| File | What it fixes |
|---|---|
| `src/common/types.h` | constants, ALL job and worker states, `JobSpec`, `Job`, `Worker`, telemetry |
| `src/common/protocol.h` | every message type, payload structs, byte order, send order for a job start |
| `src/common/net.h` | `send_msg` / `recv_msg` framing, `tcp_listen`, `tcp_connect` (implemented) |
| `src/common/log.h` | `log_line(tag, fmt, ...)` (implemented) |
| `src/scheduler/sched.h` | `Conn`, B's state/queue API, the locking rule |
| `src/scheduler/hooks.h` | functions one person writes and another calls (who/when) |
| `src/scheduler/handlers.h` | one handler per incoming message; table in `dispatch.c` |
| `src/worker/worker.h` | worker-side cache functions (C) called by A's worker loop |
| `src/client/client.h` | command table so each person adds commands in their own file |

### Contract rules
1. **Additive only.** Add a field or message; never rename or remove.
2. Post the exact change in the group chat. Everyone pulls before continuing.
3. A contract change goes through a PR that tags the other two.
4. Everything inside your own files: change freely, no asking.

### How stubs work
Each stub lives in its owner's file. When you implement the real function, **replace the stub
in place** (same file, same signature). Stubs print `[STUB] <name> is not implemented yet`,
so the log shows exactly what is still missing.

## Your first steps after pulling
- **A:** write `tests/mocks/a/` (fake scheduler), then `src/worker/main.c` (CLI, clamping), then 5A.
- **B:** write `tests/mocks/b/` (fake worker and client), then `src/scheduler/main.c`, queue, pass.
- **C:** write `tests/mocks/c/` (fake scheduler), then `src/client/main.c`, login, commands. Do discovery early: A and B fall back to the command-line address until you do.

## Git rules (short version)
- `main` always builds (`make clean && make`).
- Branch per task: `a/sandbox-limits`, `b/heartbeats`, `c/login`. Merge your own files yourself once `make` passes.
- Someone else's file, `common/`, or any contract file: open a PR and tag them.
- No squash merges (history shows who did what). Commit format: `<imperative summary> (<section>)`, e.g. `Add 500 ms output tailing to worker (5B)`.
- `git pull --rebase` before every push.

## Brief rules to remember
No AI agents on the code (a chat window for explanations and debugging is fine). Modular code, no giant `main.c`.
`fork()` + `execvp()`, never `system()`. Test with `sudo`. Linux and macOS, no Docker.
