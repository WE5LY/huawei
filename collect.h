#ifndef _COLLECT_H
#define _COLLECT_H

#define NAME_LEN 24     /* length of device name */

#define SYSFS_BLOCK "/sys/block"

#define DEVICE_CPU  0
#define DEVICE_DISK 1
#define DEVICE_MEM  2
#define DEVICE_NET  3

struct status_cpu {
    char name[NAME_LEN];
    unsigned long long user; 
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle; 
    unsigned long long iowait;        /* since Linux 2.5.41 */
    unsigned long long irq;           /* since Linux 2.6.0-test4 */
    unsigned long long softirq;       /* since Linux 2.6.0-test4 */
    unsigned long long steal;         /* since Linux 2.6.11 */
    unsigned long long guest;         /* since Linux 2.6.24 */
    unsigned long long guest_nice;    /* since Linux 2.6.33 */
};

struct status_disk {
    char name[NAME_LEN];

    /* reads completed successfully */
    unsigned long long rd_ios;
    /* reads merged */
    unsigned long long rd_merged;
    /* sectors read */
    unsigned long long rd_sector;
    /* writes completed */
    unsigned long long wr_ios;
    /* writes merged */
    unsigned long long wr_merged;
    /* sectors written */
    unsigned long long wr_sector;
    /* time spent reading (ms) */
    unsigned int rd_time;
    /* time spent writing (ms) */
    unsigned int wr_time;
    /* I/Os currently in progress */
    unsigned int io_prg;
    /* time spent doing I/Os (ms) */
    unsigned int total_time;
    /* weighted time spent doing I/Os (ms) */
    unsigned int wt_time;
};

/* disk usage after calculation with struct status_disk */
struct status_disk_io {
    char name[NAME_LEN];
    unsigned long long rd_kB_ps;
    unsigned long long wr_kB_ps;
};

struct status_mem {
    unsigned long long mem_total;
    unsigned long long mem_free;
    unsigned long long buffers;
    unsigned long long cached;
    unsigned long long swap_total;
    unsigned long long swap_free;
};

struct status_net {
    char name[NAME_LEN];
    unsigned long long rc_kB;
    unsigned long long rc_pkt;
    unsigned long long rc_err;
    unsigned long long rc_drp;
    unsigned long long rc_fifo;
    unsigned long long rc_frame;
    unsigned long long rc_cmprs;
    unsigned long long rc_multi;
    unsigned long long sd_kB;
    unsigned long long sd_pkt;
    unsigned long long sd_err;
    unsigned long long sd_drp;
    unsigned long long sd_fifo;
    unsigned long long sd_coll;
    unsigned long long sd_carrier;
    unsigned long long sd_cmprs;
};

#endif /* _COLLECT_H */
