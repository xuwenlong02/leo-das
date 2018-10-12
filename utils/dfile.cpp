#include<sys/types.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<unistd.h>
#include <stdlib.h>
//#include <
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <logger.h>

#define DFile       "xhnc.txt"
#define DMFile    "mhnc.txt"


//#define C_FILE

//QMutex s_mutex_fd,s_mutex_n;

namespace utils
{

int get_mem_info(char info[],int size)
{
    return 0;
//        __pid_t id=getpid();
//        char fi[100]={0};
//        sprintf(fi,"/proc/%d/status",id);
//	int fd;
//        fd=open(fi,O_RDONLY); // /proc/meminfo"
//	read(fd,info,size);
//	close(fd);
//	return fd;
}

int get_cpu_info(char info[],int size)
{
    FILE *fd=fopen("/proc/cpuinfo","rb");
    if(NULL==fd)
        return -1;
    fread(info,size,1,fd);
    return fclose(fd);
}

void write_debug_info_to_dir(const char *dir,const char* str,...)
{
//    char buf[500]={0};
//    va_list  ap;
////    int retval;
//    va_start(ap,str);

//    vsprintf(buf,str,ap);
//    va_end(ap);
//    int fd;
//    fd=open(dir,O_CREAT|O_WRONLY|O_APPEND);
//    write(fd,buf,strlen(buf));
//    close(fd);
}

void write_debug_info(const char* str,...)
{
//    return;
//    if(debug_text_brower==NULL)
//        return;
//    if(!debug_text_brower->IsRun())
//        return;
//    char buf[500]={0};
//    va_list  ap;
//    int retval;
//    va_start(ap,str);

//    vsprintf(buf,str,ap);
//    va_end(ap);
////    int fd;
////    fd=open(DFile,O_CREAT|O_WRONLY|O_APPEND);
////    write(fd,buf,strlen(buf));
////    close(fd);
//    debug_text_brower->write_debug_info(QString(buf));
}


void write_debug_time(const char* str,...)
{
////    return;
//    if(debug_text_brower==NULL)
//        return;
//    if(!debug_text_brower->IsRun())
//        return;
//    char buf[200]={0};
//    va_list  ap;
////    int retval;
//    va_start(ap,str);

//    vsprintf(buf,str,ap);
//    va_end(ap);
//    char  cbuf[300]={0};
//    sprintf(cbuf,"%s:%s",buf,QDateTime::currentDateTime().time().toString("mm:ss:zzz").toStdString().data());

//    int fd;
//    fd=open("smpl_inj",O_WRONLY|O_CREAT|O_APPEND);
//    write(fd,buf,strlen(cbuf));
//    close(fd);

//    debug_text_brower->write_debug_info(QString(cbuf));
}

bool file_exist (const char *file)
{
	if (file == NULL)
		return false;
    return access(file, 0)==0;
}

int load_file(const char *dir,int start,char buf[],int count,int size)
{
    if(dir==NULL||buf==NULL||size<=0||start<0)
        return -1;
//    chmod(dir,S_IREAD|S_IWRITE);

#ifdef C_FILE
    FILE *fd=fopen(dir,"rb+");
    if(NULL==fd)
        return -1;
    if (-1 == fseek(fd,start,SEEK_SET)) {
        LOG_ERR("fseek error");
    }
    int r = fread(buf,size,count,fd);
    fclose(fd);
    return 1;
#else
    int fd = open(dir,O_RDONLY|O_CREAT,0755);
    int sk = lseek(fd,start,SEEK_SET);
    int rd = read(fd,buf,size*count);
    close(fd);
    return rd;
#endif
}
void delete_file(const char *dir)
{
    if(dir==NULL)
        return;
//    chmod(dir,S_IREAD|S_IWRITE|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    remove(dir);
//    QFile::remove(QString(dir));
}

int save_file(const char *dir,int start,const char buf[],int count,int size)
{
    if(dir==NULL||buf==NULL||size<=0||start<0)
        return -1;
    //if(access(dir,F_OK))
//        chmod(dir,S_IREAD|S_IWRITE);
#ifdef C_FILE
    FILE *fd=fopen(dir,"ab+");
    if(NULL==fd)
        return -1;
    if (-1 == fseek(fd,start,SEEK_SET)) {
        LOG_ERR("fseek error");
    }
//    read(fd,(char*)buf,size);
    int w = fwrite(buf,size,count,fd);
    fclose(fd);
    return 1;
#else
    int fd = open(dir,O_WRONLY|O_CREAT,0755);
    lseek(fd,start,SEEK_SET);
    int r = write(fd,buf,size*count);
    close(fd);
    return r;
#endif
}

void reset_file(const char *dir,int length)
{
#ifdef C_FILE
    FILE *fd=fopen(dir,"wb+");
    if(NULL==fd)
        return;
    ftruncate(fileno(fd),length);
    fclose(fd);
#else
    int fd = open(dir,O_WRONLY,0755);
    ftruncate(fd,length);
    close(fd);
#endif
}
}

