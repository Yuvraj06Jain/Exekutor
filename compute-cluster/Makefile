# Milestone 1 Makefile
#
# Folder layout (everything lives under src/ so the executables can be named
# exactly "scheduler", "worker" and "client" at the repo root):
#
#   src/common/*.c      shared by all three programs
#   src/scheduler/*.c   ->  ./scheduler
#   src/worker/*.c      ->  ./worker
#   src/client/*.c      ->  ./client
#
# Adding a new .c file inside one of those folders needs NO change here.
#
#   make             build all three programs
#   make worker      build only ./worker (useful while the others are not written yet)
#   make clean       delete build/ and the three executables

CC      = cc
CFLAGS  = -Wall -Wextra -g -pthread -Isrc
LDFLAGS = -pthread

BUILD = build

# Every .c file found in each folder
COMMON_SRC = $(wildcard src/common/*.c)
SCHED_SRC  = $(wildcard src/scheduler/*.c)
WORKER_SRC = $(wildcard src/worker/*.c)
CLIENT_SRC = $(wildcard src/client/*.c)

# Every header: if any header changes, everything is rebuilt (simple and safe)
HEADERS = $(wildcard src/*/*.h)

# src/dir/file.c  ->  build/dir/file.o
COMMON_OBJ = $(patsubst src/%.c,$(BUILD)/%.o,$(COMMON_SRC))
SCHED_OBJ  = $(patsubst src/%.c,$(BUILD)/%.o,$(SCHED_SRC))
WORKER_OBJ = $(patsubst src/%.c,$(BUILD)/%.o,$(WORKER_SRC))
CLIENT_OBJ = $(patsubst src/%.c,$(BUILD)/%.o,$(CLIENT_SRC))

all: scheduler worker client

# $@ = the target (e.g. scheduler), $^ = all the .o files listed after the colon
scheduler: $(SCHED_OBJ) $(COMMON_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

worker: $(WORKER_OBJ) $(COMMON_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

client: $(CLIENT_OBJ) $(COMMON_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile one .c into one .o ($< = the .c file). Makes the build/ subfolder if needed.
$(BUILD)/%.o: src/%.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD) scheduler worker client

.PHONY: all clean