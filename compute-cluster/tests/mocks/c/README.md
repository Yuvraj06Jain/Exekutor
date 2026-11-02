# Mock C: fake scheduler for the client (C writes this)
Accepts CLIENT_LOGIN, answers SUBMIT with MSG_REPLY, answers NODES_REQ/STATUS_REQ with MSG_REPLY_TEXT,
and pushes a MSG_EVENT every few seconds. Also unit tests for deps/quota/cache using hand-made Job and Worker structs.
