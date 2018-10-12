/*************************************************************************
	> File Name: exchange.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月29日 星期六 11时19分07秒
 ************************************************************************/
#ifndef __EXCHANGE_H__
#define __EXCHANGE_H__
#include <utils/threads/thread.h>
#include "dasserver.h"
#include "customserver.h"

class Exchange:public JsonRpcInterface
{
public:
    Exchange();
    virtual ~Exchange();

    void StartServer();
    void StopServer();

    virtual void OnReceiveJson(DAS_RESULT status,
                               const std::string &method,
                               const Json::Value &result);
    S32 requestAcquisitor(
            JsonRpcInterface *_interface,
            const string &strId,
            const string &method,
            const Json::Value &params);

    DAS_RESULT getAcquisitorList(
            const Json::Value &params,
            Json::Value &result);

private:
    DasServer *m_pClientServer;
    CustomServer *m_pCustomServer;
};

#endif
