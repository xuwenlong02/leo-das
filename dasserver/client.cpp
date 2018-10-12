/*************************************************************************
	> File Name: client.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月17日 星期一 15时02分07秒
 ************************************************************************/

#include "client.h"
#include <event2/thread.h>
#include "dasserver.h"
#include <map>

Client::Client(DasServer *pServer):
    User(pServer,new JsonRpcHandler(this))
{
}

Client::~Client()
{
}

void Client::OnReceiveJson(DAS_RESULT status, const std::string &method, const Json::Value &result)
{
}

DAS_RESULT Client::registerAcquisitor(const Json::Value &params,Json::Value &result)
{
    m_version = params["version"].asString();
    m_strId = params["gateway_id"].asString();
    result = true;
    return DAS_SUCCESS;
}

S32 Client::SendJsonRpc(JsonRpcInterface *_interface,const std::string &method, const Json::Value &params)
{
    return m_pProtocolHandler->SendJsonRpc(_interface,method,params);
}

Json::Value Client::JsonAcquisitor()
{
    Json::Value acq;
    acq["version"] = m_version;
    acq["gatewayId"] = m_strId;

    sockaddr_in *addr = (sockaddr_in*)&m_iaddr;
    acq["ipaddr"] = inet_ntoa(addr->sin_addr);
    acq["port"] = addr->sin_port;
    return acq;
}

bool Client::IsIdEqual(const std::string &strId)
{
    return (m_strId == strId);
}
