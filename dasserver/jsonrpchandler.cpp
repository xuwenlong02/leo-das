/*************************************************************************
	> File Name: protocolhandler.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月24日 星期三 17时08分04秒
 ************************************************************************/

#include<stdio.h>
#include "jsonrpchandler.h"
#include <string>
#include <utils/shellcmd.h>
#include "client.h"
#include "custom.h"
#include "dasserver.h"

using namespace std;

JsonRpcHandler::JsonRpcHandler(User *interface):
    ProtocolHandler(true,true,interface) {
}

JsonRpcHandler::~JsonRpcHandler()
{
}

DAS_RESULT JsonRpcHandler::OnRequest(int msgId,const string &method, const Json::Value &params, Json::Value &result)
{
    if (method == "requestAcquisitor") {
        Custom *client = static_cast<Custom*>(m_pTransport);
        return client->requestAcquisitor(msgId,params,result);
    }
    else if (method == "registerAcquisitor") {
        Client *client = static_cast<Client*>(m_pTransport);
        return client->registerAcquisitor(params,result);
    }
    else if (method == "getAcquisitorList") {
        Custom *client = static_cast<Custom*>(m_pTransport);
        return client->getAcquisitorList(params,result);
    }
    else {
//        return ProtocolHandler::OnRequest(msgId,method,params,result);
    }
}

