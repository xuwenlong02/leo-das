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

#include "dasserver.h"
#include <logger.h>

DasServer::DasServer(Exchange *ex):
    Server(),
    m_pEx(ex)
{
}

DasServer::~DasServer()
{
}

tUser DasServer::CreateUser()
{
    return new Client(this);
}

Exchange *DasServer::Ex()
{
    return m_pEx;
}

DAS_RESULT DasServer::getAcquisitorList(
        const Json::Value &params,
        Json::Value &result) {
    vector<tUser> list = FetchAll();

    for (vector<tUser>::iterator iter = list.begin();
         iter != list.end();iter++) {
        tUser user = *iter;
        Client *client =static_cast<Client*>(user.get());
        result.append(client->JsonAcquisitor());
    }
    if (list.empty())
        return DAS_NOINFO;

    return DAS_SUCCESS;
}

Client* DasServer::FindClient(const string &strId)
{
    return static_cast<Client*>(Server::FindUser(&strId).get());
}

bool DasServer::Compare(tUser user,const void *arg)
{
    const std::string& strId = *((std::string*)(arg));
    Client *client =static_cast<Client*>(user.get());
    return client->IsIdEqual(strId);
}
