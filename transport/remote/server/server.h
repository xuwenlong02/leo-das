/*************************************************************************
    > File Name: server.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
    > Created Time: 2018年09月29日 星期六 10时00分10秒
 ************************************************************************/

#ifndef __SERVER_H__
#define __SERVER_H__
#include <event.h>
#include "user.h"
#include <vector>
#include <utils/shared_ptr.h>

using namespace std;

class Server:public threads::ThreadDelegate
{
public:
    Server();
    virtual ~Server();

    virtual void threadMain();

    bool  StartServer(const int port);
    void  StopServer();

    virtual tUser CreateWork();
    void AcceptWork(evutil_socket_t fd, sockaddr *addr);
    static void DoAccept(struct evconnlistener *listener,
                         evutil_socket_t fd,
                         struct sockaddr *sa,
                         int socklen,
                         void *user_data);
    virtual tUser CreateUser();

    vector<tUser> FetchAll();

    tUser FindUser(const void *arg);

    virtual bool Compare(tUser user,const void *arg);

private:
    struct event_base* m_EvBase;
    Lock          m_iLock;

    vector<tUser> m_pWorkList;
    struct evconnlistener* m_EvconnListener;
    int m_iPort;
    threads::Thread *m_pThread;
};

#endif
