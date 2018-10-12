/*************************************************************************
	> File Name: sclient.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年10月08日 星期一 15时31分45秒
 ************************************************************************/
#ifndef __SCLIENT_H__
#define __SCLIENT_H__
#include <event.h>
#include <vector>
#include <utils/threads/thread.h>
#include <defines.h>
#include <transport/transport.h>
#include "../protocol/protocolhandler.h"
#include "../jsonrpcinterface.h"

using namespace std;

class Sclient:public TransportInterface
{
public:
    Sclient(ProtocolHandler *pHandler);
    virtual ~Sclient();

    bool startClient(const char *ipAddr,int port);
    void stopClient();

    virtual void OnReceiveData(void *data,S32 bytes);
    virtual BOOL SendData(const char *data,S32 bytes);
    virtual void ChangeStatus(bool connect);

    ProtocolHandler *Handler() {return m_pProtocolHandler;}

    static void server_msg(bufferevent *bev,void *arg);
    static void server_event(bufferevent *bev,short ev,void *arg);
private:
    void stopSession();
private:
    struct event_base* m_EvBase;
    struct bufferevent* m_bev;
    ProtocolHandler *m_pProtocolHandler;
    Lock             m_lock;
};

#endif
