#ifndef __FIXLIST_H__
#define __FIXLIST_H__

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <defines.h>
#include "lock.h"

//--------------------------------------------------------------------------------------------------
//使用相当于QList的用法，只是内部处理不同，完全可以当QList来用
//---------------------------------------------------------------------------------------------------

//泛型类
template <class T,U64 M>
class FixList
{
public:
    //len:动态数组长度，当超过len时会自动保存到文件中，动态数组中始终
    //存放的是最后一波数据
    //max：总长度，超过部分将删除，若max==-1，则表示无限长

    FixList(S32 id);
//    FixList(const char* idstr);

    FixList(const FixList& fixlist);

    FixList& operator=(const FixList& fixlist);

    virtual ~FixList();

    void push_back (T &t);

    void append(T &t);

    T operator[](int index);

    T  at(int index);

    bool at(T *d,int start);

    T end();

    int last();

    void replace(int start,T &t);

    void remove(int index);

    void clear();

    bool  isEmpty();

    int  count();//当前的数据个数

    int size ();

private:
    //是否已经满了
    bool isFull();
    void removeLast();
private:
    class FileData{
    public:
        FileData(S32 id);
        FileData(const FileData& fileData);
        virtual ~FileData();

        U64 readLength();
        void writeLength(U64 length);
        T readLast();
        T readAt(U64 index);
        void writeAt(U64 index, const T &d);
        void writeBack(const T& d);
        void removeLast();
        void clear();
        void removeAt(U64 index);
        bool empty();
    private:
        char *filename;
        Lock file_cs;
    };

    FileData fileData;
};

#endif // FixList_H
