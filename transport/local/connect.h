/*************************************************************************
    > File Name: connect.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月22日 星期一 15时27分51秒
 ************************************************************************/
#ifndef __LOCALCONNECT__
#define __LOCALCONNECT__

#include <defines.h>
#include <utils/threads/thread.h>
#include "../transport.h"
#include <utils/lock.h>
#include <utils/conditional_variable.h>
#include <vector>
#include <logger.h>


using namespace std;

class LocalConnect:public Connection
{
public:
    LocalConnect();
    virtual ~LocalConnect();

    static LocalConnect* GetConnection(const transmgr_t &transaddr);
    BOOL SetupConnection(transmgr_t &transaddr);
    virtual BOOL ReConnect() = 0;
    void CloseConnection();
    BOOL IsConnected();

    virtual void SetAddr(const transmgr_t &transaddr, BOOL force = FALSE);
    virtual BOOL IsAddressEqual(const transmgr_t &transaddr);

    virtual void ClearWriteBuffer(){}
    virtual void ClearReadBuffer(){}

    virtual BOOL LockConnection(const transmgr_t &transaddr);
    virtual void UnlockConnection();

    virtual BOOL AtomGetInstruction(const transmgr_t &trans,const U8 cmd[],S32 byte,AcquireInterface *iface, void *pri);
     virtual BOOL AtomGetInstruction(const transmgr_t &trans,const U8 cmd[], S32 byte, U8 recv[],S32 &length);
    virtual BOOL SendInstruction(const U8 cmd[],S32 byte);
    virtual BOOL ReceiveData(U8 buffer[],S32 &byte);

    BOOL RecvData(U8 buffer[],S32 &byte);

    /*
     * 发送和接收数据
     */
    virtual S32 Send( __const void *__buf, S32 __n) = 0;
    virtual S32 Recv (void *__buf, S32 __n) = 0;
    virtual S32 Open() = 0;
    virtual void Close(bool force = false) = 0;
    virtual S32 Handle() = 0;
    virtual BOOL Test(const U8 cmd[],S32 byte);
    BOOL autoGetTrans(S32 index, transmgr_t &trans);
private:
    LOG_MODULE_TRACE(LocalConnect);

//    BOOL recvData(U8 buffer[],S32 &byte);
    S32 canRead();

protected:
    transmgr_t          m_iTransAddress;

    Lock                 m_iWriteLock;

    BOOL                 m_bIsConnected;
};

#endif//
