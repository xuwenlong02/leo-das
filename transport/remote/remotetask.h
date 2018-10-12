/*************************************************************************
	> File Name: remotetask.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月23日 星期二 09时00分46秒
 ************************************************************************/

#ifndef __REMOTETASK_H__
#define __REMOTETASK_H__

#include <defines.h>
#include <utils/threads/thread.h>
#include "connect.h"
#include "../transport.h"
#include "protocol/protocolhandler.h"
#include "client/sclient.h"

class RemoteTask:public TimerTask,public Sclient
{
public:
    RemoteTask();
    virtual ~RemoteTask();

    virtual void RunTimer();

    virtual void ChangeStatus(bool connect);
    virtual void OnStatus(bool connect);
    virtual void StartTask();
    virtual BOOL IsTaskExit();
    virtual void ExitTask();

    RPCTransportInterface *Interface();
    void RegisterInterface(ModuleInterface *modules,S32 count);

private:
    void stateChanged(BOOL connected);
private:
    LOG_MODULE_TRACE(RemoteTask);
    Timer            *m_pTimerRead;
//    RemoteConnect    *m_pRemoteConnect;
    BOOL              m_bIsExit;
//    ProtocolHandler  *m_pProtocol;
    Lock              m_iStateLock;
};

#endif

