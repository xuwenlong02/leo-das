/*************************************************************************
	> File Name: expr.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年04月10日 星期二 16时59分10秒
 ************************************************************************/
#ifndef __EXPRS_H__
#define __EXPRS_H__
#include "dbms.h"

class exprs:public dbms
{
public:
    exprs(int id);

    string arithexpr();
    string comment();
};

#endif // __EXPRS_H__
