#include "top_proc.h"

#include <sys/types.h>
#include <dirent.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int proc_comp_tid(const void* e1, const void* e2) {
    proc_t* p1 = (proc_t*)e1;
    proc_t* p2 = (proc_t*)e2;

    if (p1->tid < p2->tid) {
        return -1;
    } else if (p1->tid > p2->tid) {
        return 1;
    } else {
        return 0;
    }
}

int proc_comp_pcpu(const void* e1, const void* e2) {
    proc_t* p1 = (proc_t*)e1;
    proc_t* p2 = (proc_t*)e2;

    if (p1->pcpu < p2->pcpu) {
        return 1;
    } else if (p1->pcpu > p2->pcpu) {
        return -1;
    } else {
        return 0;
    }
}

int proc_comp_rss(const void* e1, const void* e2) {
    proc_t* p1 = (proc_t*)e1;
    proc_t* p2 = (proc_t*)e2;

    if (p1->vm_rss < p2->vm_rss) {
        return 1;
    } else if (p1->vm_rss > p2->vm_rss) {
        return -1;
    } else {
        return 0;
    }
}

unsigned long long get_total_cpu_time() {
    FILE* file = fopen("/proc/stat", "r");
    if (file == NULL) {
        perror("Could not open stat file");
        return 0;
    }

    char buffer[1024];
    unsigned long long user = 0, nice = 0, system = 0, idle = 0;
    // added between Linux 2.5.41 and 2.6.33, see man proc(5)
    unsigned long long iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guestnice = 0;

    char* ret = fgets(buffer, sizeof(buffer) - 1, file);
    if (ret == NULL) {
        perror("Could not read stat file");
        fclose(file);
        return 0;
    }
    fclose(file);

    sscanf(buffer,
           "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",
           &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guestnice);

    // sum everything up (except guest and guestnice since they are already included
    // in user and nice, see http://unix.stackexchange.com/q/178045/20626)
    return user + nice + system + idle + iowait + irq + softirq + steal;
}

int read_proc(proc_t** procs, unsigned int* procs_size) {
    *procs_size = 0;

    DIR* dir = opendir("/proc");
    if (dir == NULL) {
        perror("Could not open /proc");
        return -1;
    }

    struct dirent* file;
    while ((file = readdir(dir)) != NULL) {
        if (file->d_type == DT_DIR && isdigit(file->d_name[0])) {
            proc_t proc;
            if (read_proc_pid(&proc, file->d_name) != 0) {
                continue;
            }

            proc_t* newprocs = (proc_t*)realloc(*procs, (*procs_size + 1) * sizeof(proc_t));
            if (newprocs == NULL) {
                perror("realloc() failed");
                return -1;
            }

            *procs = newprocs;
            memcpy(&(*procs)[*procs_size], &proc, sizeof(proc_t));
            ++(*procs_size);
        }
    }

    closedir(dir);
    return 0;
}

int read_proc_pid(proc_t* proc, const char* pid) {
    // read stat file
    char path[32]; // must hold "/proc/" + highest possible PID (2^22 = 4194304) + file name
    snprintf(path, sizeof(path), "/proc/%s/stat", pid);
    FILE* file = fopen(path, "r" );
    if (file == NULL) {
        fprintf(stderr, "Could not open %s: %s", path, strerror(errno));
        return -1;
    }

    char buf[4096]; // length chosen by dice roll
    if (!fgets(buf, sizeof(buf), file)) {
        fprintf(stderr, "Could not read from %s: %s", path, strerror(errno));
        fclose(file);
        return -1;
    }

    // read pid
    int num = sscanf(buf, "%d", &proc->tid);
    if (num != 1) {
        fprintf(stderr, "Could not read task ID for %s\n", path);
        fclose(file);
        return -1;
    }

    // read name enclosed in brackets, name may contain brackets itself
    // -> reverse search for closing bracket
    char* start = strchr(buf,  '(') + 1;
    char* end   = strrchr(buf, ')');
    memcpy(&proc->cmd, start, end - start);
    proc->cmd[end - start] = '\0';

    // read remaining stuff we are interested in
    // note: when unignoring fields for sscanf() relookup the correct format specifier!
    char* pos = end + 2; // ') '
    num = sscanf(pos,
        "%*c "                       // state
        "%*d %*d %*d %*d %*d "       // ppid, pgrp, session, tty_nr, tpgid
        "%*u"                        // flags (was %lu before Linux 2.6)
        "%*u %*u %*u %*u "           // minflt, cminflt, majflt, cmajflt
        "%lu %lu %*d %*d "           // utime, stime, cutime, cstime
        "%*d %*d %*d %*d "           // priority, nice, num_threads, itrealvalue
        "%*u "                       // starttime
        "%*u "                       // vsize
        "%ld",                       // rss
        &proc->utime, &proc->stime, &proc->vm_rss);

    // convert from pages to kB
    proc->vm_rss = (proc->vm_rss * getpagesize()) / 1024;

    fclose(file);
    return 0;
}

void sample_processes(
        proc_t** result,
        unsigned int* result_size,
        struct timespec sample_time) {
    long num_cpus = sysconf(_SC_NPROCESSORS_ONLN);

    unsigned long long elapsed_cpu_time = get_total_cpu_time();

    // get all processes, first run
    proc_t* procs1 = NULL;
    unsigned int procs1_len;
    int ret = read_proc(&procs1, &procs1_len);
    if (ret != 0 || !procs1) {
        perror("read_proc() failed");
        return;
    }

    // sort by thread ID
    qsort(procs1, procs1_len, sizeof(proc_t), proc_comp_tid);

    // sleep some time
    // note: we don't care about interrupted nanosleep calls since the
    // sleeping time isn't included in any of the calculations done later
    nanosleep(&sample_time, NULL);

    // elapsed cpu time during sampling
    elapsed_cpu_time = get_total_cpu_time() - elapsed_cpu_time;

    // get all processes, second run
    proc_t* procs2 = NULL;
    unsigned int procs2_len;
    ret = read_proc(&procs2, &procs2_len);
    if (ret != 0 || !procs2) {
        perror("read_proc() failed");
        return;
    }

    // sort by thread ID
    qsort(procs1, procs1_len, sizeof(proc_t), proc_comp_tid);

    // merge
    *result_size = (procs1_len < procs2_len ? procs1_len : procs2_len);
    proc_t* tmp = calloc(*result_size, sizeof(proc_t));
    if (tmp == NULL) {
        perror("calloc() failed");
        return -1;
    }
    *result = tmp;
    unsigned int pos1 = 0, pos2 = 0;
    unsigned int newpos = 0;
    while (pos1 < procs1_len && pos2 < procs2_len) {
        if (procs1[pos1].tid < procs2[pos2].tid) {
            ++pos1;
        } else if (procs1[pos1].tid > procs2[pos2].tid) {
            ++pos2;
        } else {
            // found the same process in both lists
            (*result)[newpos].tid = procs2[pos2].tid;
            (*result)[newpos].vm_rss = procs2[pos2].vm_rss;
            strncpy((*result)[newpos].cmd, procs2[pos2].cmd, 15);

            // calculate CPU usage during sampling time
            unsigned long long cpu_time = (
                (procs2[pos2].utime + procs2[pos2].stime) -
                (procs1[pos1].utime + procs1[pos1].stime));
            (*result)[newpos].pcpu = (cpu_time / (float)elapsed_cpu_time) * 100.0 * num_cpus;

            ++pos1;
            ++pos2;
            ++newpos;
        }
    }

    free(procs1);
    free(procs2);
}
