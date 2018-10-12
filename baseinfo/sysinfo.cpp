/*************************************************************************
	> File Name: sysinfo.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年07月17日 星期二 16时09分32秒
 ************************************************************************/
#include "sysinfo.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

namespace sysinfo
{
double  cal_occupy (CPU_OCCUPY *o, CPU_OCCUPY *n){
    double od, nd;
    double id, sd;
    double scale;
    od = (double) (o->user + o->nice + o->system +o->idle);//第一次(用户+优先级+系统+空闲)的时间再赋给od
    nd = (double) (n->user + n->nice + n->system +n->idle);//第二次(用户+优先级+系统+空闲)的时间再赋给od
    scale = 100.0 / (float)(nd-od);       //100除强制转换(nd-od)之差为float类型再赋给scale这个变量
    id = (double) (n->user - o->user);    //用户第一次和第二次的时间之差再赋给id
    sd = (double) (n->system - o->system);//系统第一次和第二次的时间之差再赋给sd
    return ((sd+id)*100.0)/(nd-od); //((用户+系统)乖100)除(第一次和第二次的时间差)再赋给g_cpu_used
}

void  get_occupy (CPU_OCCUPY *o) {
    FILE *fd;
    int n;
    char buff[MAXBUFSIZE];
    fd = fopen ("/proc/stat", "r"); //这里只读取stat文件的第一行及cpu总信息，如需获取每核cpu的使用情况，请分析stat文件的接下来几行。
    fgets (buff, sizeof(buff), fd);
    sscanf (buff, "%s %u %u %u %u", o->name,
            &o->user,
            &o->nice,
            &o->system,
            &o->idle);
    fclose(fd);
}

void get_mem_occupy(MEM_OCCUPY * mem){
    FILE * fd;
    char buff[MAXBUFSIZE];
    fd = fopen("/proc/meminfo","r");
    fgets (buff, sizeof(buff), fd);
    sscanf (buff, "%s %ld", mem->name,&mem->total);
    fgets (buff, sizeof(buff), fd);
    sscanf (buff, "%s %ld", mem->name2,&mem->free);
    fclose(fd);
}

float get_io_occupy(){
    char cmd[] ="iostat -d -k";
    char buffer[MAXBUFSIZE];
    char a[20];
    float arr[20];
    FILE* pipe = popen(cmd, "r");
    if (!pipe)  return -1;
    fgets(buffer, sizeof(buffer), pipe);
    fgets(buffer, sizeof(buffer), pipe);
    fgets(buffer, sizeof(buffer), pipe);
    fgets(buffer, sizeof(buffer), pipe);
    sscanf(buffer,"%s %f %f %f %f %f %f %f %f %f %f %f %f %f ",a,&arr[0],&arr[1],&arr[2],&arr[3],&arr[4],&arr[5],&arr[6],&arr[7],&arr[8],&arr[9],&arr[10],&arr[11],&arr[12]);
    //printf("%f\n",arr[12]);

    pclose(pipe);
    return arr[12];
}
void get_disk_occupy(char ** reused){
    char currentDirectoryPath[ MAXBUFSIZE ];
    getcwd(currentDirectoryPath, MAXBUFSIZE);
    //printf("当前目录：%s\n",currentDirectoryPath);
    char cmd[50]="df ";
    strcat(cmd,currentDirectoryPath);
    //printf("%s\n",cmd);

    char buffer[MAXBUFSIZE];
    FILE* pipe = popen(cmd, "r");
    char fileSys[20];
    char blocks[20];
    char used[20];
    char free[20];
    char percent[10];
    char moment[20];

    if (!pipe)  return ;
    if(fgets(buffer, sizeof(buffer), pipe)!=NULL){
        sscanf(buffer,"%s %s %s %s %s %s",fileSys,blocks,used,free,percent,moment);
    }
    if(fgets(buffer, sizeof(buffer), pipe)!=NULL){
        sscanf(buffer,"%s %s %s %s %s %s",fileSys,blocks,used,free,percent,moment);
    }
    //printf("desk used:%s\n",percent);
    strcpy(*reused,percent);
    pclose(pipe);
    return ;
}

void getCurrentDownloadRates(long int * save_rate)
{
    char intface[] = "eth0:";  //这是网络接口名，根据主机配置
    //char intface[] = "wlan0:";
    FILE * net_dev_file;
    char buffer[1024];
    size_t bytes_read;
    char * match;
    if ( (net_dev_file=fopen("/proc/net/dev", "r")) == NULL )
    {
        printf("open file /proc/net/dev/ error!\n");
//        exit(EXIT_FAILURE);
        return;
    }

    int i = 0;
    while(i++<20){
        if(fgets(buffer, sizeof(buffer), net_dev_file)!=NULL){
            if(strstr(buffer,intface)!=NULL){
                //printf("%d   %s\n",i,buffer);
                sscanf(buffer,"%s %ld",buffer,save_rate);
                break;
            }
        }
    }
    if(i==20) *save_rate=0.01;
    fclose(net_dev_file); //关闭文件
    return ;
}
}
