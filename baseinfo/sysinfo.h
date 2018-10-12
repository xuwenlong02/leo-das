/*************************************************************************
	> File Name: sysinfo.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年07月17日 星期二 16时09分53秒
 ************************************************************************/
#ifndef __SYSINFO_H__
#define __SYSINFO_H__

#define MAXBUFSIZE 1024
#define WAIT_SECOND 3   //暂停时间，单位为“秒”
typedef  struct occupy
{
    char name[20];
    unsigned int user;
    unsigned int nice;
    unsigned int system;
    unsigned int idle;
} CPU_OCCUPY ;

typedef struct PACKED
{
    char name[20];
    long total;
    char name2[20];
    long free;
}MEM_OCCUPY;

namespace sysinfo
{
double  cal_occupy (CPU_OCCUPY *o, CPU_OCCUPY *n);

void  get_occupy (CPU_OCCUPY *o);

void get_mem_occupy(MEM_OCCUPY * mem);

float get_io_occupy();

void get_disk_occupy(char ** reused);

void getCurrentDownloadRates(long int * save_rate);
}
#endif
