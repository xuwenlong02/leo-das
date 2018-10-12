/*************************************************************************
    > File Name: localtask.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月23日 星期二 09时00分46秒
 ************************************************************************/

#ifndef __LOCALTASK_H__
#define __LOCALTASK_H__

#include <defines.h>
#include <utils/lock.h>
#include "connect.h"
#include "../transport.h"
#include <vector>
#include <utils/shared_ptr.h>

using namespace std;

class LocalTask:public DevTransportInterface
{
public:
    LocalTask();
    virtual ~LocalTask();

    void ExitTask();

    void RegisterInterface(ModuleInterface *module);

    /*
     * 增加连接
     */
    tConnection GetConnection(const transmgr_t &transaddr);

    S32 Count();

private:
    vector<tConnection>             m_listConnect;
    Lock                            m_lockConnect;
    bool                            m_bExit;
};

#endif

