/*************************************************************************
	> File Name: client.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月17日 星期一 15时02分07秒
 ************************************************************************/

#include "custom.h"
#include <event2/thread.h>
#include "customserver.h"
#include <map>
#include "exchange.h"

Custom::Custom(CustomServer *pServer):
    User(pServer,new JsonRpcHandler(this))
{
}

Custom::~Custom()
{
}

void Custom::OnReceiveJson(DAS_RESULT status, const std::string &method, const Json::Value &result)
{
    S32 msgId = -1;
    {
        AutoLock autolock(m_ireqLock);
        std::map<S32,std::string&>::iterator iter;
        for(iter = m_reqList.begin();iter != m_reqList.end();iter++) {
            std::string &method_ = iter->second;
            if (method_ == method) {
                msgId = iter->first;
                m_reqList.erase(iter);
                break;
            }
        }
    }while(0);

    m_pProtocolHandler->ReturnAsyncResult(status,msgId,result);
}

DAS_RESULT Custom::requestAcquisitor(int waitId,const Json::Value &params, Json::Value &result)
{
    std::string strId = params["gatewayId"].asString();
    std::string second_method = params["method"].asString();
    Json::Value second_params = params["params"];

    {
        AutoLock autolock(m_ireqLock);
        if (m_reqList.find(waitId) != m_reqList.end()) {
            return DAS_COLLISION;
        }
        m_reqList.insert(std::pair<S32,std::string&>(waitId,second_method));
    }while(0);

    if (-1 == ((CustomServer*)server())->Ex()->requestAcquisitor(this,strId,second_method,second_params))
        return DAS_DEVICENOTEXIST;

    return DAS_RESULT_MAX;
}

DAS_RESULT Custom::getAcquisitorList(const Json::Value &params, Json::Value &result)
{
    return ((CustomServer*)server())->Ex()->getAcquisitorList(params,result);
}

S32 Custom::SendJsonRpc(tJsonRpcInterface _interface,const std::string &method, const Json::Value &params)
{
    return m_pProtocolHandler->SendJsonRpc(_interface,method,params);
}
