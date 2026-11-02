# Builds ./scheduler ./worker ./client at the repo root.
# Wildcards: adding a .c file under src/<dir>/ needs NO Makefile edit.
CC      ?= cc
CFLAGS  := -std=gnu11 -Wall -Wextra -g -O0 -pthread -Isrc $(CFLAGS)
LDFLAGS := -pthread $(LDFLAGS)
BUILD   := build

COMMON_SRC := $(wildcard src/common/*.c)
SCHED_SRC  := $(wildcard src/scheduler/*.c)
WORKER_SRC := $(wildcard src/worker/*.c)
CLIENT_SRC := $(wildcard src/client/*.c)
HDRS       := $(wildcard src/*/*.h)

COMMON_OBJ := $(COMMON_SRC:src/%.c=$(BUILD)/%.o)
SCHED_OBJ  := $(SCHED_SRC:src/%.c=$(BUILD)/%.o)
WORKER_OBJ := $(WORKER_SRC:src/%.c=$(BUILD)/%.o)
CLIENT_OBJ := $(CLIENT_SRC:src/%.c=$(BUILD)/%.o)

.PHONY: all clean
all: scheduler worker client

scheduler: $(SCHED_OBJ) $(COMMON_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

worker: $(WORKER_OBJ) $(COMMON_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

client: $(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(BUILD)/%.o: src/%.c $(HDRS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD) scheduler worker client
