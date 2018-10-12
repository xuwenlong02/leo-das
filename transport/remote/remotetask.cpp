/*************************************************************************
	> File Name: remotetask.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月23日 星期二 09时00分36秒
 ************************************************************************/
#include "remotetask.h"
#include <json/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <utils/config.h>
#include "protocol/protocolhandler.h"

using namespace std;

RemoteTask::RemoteTask():
    Sclient(new ProtocolHandler(false,true,this)),
    m_pTimerRead(new Timer("RemoteTask",this))
{

}

RemoteTask::~RemoteTask()
{
    ExitTask();
    delete m_pTimerRead;
}

void RemoteTask::RunTimer()
{
    stateChanged(FALSE);
    socketinfo_t si;
    strcpy(si.ip_addr,utils::GetConfigFileStringValue("GATEWAY","center","").c_str());
    si.port = utils::GetConfigFileIntValue("GATEWAY","port",9090);
    startClient(si.ip_addr,si.port);
    stateChanged(FALSE);
    stopClient();
}

void RemoteTask::ChangeStatus(bool connect)
{
    stopClient();
}

void RemoteTask::OnStatus(bool connect)
{
    stateChanged(connect);
}

void RemoteTask::StartTask()
{
    m_pTimerRead->Start(1000,kPeriodic);
}

BOOL RemoteTask::IsTaskExit()
{
    return !m_pTimerRead->IsRunning();
}

void RemoteTask::ExitTask()
{
    stopClient();
    m_pTimerRead->Stop();
}

RPCTransportInterface *RemoteTask::Interface()
{
    return (RPCTransportInterface*)Handler();
}

void RemoteTask::RegisterInterface(ModuleInterface *modules, S32 count)
{
    for (int i = 0;i < count;i++){
        modules->RegisterModule(Interface());
    }
    LOG_DEBUG("remote %p",Interface());
}

void RemoteTask::stateChanged(BOOL connected)
{
    LOG_DEBUG("Network changed:%d",(int)connected);
    for (U32 task = 0;
         task < DAS_TASK_MAX;task++) {
        ModuleInterface *bi = (ModuleInterface*)GetTask((DAS_TASK)task);
        if (bi) {

            bi->RedoNetwork(connected);
        }
    }
}


