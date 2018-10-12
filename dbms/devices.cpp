/*************************************************************************
	> File Name: devices.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年04月12日 星期四 15时01分12秒
 ************************************************************************/

#include "devices.h"

devices::devices(int id):
    dbms("devices",id) {

}

devices::~devices()
{
}

BOOL devices::inorup(const string &name, const string &devid, S32 model, S32 type)
{
    if (!exist())
        return insert(name,devid,model,type);
    else
        return update(name,devid,model,type);
}

BOOL devices::insert(const string& name,const string &devid,S32 model, S32 type)
{
    char cond[MAX_COND_LEN] = {0};

    sprintf(cond,"%d,'%s','%s',%d,%d",m_iId,devid.c_str(),name.c_str(),model,type);
    return m_sqlite.Insert("id,devId,name,model,type",cond);
}

BOOL devices::update(const string &name, const string &devid, S32 model, S32 type)
{
    char cond[MAX_COND_LEN] = {0};

    sprintf(cond,"devId='%s',name='%s',model=%d,type=%d",devid.c_str(),name.c_str(),model,type);
    return m_sqlite.Update(cond,getcondition().c_str());
}

