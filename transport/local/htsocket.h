/*************************************************************************
	> File Name: httcp.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月17日 星期三 11时29分28秒
 ************************************************************************/
#ifndef __HTSOCKET_H__
#define __HTSOCKET_H__

#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <defines.h>

#include "../transport.h"
#include <utils/lock.h>
#include <utils/conditional_variable.h>
#include <vector>
#include <logger.h>
#include "connect.h"


class SocketConnect:public LocalConnect
{
public:
    SocketConnect();
    virtual ~SocketConnect();

    virtual BOOL ReConnect();

    virtual void ClearWriteBuffer();
    virtual void ClearReadBuffer();

    /*
     * 发送和接收数据
     */
    virtual S32 Send( __const void *__buf, S32 __n);
    virtual S32 Recv (void *__buf, S32 __n);
    virtual S32 Open();
    virtual void Close(bool force = false);
    virtual S32 Handle(){return m_iSockFd;}

private:
    LOG_MODULE_TRACE(SocketConnect);

    int             m_iSockFd;
};
#endif//
