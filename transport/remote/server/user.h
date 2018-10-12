/*************************************************************************
	> File Name: client.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月17日 星期一 15时02分16秒
 ************************************************************************/
#ifndef __SERVER_USER_H__
#define __SERVER_USER_H__
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
#include <utils/threads/thread.h>
#include <utils/threads/thread_delegate.h>
#include <defines.h>
#include <transport/transport.h>
#include "../protocol/protocolhandler.h"
#include "../jsonrpcinterface.h"

class Server;
class User;
typedef utils::SharedPtr<User> tUser;

class User:public TransportInterface,public JsonRpcInterface,public threads::ThreadDelegate
{
public:
    User(Server* server,ProtocolHandler *pHandler);
    virtual ~User();

    virtual void OnReceiveJson(DAS_RESULT status,
                               const std::string &method,
                               const Json::Value &result);

    virtual void threadMain();
    bool startWork(int fd, sockaddr *addr);
    void stopWork();

    bool isValid() {return (m_pbase != NULL && m_bev != NULL);}
    virtual void OnReceiveData(void *data,S32 bytes);
    virtual BOOL SendData(const char *data,S32 bytes);
    virtual BOOL IsTaskExit();

    static void DoRead(struct bufferevent *bev, void *ctx);
    static void DoError(struct bufferevent *bev, short error, void *ctx);
    void CloseSession();

    Server *server();

private:

    struct event_base*  m_pbase;
    struct bufferevent* m_bev;
    evutil_socket_t     m_ifd;

    threads::Thread    *m_pThread;

protected:
    ProtocolHandler*     m_pProtocolHandler;
    sockaddr             m_iaddr;
    Server*              m_pServer;
};

#endif
