/*************************************************************************
	> File Name: transport.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年04月12日 星期四 15时07分48秒
 ************************************************************************/

#ifndef __TRANSPORTS_H__
#define __TRANSPORTS_H__

#include "dbms.h"
#include <string>

using namespace std;

class transports:public dbms
{
public:
    transports(int id);
    BOOL inorup(F32 sample,U64 regtime,S32 timeout,S32 mode,const char *addr);
    BOOL insert(F32 sample,U64 regtime,S32 timeout,S32 mode,const char *addr);
    BOOL update(F32 sample,U64 regtime,S32 timeout,S32 mode,const char *addr);
protected:

private:
};

#endif
