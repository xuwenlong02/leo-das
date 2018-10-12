/*************************************************************************
    > File Name: localtask.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月23日 星期二 09时00分36秒
 ************************************************************************/
#include "localtask.h"
#include <json/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace std;

LocalTask::LocalTask():
    DevTransportInterface(),
    m_lockConnect(),
    m_bExit(false)
{
}

LocalTask::~LocalTask()
{
    m_bExit = true;
    AutoLock autolock(m_lockConnect);
    m_listConnect.clear();
}

void LocalTask::ExitTask()
{
    m_bExit = true;
    AutoLock autolock(m_lockConnect);
    m_listConnect.clear();
}

void LocalTask::RegisterInterface(ModuleInterface *module)
{
    module->RegisterModule((DevTransportInterface*)this);
    LOG_DEBUG(" local %p",this);
}

tConnection LocalTask::GetConnection(const transmgr_t &transaddr)
{
    LOG_TRACE();
#if 1
    AutoLock autolock(m_lockConnect);
    if (m_bExit)
        return NULL;
    LOG_DEBUG("size = %d",m_listConnect.size());
    for (int i = m_listConnect.size()-1;i >= 0 ;i--){
        tConnection  connect = m_listConnect.at(i);
        if (!connect.valid()) {
            m_listConnect.erase(m_listConnect.begin()+i);
            continue;
        }
        if (connect->IsAddressEqual(transaddr)){
            return connect;
        }
    }

    Connection *newconn = LocalConnect::GetConnection(transaddr);

    tConnection tnew = newconn;
    if (tnew.valid())
        m_listConnect.push_back(tnew);
    return tnew;
#endif
}

S32 LocalTask::Count()
{
    return m_listConnect.size();
}


