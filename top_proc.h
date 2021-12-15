#include <time.h>

/// process information
typedef struct proc_t {
    int tid;               // task id
    char cmd[16];          // name
    float pcpu;            // %CPU during sampling
    unsigned long utime;   // time in user mode
    unsigned long stime;   // time in kernel mode
    unsigned long vm_rss;  // resident set size (in kB)
} proc_t;

/// compare function for sorting proc_t entries by CPU usage, highest first
int proc_comp_pcpu(const void* e1, const void* e2);

/// compare function for sorting proc_t entries by resident set size, highest first
int proc_comp_rss(const void* e1, const void* e2);

/// populate procs argument with all currently running proceses,
/// the number of processes is stored in procs_size
///
/// note: the memory allocated for procs must be freed by the caller
int read_proc(proc_t** procs, unsigned int* procs_size);

/// populate proc argument by process information for the specified pid
int read_proc_pid(proc_t* proc, const char* pid);

/// populate procs argument with process information during the given sample time,
/// the number of sampled processes is stored in procs_size
///
/// note: the number of processes might be lower than procs_size, check
/// for the fist entry with an empty cmd member
///
/// note: the memory allocated for procs must be freed by the caller
void sample_processes(proc_t** procs,
                      unsigned int* procs_size,
                      struct timespec sample_time);
