#include "top_proc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <proc/readproc.h>

int main() {
    // sample for 500 ms
    struct timespec tm;
    tm.tv_sec  = 0;
    tm.tv_nsec = 500 * 1000 * 1000; /* 500 ms */

    myproc_t* myprocs = NULL;
    unsigned int myprocs_len = 0;
    sample_processes(&myprocs, &myprocs_len, tm);
    
    // sort by CPU usage and print
    qsort(myprocs, myprocs_len, sizeof(myprocs[0]), myproc_comp_pcpu);
    // print
    printf("%-10s %-10s %-10s %-10s\n", "PID", "<CPU>", "RSS", "CMD");
    for (unsigned int i = 0; i < myprocs_len && i < 5; ++i) {
        if (strlen(myprocs[i].cmd) == 0) {
            break;
        }
        printf("%-10d %-10.2f %-10lu %-10s\n", myprocs[i].tid, myprocs[i].pcpu, myprocs[i].vm_rss, myprocs[i].cmd);
    }

    printf("\n");
    // sort by resident set size and print
    qsort(myprocs, myprocs_len, sizeof(myprocs[0]), myproc_comp_rss);
    printf("%-10s %-10s %-10s %-10s\n", "PID", "CPU", "<RSS>", "CMD");
    for (unsigned int i = 0; i < myprocs_len && i < 5; ++i) {
        if (strlen(myprocs[i].cmd) == 0) {
            break;
        }
        printf("%-10d %-10.2f %-10lu %-10s\n", myprocs[i].tid, myprocs[i].pcpu, myprocs[i].vm_rss, myprocs[i].cmd);
    }
    
    free(myprocs);
}
