#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common/log.h"
#include "common/net.h"
#include "common/types.h"
#include "worker/job.h"

#define MAX_JOBS 64 // ASSUMPTION: a worker never runs more than this many jobs at once

typedef struct{
    int      in_use;
    uint32_t job_id;
    pid_t    pid;
}RunningJob;

static RunningJob jobs[MAX_JOBS];

static int send_update(int sock, uint32_t worker_id, uint32_t job_id, uint32_t state, char *reason){
    JobUpdateMsg u;
    memset(&u, 0, sizeof u);
    u.job_id = job_id;
    u.worker_id = worker_id;
    u.new_state = state;
    if(reason)
        snprintf(u.reason, sizeof u.reason, "%s", reason);
    return send_msg(sock, MSG_JOB_UPDATE, &u, sizeof u);
}

// Runs in the child, between fork() and execvp(). Never returns.
static void child_run(int sock, JobAssignMsg *a){
    close(sock);              // the job must not keep our scheduler connection open
    signal(SIGPIPE, SIG_DFL); // we ignore SIGPIPE, and ignored signals survive exec: undo it

    // TEMPORARY: silence the job. Section 5B replaces this with /tmp/osn_job_<id>/output.log
    int null = open("/dev/null", O_RDWR);
    if(null >= 0){
        dup2(null, 0);
        dup2(null, 1);
        dup2(null, 2);
        if(null > 2)
            close(null);
    }

    char *argv[] = { a->name, NULL };
    execvp(a->name, argv);
    _exit(127); // exec failed (for example command not found): 127 is the shell convention
}

// Turns a waitpid status into COMPLETED or FAILED plus a short reason.
// Plain wording only; the detailed diagnosis (Section 5E) comes in a later stage.
static void describe_exit(int status, uint32_t *state, char *reason, size_t n){
    reason[0] = '\0';
    if(WIFEXITED(status) && WEXITSTATUS(status) == 0)
        *state = COMPLETED;
    else if(WIFEXITED(status)){
        *state = FAILED;
        snprintf(reason, n, "Exited with code %d", WEXITSTATUS(status));
    }
    else if(WIFSIGNALED(status)){
        *state = FAILED;
        snprintf(reason, n, "Terminated by signal %d", WTERMSIG(status));
    }
    else{
        *state = FAILED;
        snprintf(reason, n, "Unknown exit status");
    }
}

// ASSUMPTION: until file transfer exists (Section 13), a.name is either a command
// found in PATH or a path that already exists on this machine; execvp handles both.
int job_start(int sock, uint32_t worker_id, char *tag, JobAssignMsg *a){
    a->name[sizeof a->name - 1] = '\0'; // never trust a string that came off the network
    log_line(tag, "Job %u ASSIGNED, forking...", a->job_id);

    int slot = -1;
    for(int i = 0; i < MAX_JOBS; i++){
        if(!jobs[i].in_use){
            slot = i;
            break;
        }
    }
    if(slot < 0){
        log_line(tag, "Job %u FAILED: too many jobs running at once.", a->job_id);
        return send_update(sock, worker_id, a->job_id, FAILED, "Worker has too many running jobs");
    }

    pid_t pid = fork();
    if(pid < 0){
        log_line(tag, "Job %u FAILED: fork failed.", a->job_id);
        return send_update(sock, worker_id, a->job_id, FAILED, "fork failed");
    }
    if(pid == 0)
        child_run(sock, a);

    jobs[slot].in_use = 1;
    jobs[slot].job_id = a->job_id;
    jobs[slot].pid = pid;
    log_line(tag, "Job %u RUNNING (pid %d)", a->job_id, (int)pid);
    return send_update(sock, worker_id, a->job_id, RUNNING, NULL);
}

int jobs_reap(int sock, uint32_t worker_id, char *tag){
    for(int i = 0; i < MAX_JOBS; i++){
        if(!jobs[i].in_use)
            continue;

        int status = 0;
        pid_t r = waitpid(jobs[i].pid, &status, WNOHANG);
        if(r == 0)
            continue; // still running

            uint32_t id = jobs[i].job_id;
        jobs[i].in_use = 0;

        uint32_t state;
        char reason[MAX_REASON];
        if(r < 0){
            state = FAILED;
            snprintf(reason, sizeof reason, "Lost track of the job process");
        }
        else
            describe_exit(status, &state, reason, sizeof reason);

        if(state == COMPLETED)
            log_line(tag, "Job %u COMPLETED", id);
        else
            log_line(tag, "Job %u FAILED: %s", id, reason);

        if(send_update(sock, worker_id, id, state, reason) < 0)
            return -1;
    }
    return 0;
}

uint32_t jobs_running(void){
    uint32_t n = 0;
    for(int i = 0; i < MAX_JOBS; i++)
        if(jobs[i].in_use)
            n++;
    return n;
}
