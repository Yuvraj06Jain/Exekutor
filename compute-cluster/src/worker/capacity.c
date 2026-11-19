#include <unistd.h>
#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

#include "common/log.h"
#include "worker/capacity.h"

int machine_cores(void){
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    return (n > 0) ? (int)n : -1;
}

long long machine_ram_mb(void){
    long long bytes = -1;

    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    if(pages > 0 && page_size > 0)
        bytes = (long long)pages * page_size;

    #ifdef __APPLE__
    // macOS alternative from the brief: sysctl(HW_MEMSIZE), used only if sysconf failed
    if(bytes < 0){
        int mib[2] = { CTL_HW, HW_MEMSIZE };
        unsigned long long mem = 0;
        size_t len = sizeof mem;
        if(sysctl(mib, 2, &mem, &len, NULL, 0) == 0)
            bytes = (long long)mem;
    }
    #endif

    return (bytes > 0) ? bytes / (1024 * 1024) : -1;
}

// ASSUMPTION: if the real value cannot be detected we keep the requested value
// and warn, because refusing to start would be worse than trusting the user.
void clamp_capacity(WorkerConfig *cfg){
    int real_cores = machine_cores();
    if(real_cores < 0){
        log_line("WORKER", "Warning: could not detect the number of cores, not clamping.");
    }
    else if(cfg->cores > real_cores){
        log_line("WORKER", "Warning: requested %d cores but this machine has %d. Clamping to %d.",
                 cfg->cores, real_cores, real_cores);
        cfg->cores = real_cores;
    }

    long long real_ram = machine_ram_mb();
    if(real_ram < 0){
        log_line("WORKER", "Warning: could not detect the amount of RAM, not clamping.");
    }
    else if(cfg->ram_mb > real_ram){
        log_line("WORKER", "Warning: requested %dMB RAM but this machine has %lldMB. Clamping to %lldMB.",
                 cfg->ram_mb, real_ram, real_ram);
        cfg->ram_mb = (int)real_ram;
    }
}
