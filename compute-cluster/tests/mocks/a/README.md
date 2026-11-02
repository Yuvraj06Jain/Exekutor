# Mock A: fake scheduler (A writes this)
Accepts one worker connection, replies to MSG_REGISTER with MSG_REGISTER_ACK, sends a MSG_JOB_ASSIGN
for a PATH command (e.g. `ls`), and prints every message the worker sends back.
Use `src/common/net.c` for framing. Lets A test the worker with no real scheduler.
