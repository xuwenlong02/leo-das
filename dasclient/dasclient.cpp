/*************************************************************************
	> File Name: dasserver.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月17日 星期一 14时00分18秒
 ************************************************************************/

#include <iostream>
#include <sys/select.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <event.h>
#include <stdlib.h>
#include <event2/rpc.h>
#include <event2/listener.h>
#include <event2/thread.h>

#include "dasclient.h"
#include "clienthandler.h"
#include <logger.h>
#include <utils/config.h>

DasClient::DasClient():
    Sclient(new ClientHandler(this)),
    m_pTimer(new Timer("dasclient",this))
{
    m_pTimer->Start(5000,kPeriodic);
}

DasClient::~DasClient()
{
    stopClient();
    m_pTimer->Stop();
}

void DasClient::RunTimer()
{
    std::string ipAddr =
            utils::GetConfigFileStringValue("BASE","center","127.0.0.1");
    int port = utils::GetConfigFileIntValue("BASE","port",9797);
    if (!startClient(ipAddr.c_str(),port)) {
        LOG_ERR("connect to %s:%d failed",ipAddr.c_str(),port);
    }
    stopClient();
}

void DasClient::OnStatus(bool connect)
{
    LOG_DEBUG("Network changed:%d",(int)connect);
    for (U32 task = 0;
         task < DAS_TASK_MAX;task++) {
        ModuleInterface *bi = (ModuleInterface*)GetTask((DAS_TASK)task);
        if (bi) {

            bi->RedoNetwork(connect);
        }
    }
}
