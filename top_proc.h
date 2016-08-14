#include <time.h>

/// parts of procps' proc_t that we might be interested in
typedef struct myproc_t {
    int tid;               // task id, the POSIX thread ID
    float pcpu;            // %CPU usage during measurement (not avg since startup)
    unsigned long vm_rss;  // resident set size (in kB)
    char cmd[16];          // basename of executable file in call to exec(2)
} myproc_t;

/// compare function for sorting myproc_t entries by CPU usage, highest first
int myproc_comp_pcpu(const void* e1, const void* e2);

/// compare function for sorting myproc_t entries by resident set size, highest first
int myproc_comp_rss(const void* e1, const void* e2);

/// fills in myprocs with process information during the given sample time,
/// the number of sampled processes is stored in myprocs_size
///
/// note: the number of processes might be lower than myprocs_size, check
/// for the fist entry with an empty cmd member
///
/// note: the memory allocated for myprocs must be freed by the caller
/// 
void sample_processes(myproc_t** myprocs,
                      unsigned int* myprocs_size,
                      struct timespec sample_time);