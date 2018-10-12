/*************************************************************************
	> File Name: client.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月17日 星期一 15时02分16秒
 ************************************************************************/
#ifndef __SERVER_CLIENT_H__
#define __SERVER_CLIENT_H__
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
#include <utils/threads/thread.h>
#include <utils/threads/thread_delegate.h>
#include <defines.h>
#include <transport/transport.h>
#include <transport/remote/server/server.h>
#include <transport/remote/server/user.h>
#include "jsonrpchandler.h"

class DasServer;

class Client:public User
{
public:
    Client(DasServer *pServer);
    virtual ~Client();

    virtual void OnReceiveJson(DAS_RESULT status,
                               const std::string &method,
                               const Json::Value &result);

    DAS_RESULT registerAcquisitor(const Json::Value &params,
                           Json::Value &result);
    S32 SendJsonRpc(JsonRpcInterface *_interface,const std::string &method, const Json::Value &params);

    Json::Value JsonAcquisitor();

    bool IsIdEqual(const std::string &strId);

private:
    std::string m_strId;
    std::string m_version;

};

#endif
