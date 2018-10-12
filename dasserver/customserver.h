/*************************************************************************
	> File Name: dasserver.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月17日 星期一 14时00分10秒
 ************************************************************************/

#ifndef __CUSTOMSERVER_H__
#define __CUSTOMSERVER_H__
#include <event.h>
#include "client.h"
#include <vector>
#include <utils/shared_ptr.h>
#include <transport/remote/server/server.h>
#include <transport/remote/server/user.h>

class Exchange;

using namespace std;

class CustomServer:public Server
{
public:
    CustomServer(Exchange *ex);
    virtual ~CustomServer();

    Exchange *Ex();

    virtual tUser CreateUser();

private:
    Exchange *m_pEx;
};

#endif
