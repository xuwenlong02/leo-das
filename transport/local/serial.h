/*************************************************************************
	> File Name: tcpserver.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月22日 星期一 15时27分51秒
 ************************************************************************/
#ifndef __SERIALCONNECT__
#define __SERIALCONNECT__

#include <defines.h>
#include "../transport.h"
#include <utils/lock.h>
#include <utils/conditional_variable.h>
#include <vector>
#include <logger.h>
#include "connect.h"


using namespace std;

class SerialConnect:public LocalConnect
{
public:
    SerialConnect();
    virtual ~SerialConnect();

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
    virtual S32 Handle() {return m_iComFd;}

private:
    LOG_MODULE_TRACE(SerialConnect);

    int             m_iComFd;
};

#endif//__SERIALCONNECT__
