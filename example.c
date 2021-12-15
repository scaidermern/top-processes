#include "top_proc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main() {
    // sample for 500 ms
    struct timespec tm;
    tm.tv_sec  = 0;
    tm.tv_nsec = 500 * 1000 * 1000; /* 500 ms */

    proc_t* procs = NULL;
    unsigned int procs_len = 0;
    sample_processes(&procs, &procs_len, tm);

    // sort by CPU usage and print
    qsort(procs, procs_len, sizeof(procs[0]), proc_comp_pcpu);
    // print
    printf("%-10s %-10s %-10s %-10s\n", "PID", "<CPU>", "RSS", "CMD");
    for (unsigned int i = 0; i < procs_len && i < 5; ++i) {
        if (strlen(procs[i].cmd) == 0) {
            break;
        }
        printf("%-10d %-10.2f %-10lu %-10s\n", procs[i].tid, procs[i].pcpu, procs[i].vm_rss, procs[i].cmd);
    }

    printf("\n");
    // sort by resident set size and print
    qsort(procs, procs_len, sizeof(procs[0]), proc_comp_rss);
    printf("%-10s %-10s %-10s %-10s\n", "PID", "CPU", "<RSS>", "CMD");
    for (unsigned int i = 0; i < procs_len && i < 5; ++i) {
        if (strlen(procs[i].cmd) == 0) {
            break;
        }
        printf("%-10d %-10.2f %-10lu %-10s\n", procs[i].tid, procs[i].pcpu, procs[i].vm_rss, procs[i].cmd);
    }

    free(procs);
}
