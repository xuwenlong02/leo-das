/*************************************************************************
	> File Name: acqtypes.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年04月10日 星期二 17时00分50秒
 ************************************************************************/

#ifndef __ACQTYPES_H__
#define __ACQTYPES_H__

#include "dbms.h"
#include <defines.h>

class acqtypes:public dbms
{
public:
    acqtypes(int id);
protected:
    virtual string getcondition();
};

#endif
