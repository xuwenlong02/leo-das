#ifndef __DFILE_H__
#define __DFILE_H__
#include<stdio.h>
#include<string.h>

namespace utils

{
    int get_mem_info(char info[],int size);
    int get_cpu_info(char info[],int size);
    void write_debug_info(const char* str,...);
    void write_debug_info_to_dir(const char *dir,const char* str,...);
//extern void write_debug_time(char *str);
    void write_debug_time(const char*,...);

    bool file_exist (const char *file);

    int load_file(const char *dir,int start,char buf[],int count,int size = 1);
    int save_file(const char *dir,int start,const char buf[],int count,int size = 1);

    void delete_file(const char *dir);
    void reset_file(const char *dir,int length);
}


#endif // __DFILE_H__
