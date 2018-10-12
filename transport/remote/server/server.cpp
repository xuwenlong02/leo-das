/*************************************************************************
	> File Name: dasserver.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月17日 星期一 14时00分18秒
 ************************************************************************/

#include <iostream>
#include <sys/select.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <event.h>
#include <stdlib.h>
#include <event2/rpc.h>
#include <event2/listener.h>
#include <event2/thread.h>

#include "server.h"
#include <logger.h>

Server::Server():
    m_EvBase(NULL)
    ,m_EvconnListener(NULL),
    m_pThread(threads::CreateThread("server",this))

{
}

Server::~Server()
{
    StopServer();
    threads::DeleteThread(m_pThread);
    m_EvconnListener = NULL;
    m_EvBase = NULL;
}

void Server::threadMain()
{
    evthread_use_pthreads();
    m_EvBase = event_base_new();

//    evthread_make_base_notifiable(m_EvBase);
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(m_iPort);
//    sin.sin_addr = INADDR_ANY;
    m_EvconnListener = evconnlistener_new_bind(
                m_EvBase,
                DoAccept,
                (void*)this,
                LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | LEV_OPT_THREADSAFE,
                -1,
                (struct sockaddr*)&sin, sizeof(sin));
    if (m_EvconnListener == NULL)
    {
        return;
    }

    event_base_dispatch(m_EvBase);
}

bool Server::StartServer(const int port)
{
    m_iPort = port;
    return m_pThread->start();
}

tUser Server::CreateWork()
{
    AutoLock lock(m_iLock);
    for (int i = m_pWorkList.size()-1;i >= 0 ;i--) {
        tUser work = m_pWorkList.at(i);
        if (work->IsTaskExit()) {
//            delete work;
            m_pWorkList.erase(m_pWorkList.begin()+i);
        }
    }

    tUser pWork = CreateUser();
    m_pWorkList.push_back(pWork);
    return pWork;
}

void Server::AcceptWork(int fd,sockaddr *addr)
{
    tUser pWork = CreateWork();

    pWork->startWork(fd,addr);

}

void Server::DoAccept(evconnlistener *listener, int fd, sockaddr *sa, int socklen, void *user_data)
{
    Server* pServer = static_cast<Server *>(user_data);

    sockaddr_in *addr_in = (sockaddr_in*)sa;
    LOG_INFO("new client  %s:%d",inet_ntoa(addr_in->sin_addr),
             addr_in->sin_port);
    pServer->AcceptWork(fd,sa);
}

tUser Server::CreateUser()
{
    return new User(this,new ProtocolHandler(true,false));
}

vector<tUser> Server::FetchAll()
{
    AutoLock lock(m_iLock);
    return m_pWorkList;
}

tUser Server::FindUser(const void *arg)
{
    AutoLock lock(m_iLock);
    for (vector<tUser>::iterator iter = m_pWorkList.begin();
         iter != m_pWorkList.end();iter++) {
        tUser work = *iter;
        if (Compare(work,arg)) {
            return work;
        }
    }
}

bool Server::Compare(tUser user,const void *arg)
{
    return false;
}

void Server::StopServer()
{
    m_iLock.Acquire();
    m_pWorkList.clear();
    m_iLock.Release();

    if (m_EvconnListener)
        evconnlistener_free(m_EvconnListener);
    if (m_EvBase) {
        //        struct timeval delay = { 5, 0 };
        event_base_loopexit(m_EvBase,NULL);
        event_base_free(m_EvBase);
    }
    m_pThread->join();
}
