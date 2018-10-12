/*************************************************************************
	> File Name: client.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月17日 星期一 15时02分07秒
 ************************************************************************/
#include "user.h"
#include <event2/thread.h>
#include <map>

User::User(Server *server, ProtocolHandler *pHandler):
    m_ifd(-1),
    m_pbase(NULL),
    m_pServer(server),
    m_pProtocolHandler(pHandler),
    m_pThread(threads::CreateThread("user",this))
{
    pHandler->SetTransport(this);
}

User::~User()
{
    stopWork();

    if (m_pbase) {
        event_base_free(m_pbase);
    }
    delete m_pProtocolHandler;
    threads::DeleteThread(m_pThread);
}

void User::OnReceiveJson(DAS_RESULT status, const std::string &method, const Json::Value &result)
{
}

void User::threadMain()
{
    m_pbase = event_base_new();
    evthread_make_base_notifiable(m_pbase);
    m_bev = bufferevent_socket_new(
                m_pbase, m_ifd,
                BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);
    if (m_bev == NULL) {
        LOG_ERR("bufferevent create socket failed");

        return;
    }
    bufferevent_setcb(m_bev, DoRead, NULL, DoError, (void *)this);
    bufferevent_setwatermark(m_bev, EV_READ, 0, MAX_RECV_SIZE);
    bufferevent_enable(m_bev, EV_READ);

    evutil_make_socket_nonblocking(m_ifd);

    bufferevent_setfd(m_bev, m_ifd);
    bufferevent_enable(m_bev, EV_READ);

    //阻塞
    event_base_dispatch(m_pbase);
    LOG_INFO("pWork idx = %p Exit", this);
}

bool User::startWork(int fd,sockaddr *addr)
{
    m_ifd = fd,
    m_iaddr = *addr;
    m_pThread->start();
    return true;
}

void User::stopWork()
{
    LOG_TRACE();
    CloseSession();
    if (m_pbase) {
        event_base_loopbreak(m_pbase);
    }
    m_pProtocolHandler->StopTimer();
    m_pThread->join();
}

void User::OnReceiveData(void *data, S32 bytes)
{
    m_pProtocolHandler->OnReceiveData(data,bytes);
    m_pProtocolHandler->RpcParse();
}

BOOL User::SendData(const char *data, S32 bytes)
{
    return 0==bufferevent_write(m_bev,data,bytes);
}

BOOL User::IsTaskExit()
{
    return !m_pThread->is_running();
}

void User::DoRead(bufferevent *bev, void *ctx)
{
    User *pWork = (User *)ctx;
    struct evbuffer* input = bufferevent_get_input(bev);
    size_t length = evbuffer_get_length(input);
    char *buf = (char*)malloc(length+1);
    evbuffer_remove(input, buf, length);
    buf[length] = 0;
    LOG_DEBUG("recv:%d %s",length,buf);
    pWork->OnReceiveData(buf,length);
    free(buf);
}

void User::DoError(bufferevent *bev, short error, void *ctx)
{
    User* pWork = (User*)ctx;
    if (error & EVBUFFER_TIMEOUT )
    {
//        pWork->CloseSession();
        LOG_ERR("pWork idx = %p EVBUFFER_TIMEOUT", pWork);
    }
    else if (error & EVBUFFER_EOF )
    {
        pWork->CloseSession();
        LOG_INFO("pWork idx = %p EVBUFFER_EOF", pWork);
    }
    else if (error & EVBUFFER_ERROR)
    {
        LOG_ERR("pWork idx = %p EVBUFFER_ERROR", pWork);
        pWork->CloseSession();
    }

}

void User::CloseSession()
{
    if (m_bev) {
//        bufferevent_disable(m_bev,EV_READ);
//        bufferevent_enable(m_bev,EV_READ | EV_WRITE);
        bufferevent_free(m_bev);
        evutil_closesocket(m_ifd);
        m_bev = NULL;
    }
}

Server *User::server()
{
    return m_pServer;
}
