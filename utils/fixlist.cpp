#include "fixlist.h"
#include "stdio.h"
#include "dfile.h"
#include <vector>
#include <defines.h>
#include <stdlib.h>

#define DATA_SHIFT   sizeof(U64)

template <class T,U64 M>
FixList<T,M>::FixList(S32 id):
    fileData(id){
}

template <class T,U64 M>
FixList<T,M>::FixList(const FixList &fixlist)
{
    operator =(fixlist);
}

template <class T,U64 M>
FixList<T,M>& FixList<T,M>::operator =(const FixList<T,M> &fixlist)
{
    fileData = fixlist.fileData;
    return *this;
}

template <class T,U64 M>
FixList<T,M>::~FixList()
{
    /* 保存内容 */
}

template <class T,U64 M>
void FixList<T,M>::removeLast()
{
    fileData.removeLast();
}

template <class T,U64 M>
bool FixList<T,M>::isFull()
{
    U16 length = fileData.readLength();
    return (M!=-1 && length == M);
}

template <class T,U64 M>
void FixList<T,M>::push_back(T &t)
{
    append(t);
}

template <class T,U64 M>
void FixList<T,M>::append(T &t)
{
    if (isFull()) {
        fileData.removeAt(0);
    }

    fileData.writeBack(t);
}

template <class T,U64 M>
T FixList<T,M>::operator [](int index)
{
    T  t;
    this->at(&t,index);
    return  t;
}

template <class T,U64 M>
T FixList<T,M>::at(int index)
{
    T  t;
    this->at(&t,index);
    return  t;
}


template <class T,U64 M>
bool FixList<T,M>::at(T *d, int start)
{
    U64 length = fileData.readLength();
    if (start >= length)
        return false;
    *d = fileData.readAt(start);
    return true;
}

template <class T,U64 M>
T FixList<T,M>::end()
{
    return fileData.readLast();
}

template <class T,U64 M>
void FixList<T,M>::replace(int start, T &t)
{
    fileData.writeAt(start,t);
}

template<class T,U64 M>
void FixList<T,M>::remove(int index)
{
    fileData.removeAt(index);
}

template <class T,U64 M>
void FixList<T,M>::clear()
{
    fileData.clear();
}

template <class T,U64 M>
bool FixList<T,M>::isEmpty()
{
    return fileData.empty();
}

template <class T,U64 M>
int FixList<T,M>::count()
{
    return fileData.readLength();
}

template <class T,U64 M>
int FixList<T,M>::size()
{
    return count ();
}

template <class T,U64 M>
FixList<T,M>::FileData::FileData(S32 id):
    file_cs(true)
{
    int len = 30+strlen(getenv(CONFIG_DIR_ENV));
    filename = (char*)malloc(len);
    memset(filename,0,len);
    sprintf(filename,"%s/data/LIST%04d.data",getenv(CONFIG_DIR_ENV),id);

    if(!utils::file_exist(filename))
    {
        writeLength(0);
    }
}

template <class T,U64 M>
FixList<T,M>::FileData::FileData(const FixList::FileData &fileData):
    file_cs(true)
{
    char *filename_ = (char*)malloc(strlen(fileData.filename)+1);
    strcpy(filename_,fileData.filename);
    if (filename) {
        free(filename);
    }
    filename = filename_;
}

template <class T,U64 M>
FixList<T,M>::FileData::~FileData()
{
    AutoLock autolock(file_cs);
    free(filename);
}

template <class T,U64 M>
U64 FixList<T,M>::FileData::readLength()
{
    AutoLock autolock(file_cs);
    U64 length = 0;
    utils::load_file(filename,0,(char*)&length,DATA_SHIFT);
    return length;
}

template <class T,U64 M>
void FixList<T,M>::FileData::writeLength(U64 length)
{
    AutoLock autolock(file_cs);
    utils::save_file(filename,0,(char*)&length,DATA_SHIFT);
}

template <class T,U64 M>
T FixList<T,M>::FileData::readLast()
{
    AutoLock autolock(file_cs);
    U64  length = readLength();
    return readAt(length-1);
}

template <class T,U64 M>
T FixList<T,M>::FileData::readAt(U64 index)
{
    AutoLock autolock(file_cs);
    T d;
    utils::load_file(filename,DATA_SHIFT+sizeof(T)*index,(char*)&d,1,sizeof(T));
    return d;
}

template <class T,U64 M>
void FixList<T,M>::FileData::writeAt(U64 index,const T& d)
{
    AutoLock autolock(file_cs);
    U64  length = readLength();

    if (index>=length)
        return;
    utils::save_file(filename,DATA_SHIFT+sizeof(T)*index,(const char*)&d,1,sizeof(T));
}

template <class T,U64 M>
void FixList<T,M>::FileData::writeBack(const T &d)
{
    AutoLock autolock(file_cs);
    U64  length = readLength();

    utils::save_file(filename,DATA_SHIFT+length*sizeof(T),(char*)&d,1,sizeof(T));
    length ++;
    writeLength(length);
}

template <class T,U64 M>
void FixList<T,M>::FileData::removeLast()
{
    AutoLock autolock(file_cs);
    U64  length = readLength();
    length--;
    writeLength(length);
    utils::reset_file(filename,DATA_SHIFT+length*sizeof(T));
}

template <class T,U64 M>
void FixList<T,M>::FileData::clear()
{
    AutoLock autolock(file_cs);
    utils::reset_file(filename,DATA_SHIFT);
    writeLength(0);
}

template <class T,U64 M>
void FixList<T,M>::FileData::removeAt(U64 index)
{
    AutoLock autolock(file_cs);
    U64  length = readLength();

    if (index>=length)
        return;
    U64 rl = length-index-1;
    length--;

    T *data = (T*)malloc(sizeof(T)*rl);
    bzero(data,sizeof(T)*rl);
    writeLength(length);
    utils::load_file(filename,DATA_SHIFT+(index+1)*sizeof(T),(char*)data,rl,sizeof(T));
    utils::save_file(filename,DATA_SHIFT+index*sizeof(T),(char*)data,rl,sizeof(T));
    utils::reset_file(filename,DATA_SHIFT+length*sizeof(T));

    free(data);
}

template <class T,U64 M>
bool FixList<T,M>::FileData::empty()
{
    AutoLock autolock(file_cs);
    U64  length = readLength();
    return length == 0;
}
