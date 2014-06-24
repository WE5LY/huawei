#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "collect.h"

#define ERROR(x) perror(x)

void submit(int devicetype, void *device)
{
    switch (devicetype)
    {
        case DEVICE_CPU:
            printf("%s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu \n", 
                ((struct status_cpu*)device)->name, ((struct status_cpu*)device)->user, 
                ((struct status_cpu*)device)->nice, ((struct status_cpu*)device)->system,
                ((struct status_cpu*)device)->idle, ((struct status_cpu*)device)->iowait,
                ((struct status_cpu*)device)->irq, ((struct status_cpu*)device)->softirq,
                ((struct status_cpu*)device)->steal, ((struct status_cpu*)device)->guest,
                ((struct status_cpu*)device)->guest_nice);
            break;

        case DEVICE_DISK:
            printf("%s %llu %llu\n",
                    ((struct status_disk_io*)device)->name, 
                    ((struct status_disk_io*)device)->rd_kB_ps, 
                    ((struct status_disk_io*)device)->wr_kB_ps);
            break;

        case DEVICE_MEM:
            printf("mem total: %llu \n mem free: %llu \n buffers: %llu\ncached: %llu\n \
swap total: %llu \n swap free: %llu\n ", 
                    ((struct status_mem*)device)->mem_total, ((struct status_mem*)device)->mem_free, 
                    ((struct status_mem*)device)->buffers, ((struct status_mem*)device)->cached, 
                    ((struct status_mem*)device)->swap_total, ((struct status_mem*)device)->swap_free);
            break;

    }
}



int cpu(void)
{
    struct status_cpu cpu;
    char buf[512];
    FILE *file;
    int  count;

    if ((file = fopen("/proc/stat", "r")) == NULL) {
        ERROR("Can't open /proc/stat");
        return -1;
    }

    while (fgets(buf, 512, file) != NULL) {
        if (strncasecmp(buf, "cpu", 3) != 0)
            continue;
        
        if (buf[3] < '0' || buf[3] > '9')
            continue;

        memset(&cpu, 0, sizeof(struct status_cpu));
        count = sscanf (buf, "%s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", 
                cpu.name, &cpu.user, &cpu.nice, &cpu.system, &cpu.idle, &cpu.iowait, &cpu.irq, 
                &cpu.softirq, &cpu.steal, &cpu.guest, &cpu.guest_nice);

        submit(DEVICE_CPU, &cpu);
    }
}

/* check a item in diskstat is a partion or a device */
int is_device(char *name)
{
    char syspath[PATH_MAX];
    char *slash;

    /* Some devices may have a slash in their name (eg. cciss/c0d0...) */
    while ((slash = strchr(name, '/')))
        *slash = '!';

    snprintf(syspath, sizeof(syspath), "%s/%s%s", SYSFS_BLOCK, name, "/device");
    
    return !access(syspath, F_OK);
}

int disk(void)
{
    struct status_disk_io diskio;
    struct status_disk *pdev[2];
    char buf[512];
    FILE *file;
    int major, minor;
    int linenum = 0, devnum = 0;
    int i = 0, j = 0, k;



    if ((file = fopen("/proc/diskstats", "r")) == NULL ) {
        ERROR("Can't open /proc/diskstats");
        return -1;
    }

    /* get the number of devices and partions */
    while (fgets(buf, 512, file) != NULL)
        linenum++;

    pdev[0] = (struct status_disk *) calloc(linenum, sizeof(struct status_disk));
    pdev[1] = (struct status_disk *) calloc(linenum, sizeof(struct status_disk));

    memset(pdev[0], 0, linenum * sizeof(struct status_disk));
    memset(pdev[1], 0, linenum * sizeof(struct status_disk));

    fclose(file);


    /* read status from diskstat, read twice */
    while (1) {
        if ((file = fopen("/proc/diskstats", "r")) == NULL ) {
            ERROR("Can't open /proc/diskstats");
            return -1;
        }

        j = 0;
        while (fgets(buf, 512, file) != NULL) {
            /* major minor name rd_ios rd_merges rd_sector rd_time wr_ios wr_merges wr_sector wr_time io_prg total_time weighted_time */
            sscanf(buf, "%d %d %s %llu %llu %llu %d %llu %llu %llu %d %d %d %d", &major, &minor, (pdev[i] + j)->name,
                    &(pdev[i] + j)->rd_ios, &(pdev[i] + j)->rd_merged, &(pdev[i] + j)->rd_sector, &(pdev[i] + j)->rd_time, 
                    &(pdev[i] + j)->wr_ios, &(pdev[i] + j)->wr_merged, &(pdev[i] + j)->wr_sector, &(pdev[i] + j)->wr_time,
                    &(pdev[i] + j)->io_prg, &(pdev[i] + j)->total_time, &(pdev[i] + j)->wt_time);
            j++;
        }

        fclose(file);

        if (0 == i) {
            i++;
            sleep(2);  /* set a timer */
        }
        else 
            break;
    }

    for (k = 0; k < linenum; k++) {
        if (!is_device((pdev[0] + k)->name))
            continue;
       strncpy(diskio.name, (pdev[0] + k)->name, NAME_LEN);

#ifdef DEBUG
       printf("pdev[0]: %llu %llu\n", (pdev[0] + k)->rd_ios, (pdev[0] + k)->wr_ios);
       printf("pdev[1]: %llu %llu\n", (pdev[1] + k)->rd_ios, (pdev[1] + k)->wr_ios);
#endif


       diskio.rd_kB_ps = ((pdev[1] + k)->rd_ios - (pdev[0] + k)->rd_ios) >> 1;
       diskio.wr_kB_ps = ((pdev[1] + k)->wr_ios - (pdev[0] + k)->wr_ios) >> 1;
       
       submit(DEVICE_DISK, &diskio);
    }
}

int memory()
{
    struct status_mem memory;
    FILE *file;
    char buf[512];
    unsigned long long  mem_total, mem_free, swap_total, swap_free, buffers, cached;
    unsigned long long  *field;


    if ((file = fopen("/proc/meminfo", "r")) == NULL ) {
        ERROR("Can't open /proc/diskstats");
        return -1;
    }

    while (fgets(buf, 512, file) != NULL) {
        if (strncasecmp(buf, "MemTotal:", 9) == 0)
            field = &mem_total;
        else if (strncasecmp(buf, "MemFree:", 8) == 0)
            field = &mem_free;
        else if (strncasecmp(buf, "Buffers:", 8) == 0)
            field = &buffers;
        else if (strncasecmp(buf, "Cached:", 7) == 0)
            field = &cached;
        else if (strncasecmp(buf, "SwapTotal:", 10) == 0)
            field = &swap_total;
        else if (strncasecmp(buf, "SwapFree:", 9) == 0)
            field = &swap_free;
        *field = atoll((strchr(buf, ':') + 1));
    }

    memory.mem_total = mem_total;
    memory.mem_free = mem_free;
    memory.buffers = buffers;
    memory.cached = cached;
    memory.swap_total = swap_total;
    memory.swap_free = swap_free;

    submit(DEVICE_MEM, &memory);
}

int main(){
    cpu();
    disk();
    memory();
}
