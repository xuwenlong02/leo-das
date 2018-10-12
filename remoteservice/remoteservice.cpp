/*************************************************************************
	> File Name: remoteservice.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月25日 星期四 11时43分47秒
 ************************************************************************/

#include<stdio.h>
#include "remoteservice.h"
#include <utils/shellcmd.h>
#include <utils/socket_request.h>

MODULE_DEFINE_BEGIIN(RemoteService)
if (ret) {

}
else {
m_pTimer->Stop();
delete m_pTimer;
}

MODULE_DEFINE_END

MODULE_NETWORK_BEGIN(RemoteService)

MODULE_NETWORK_END

RemoteService::RemoteService():
    m_pTimer(new Timer("RemoteService",this))
{
    addReference();
    m_iWaitedId = -1;
    m_iUpdateStatus = 0;
}

RemoteService::~RemoteService()
{
    dropReference();
}

void RemoteService::OnReceiveJson(DAS_RESULT result, const string &method, const Json::Value &json)
{
}

void RemoteService::RunTimer()
{
    LOG_DEBUG("updatePacket %d",m_iUpdateStatus);

    if (m_iUpdateStatus == 1) {
        Json::Value result;
        if (downPackets(m_url,m_version)) {

            result["version"] = m_version;
            result["stsCode"] = 30;
            m_iUpdateStatus = 2;
        }
        else {
            result["version"] = m_version;
            result["stsCode"] = 12;
            m_iUpdateStatus = 0;
        }
        m_pRPCInterface->ReturnAsyncResult(DAS_SUCCESS,m_iWaitedId,result);
        if (m_iUpdateStatus == 2) {
            exit(0);
        }

    }
}

DAS_RESULT RemoteService::updatePackets(int msgId,const Json::Value &params, Json::Value &result)
{
    m_pTimer->Stop();
    m_iWaitedId = msgId;
    if (!params.isMember("url"))
        return DAS_INVALIDPARAMETER;
    if (!params.isMember("version"))
        return DAS_INVALIDPARAMETER;
    m_url = params["url"].asString();
    m_version = params["version"].asString();

    m_iUpdateStatus = 1;
//    m_pTimer->Start(100,kPeriodic);


    m_pTimer->Start(100,kSingleShot);
    return DAS_RESULT_MAX;
}

void RemoteService::showStatus(S32 stsCode)
{
    Json::Value stsJson;
    stsJson["stsCode"]=stsCode;
    stsJson["stsComment"]="";
    stsJson["version"] = m_version;
    m_iUpdateStatus = stsCode;
    m_pRPCInterface->SendJsonRpc(this,"updateStatus",stsJson);
}

BOOL RemoteService::downPackets(const string &url,const string &ver)
{
    char script[256];

    sprintf(script,"%s/script/downpacket.sh %s %s",
            getenv(CONFIG_DIR_ENV),
            url.c_str(),ver.c_str());

    Json::Value result = SocketRequest().scriptRequest(script,1);
    return (result.asInt() == 0);

}
