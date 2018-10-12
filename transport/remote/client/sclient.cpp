/*************************************************************************
	> File Name: sclient.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年10月08日 星期一 15时30分34秒
 ************************************************************************/
#include "sclient.h"
#include <iostream>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <event.h>
#include <logger.h>
#include <event2/thread.h>

Sclient::Sclient(ProtocolHandler *pHandler):
    m_EvBase(NULL),
    m_pProtocolHandler(pHandler),
    m_lock(true){
    pHandler->SetTransport(this);
}

Sclient::~Sclient()
{
    stopClient();
    delete m_pProtocolHandler;
}

bool Sclient::startClient(const char *ipAddr, int port)
{
    AutoLock aulock(m_lock);
    evthread_use_pthreads();
    m_EvBase = event_base_new();
    m_bev = bufferevent_socket_new(
                m_EvBase,-1,
                BEV_OPT_CLOSE_ON_FREE);
//    struct event *ev = event_new(m_EvBase,)
    sockaddr_in addr;
    memset(&addr,0,sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_aton(ipAddr,&addr.sin_addr);
    if (-1 == bufferevent_socket_connect(m_bev,(sockaddr*)&addr,sizeof(addr))) {
        bufferevent_free(m_bev);
        event_base_free(m_EvBase);
        m_bev = NULL;
        m_EvBase = NULL;
        return false;
    }
    bufferevent_setcb(
                m_bev,
                Sclient::server_msg,
                NULL,
                Sclient::server_event,this);
    bufferevent_enable(m_bev,EV_READ|EV_PERSIST);

    {
        AutoUnlock unlock(m_lock);
        event_base_dispatch(m_EvBase);
    }while(0);
    return false;
}

void Sclient::stopClient()
{
    AutoLock aulock(m_lock);
    stopSession();
    if (m_EvBase) {
        //        struct timeval delay = { 5, 0 };
        event_base_loopexit(m_EvBase,NULL);
        event_base_free(m_EvBase);
        m_EvBase = NULL;
    }
}

void Sclient::OnReceiveData(void *data, S32 bytes)
{
    m_pProtocolHandler->OnReceiveData(data,bytes);
    m_pProtocolHandler->RpcParse();
}

BOOL Sclient::SendData(const char *data, S32 bytes)
{
    return (0==bufferevent_write(m_bev,data,bytes));
}

void Sclient::ChangeStatus(bool connect)
{
    stopClient();
}

void Sclient::server_msg(bufferevent *bev, void *arg)
{
    Sclient *pWork = (Sclient *)arg;
    struct evbuffer* input = bufferevent_get_input(bev);
    size_t length = evbuffer_get_length(input);
    char *buf = (char*)malloc(length+1);
    evbuffer_remove(input, buf, length);
    buf[length] = 0;
    LOG_DEBUG("recv:%d %s",length,buf);
    pWork->OnReceiveData(buf,length);
    free(buf);
}

void Sclient::server_event(bufferevent *bev, short ev, void *arg)
{
    static int test = 0;
    Sclient *client = static_cast<Sclient*>(arg);
    if (ev & BEV_EVENT_EOF) {
        client->stopSession();
        client->OnStatus(false);
        LOG_INFO("Connection closed.");
    }
    else if (ev & BEV_EVENT_ERROR) {
        if (test == EVUTIL_SOCKET_ERROR())
            return;
        LOG_ERR("Some other error,%d,%s.",
                EVUTIL_SOCKET_ERROR(),
                evutil_socket_error_to_string(errno));
        test = EVUTIL_SOCKET_ERROR();
    }
    else if (ev & BEV_EVENT_CONNECTED) {
        client->OnStatus(true);
        LOG_INFO("Client has successfully connected.");
    }

        // free event_cmd
        // need struct as event is defined as parameter
}

void Sclient::stopSession()
{
    AutoLock aulock(m_lock);
    if (m_bev) {
        bufferevent_free(m_bev);
        m_bev = NULL;
    }
}
