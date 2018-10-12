/*************************************************************************
	> File Name: transport_manager.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月17日 星期三 11时19分22秒
 ************************************************************************/
#ifndef __TRANSPORT_MANAGER_H__
#define __TRANSPORT_MANAGER_H__

#include <utils/threads/thread.h>
#include <vector>
#include "transport.h"
#include "remote/remotetask.h"
#include "local/localtask.h"
//#include "httpserver/httpserver.h"
#include "remote/server/server.h"

/***********************************
 * transport functions
 * 传输函数
 * 参数：
 * return:1->true,0->false
 ***********************************/
typedef int (*transport_result_callback)();

class TransportManager
{
public:
    TransportManager();
    virtual ~TransportManager();

    void SetupRemoteTask();
    void SetupLocalTask();

    void ExitTask();
    void RegisterModules(ModuleInterface * module);
private:
    RemoteTask *                  m_pRemoteTask;
    LocalTask *                   m_pLocalTask;
#ifdef HTTP_SERVER
    Server *                  m_pServer;
#endif
};

#endif
