/*************************************************************************
	> File Name: remoteservice.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月25日 星期四 11时43分54秒
 ************************************************************************/

#ifndef __REMOTESERVICE_H__
#define __REMOTESERVICE_H__

#include <stdio.h>
#include <transport/transport.h>
#include <logger.h>
#include <utils/timer.h>
#include <string>

using namespace std;

class RemoteService:public ModuleInterface,public TimerTask,public JsonRpcInterface
{
public:
    RemoteService();
    virtual ~RemoteService();
    virtual void OnReceiveJson(DAS_RESULT result,const std::string &method, const Json::Value &json);

    virtual void RunTimer();

    virtual DAS_RESULT updatePackets(int msgId,const Json::Value &params, Json::Value &result);
private:
    void showStatus(S32 stsCode);
    BOOL downPackets(const string &url, const string &ver);
private:
    Timer  *m_pTimer;
    string m_url,m_version;
    S32     m_iUpdateStatus;
    int   m_iWaitedId;
public:
    MODULE_DECLARE(RemoteService)
};

#endif//__REMOTESERVICE_H__
