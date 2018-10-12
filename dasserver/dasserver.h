/*************************************************************************
	> File Name: dasserver.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月17日 星期一 14时00分10秒
 ************************************************************************/

#ifndef __DASSERVER_H__
#define __DASSERVER_H__
#include <event.h>
#include "client.h"
#include <vector>
#include <utils/shared_ptr.h>
#include <transport/remote/server/server.h>
#include <transport/remote/server/user.h>

class Exchange;

using namespace std;

class DasServer:public Server
{
public:
    DasServer(Exchange *ex);
    virtual ~DasServer();

    virtual tUser CreateUser();

    Exchange *Ex();

    DAS_RESULT getAcquisitorList(
            const Json::Value &params,
            Json::Value &result);

    Client *FindClient(const std::string &strId);
    virtual bool Compare(tUser user, const void *arg);
private:
    Exchange *m_pEx;
};

#endif
