/*************************************************************************
	> File Name: devices.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年04月12日 星期四 15时01分24秒
 ************************************************************************/

#ifndef __DEVICESSQL_H__
#define __DEVICESSQL_H__

#include <string>
#include "dbms.h"

using namespace std;

class devices:public dbms
{
public:
    devices(int id);
    virtual ~devices();

    BOOL inorup(const string& name,
                const string &devid,
                S32 model,
                S32 type);

    BOOL insert(const string& name,
                const string &devid,
                S32 model,
                S32 type);
    BOOL update(const string& name,
                const string &devid,
                S32 model,
                S32 type);
protected:
private:
};

#endif
