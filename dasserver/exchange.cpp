/*************************************************************************
	> File Name: exchange.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月29日 星期六 11时19分00秒
 ************************************************************************/
#include "exchange.h"

Exchange::Exchange():
    m_pClientServer(new DasServer(this)),
    m_pCustomServer(new CustomServer(this))
{
}

Exchange::~Exchange()
{
    StopServer();
    delete m_pClientServer;
    delete m_pCustomServer;
}

void Exchange::StartServer()
{
    m_pClientServer->StartServer(9797);
    m_pCustomServer->StartServer(9292);
}

void Exchange::StopServer()
{
    m_pClientServer->StopServer();
    m_pCustomServer->StopServer();
}

void Exchange::OnReceiveJson(DAS_RESULT status, const string &method, const Json::Value &result)
{
}

S32 Exchange::requestAcquisitor(JsonRpcInterface *_interface,
                                 const string &strId,
                                 const string &method,
                                 const Json::Value &params)
{

    Client* client = m_pClientServer->FindClient(strId);
    if (!client)
        return -1;
    return client->SendJsonRpc(_interface,method,params);
}

DAS_RESULT Exchange::getAcquisitorList(const Json::Value &params, Json::Value &result)
{
    return m_pClientServer->getAcquisitorList(params,result);
}
