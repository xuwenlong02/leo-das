/*************************************************************************
	> File Name: transport_manager.c
    > Author: xuwenlong
    > Mail: myxuan475@126.com
	> Created Time: 2018年01月16日 星期二 18时05分33秒
 ************************************************************************/

#include<stdio.h>
#include "transport_manager.h"

TransportManager::TransportManager()
    :m_pRemoteTask(NULL)
    ,m_pLocalTask(NULL)
#ifdef HTTP_SERVER
    ,m_pServer(new Server())
#endif
{
#ifdef HTTP_SERVER
    m_pServer->StartServer(4750);
#endif
}

TransportManager::~TransportManager()
{
    if (m_pRemoteTask){
        delete m_pRemoteTask;
    }

    if (m_pLocalTask) {
        delete m_pLocalTask;
    }
#ifdef HTTP_SERVER
    if (m_pServer)
        delete m_pServer;
#endif
}

void TransportManager::SetupRemoteTask()
{
    if (!m_pRemoteTask){
        m_pRemoteTask = new RemoteTask();
    }
    if (m_pRemoteTask && m_pRemoteTask->IsTaskExit()){
        m_pRemoteTask->StartTask();
    }
}

void TransportManager::SetupLocalTask()
{
    if (!m_pLocalTask)
        m_pLocalTask = new LocalTask();
}

void TransportManager::ExitTask()
{
    m_pLocalTask->ExitTask();
    m_pRemoteTask->ExitTask();
#ifdef HTTP_SERVER
    if (m_pServer)
        m_pServer->StopServer();
#endif
}

void TransportManager::RegisterModules(ModuleInterface *module)
{
    /* remote */
    m_pRemoteTask->RegisterInterface(module,1);

    /* local */
//    m_pLocalTask->RegisterInterface(module);
    module->RegisterModule((DevTransportInterface*)m_pLocalTask);
}


