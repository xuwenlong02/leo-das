/*************************************************************************
	> File Name: models.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年04月17日 星期二 13时41分41秒
 ************************************************************************/
#ifndef __MODELS_H__
#define __MODELS_H__

#include "dbms.h"
#include <defines.h>

class models:public dbms
{
public:
    models(S32 id);

    BOOL inorup(const string& protocol);
    BOOL insert(const string& protocol);
    BOOL update(const string& protocol);
    BOOL updateName(const string& name);
};


#endif
