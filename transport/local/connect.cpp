/*************************************************************************
	> File Name: tcpserver.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月22日 星期一 15时27分39秒
 ************************************************************************/
#include "connect.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#include <logger.h>
#include "serial.h"
#include "htsocket.h"

//#define  LOCAL_TEST   1

using namespace std;

/*
 * auto lock
 */

#define TRANS_PRINT(s,p,len)  \
{\
    char *info=(char*)malloc(3*len+1);\
    for (int i = 0; i < len; i++){\
    sprintf(&info[3 * i], "%02X ", (p)[i]); \
    }\
    info[3*len]='\0';\
    if (s) \
    LOG_INFO("send:%s\n",info); \
    else\
    LOG_INFO("recv:%s\n",info); \
    free(info);\
}while (0)

LocalConnect::LocalConnect():
    m_bIsConnected(FALSE),
    m_iWriteLock(TRUE)
{
    memset(&m_iTransAddress,0,sizeof(m_iTransAddress));
}

LocalConnect::~LocalConnect()
{
//    CloseConnection();
    m_bIsConnected = FALSE;
}

LocalConnect *LocalConnect::GetConnection(const transmgr_t &transaddr)
{
    LocalConnect *newconn = NULL;
    if (transaddr.trans_mode == TM_SEARIAL)
        newconn = new SerialConnect();
    else if(transaddr.trans_mode == TM_TCP){
        newconn = new SocketConnect();
    }
    else{
        newconn = new SerialConnect();
    }
    newconn->SetAddr(transaddr,TRUE);

    return newconn;
}

void LocalConnect::CloseConnection()
{
    AutoLock auto_lock(m_iWriteLock);
    Close();
    m_bIsConnected = FALSE;
}

BOOL LocalConnect::SetupConnection(transmgr_t &transaddr)
{
    AutoLock auto_lock(m_iWriteLock);
    m_iTransAddress = transaddr;
    return (m_bIsConnected = ReConnect());
}

BOOL LocalConnect::IsConnected()
{

#ifndef LOCAL_TEST
    return m_bIsConnected;
//    AutoLock auto_lock(m_iWriteLock);
//    return (m_bIsConnected = ReConnect());
#else
    return TRUE;
#endif
}

void LocalConnect::SetAddr(const transmgr_t &transaddr,BOOL force)
{
    if (force)
        m_iTransAddress = transaddr;
    else {
        if (IsAddressEqual(transaddr))
            m_iTransAddress = transaddr;
        else{
            LOG_WARN("try to modify transaddress which is not compatible");
        }
    }
}

BOOL LocalConnect::IsAddressEqual(const transmgr_t &transaddr)
{
    return m_iTransAddress == transaddr;
}

BOOL LocalConnect::LockConnection(const transmgr_t& transaddr)
{
    m_iWriteLock.Acquire();
    SetAddr(transaddr);
    return (m_bIsConnected = ReConnect());
}

void LocalConnect::UnlockConnection()
{
    CloseConnection();
    m_iWriteLock.Release();
}

BOOL LocalConnect::autoGetTrans(S32 index,transmgr_t& trans)
{

    int bautrates[] = {9600,2400,600,1200,4800,19200,38400,57600,76800,115200};
    char veries[] = {0,1,2};
    char stopbits[] = {1,2};
    int bautesize = sizeof(bautrates)/sizeof(int);
    int vsize = sizeof(veries)/sizeof(char);
    int stopsize = sizeof(stopbits)/sizeof(char);

    if (index >= bautesize*vsize*stopsize)
        return FALSE;

    trans.trans_mode = TM_SEARIAL;

    trans.trans_addr.portaddr.stopbit = stopbits[index % stopsize];
    index /= stopsize;
    trans.trans_addr.portaddr.parity = veries[index%vsize];
    index /= vsize;

//    strcpy(trans.trans_addr.portaddr.com,coms[index%10]);
//    index /=10;
    trans.trans_addr.portaddr.baudrate = bautrates[index%bautesize];

    trans.trans_addr.portaddr.databit = 8;

    return TRUE;
}

S32 LocalConnect::canRead()
{
    fd_set fd;
    int    ret;

    FD_ZERO(&fd);
    FD_SET(Handle(),&fd);

    timeval tm;
    tm.tv_sec=m_iTransAddress.timeout/1000;
    tm.tv_usec=(m_iTransAddress.timeout%1000)*1000;

    ret = select(Handle()+1,&fd,NULL,NULL,&tm);

    if ( ret < 0){
        LOG_DEBUG("receive error:%d, %s",ret,strerror(errno));
        Close(true);
        m_bIsConnected = FALSE;
        return ret;
    }
    else if(ret == 0){
        return ret;
    }

    if (FD_ISSET(Handle(),&fd)) {
        return ret;
    }
    return 0;
}

BOOL LocalConnect::AtomGetInstruction(const transmgr_t &trans,const U8 cmd[], S32 byte,AcquireInterface *iface,void *pri)
{
    ConnectorAutoLock autoLock(this,trans);

    S32 length = 0;
    U8 recv[MAX_RECV_SIZE];

    if (!autoLock.IsConnected()) {
        LOG_ERR("connect failed or invalid");
        return FALSE;
    }
    usleep(400000);

    TRANS_PRINT(TRUE,cmd,byte);
    if (!SendInstruction(cmd,byte)){
        LOG_ERR("send failed");
        return FALSE;
    }

    if (ReceiveData(recv,length)){
        if (iface->AcquireData(recv,length,pri)){
            TRANS_PRINT(FALSE,recv,length);
            return TRUE;
        }
    }

//    TRANS_PRINT(FALSE,recv,length);
    LOG_ERR("recv faild,length = %d",length);
    return FALSE;

}

BOOL LocalConnect::AtomGetInstruction(const transmgr_t &trans, const U8 cmd[], S32 byte, U8 recv[], S32 &length)
{
    ConnectorAutoLock autoLock(this,trans);

    if (!autoLock.IsConnected()) {
        LOG_ERR("connect failed or invalid");
        return FALSE;
    }

    TRANS_PRINT(TRUE,cmd,byte);
    int test = 0;

    while (test++ < 2) {
        length = 0;

        if (!SendInstruction(cmd,byte)){
            LOG_ERR("send failed");
            return FALSE;
        }

        if (ReceiveData(recv,length)) {
            TRANS_PRINT(FALSE,recv,length);
            return TRUE;
        }
        usleep(500000);
        LOG_DEBUG("test %d time",test);
    }

//    TRANS_PRINT(FALSE,recv,length);
    LOG_ERR("recv faild,%d",length);
    length = 0;
    return FALSE;

}

BOOL LocalConnect::SendInstruction(const U8 cmd[], S32 byte)
{
#ifndef LOCAL_TEST
    S32 total = 0;
    ClearWriteBuffer();
    ClearReadBuffer();
    do {
          S32 iSent = Send((const char *)cmd + total,
                             byte - total);
          if (iSent <= 0) {
              if(errno == EINTR )
                  continue;
              else if ( errno == EAGAIN) {
                  sleep(1);
                  continue;
              }
              else {
                  LOG_ERR("%d,%s",errno,strerror(errno));
                  Close(true);
                  m_bIsConnected = FALSE;
                  return FALSE;
              }
          }
          total += iSent;
    } while (total < byte);
    return (total == byte);
#else
    return TRUE;
#endif
}

BOOL LocalConnect::ReceiveData(U8 buffer[], S32 &byte)
{
#ifndef LOCAL_TEST
    S32 recv = 0;
    S32 ret=0;

    byte = 0;
    while((ret = canRead()) > 0) {

        if (RecvData(&buffer[byte],recv) > 0) {
            byte += recv;
            recv = 0;
        }
        else {
            if (errno == 0) {
                return byte>0;
            }
            else if(errno == EINTR ) {
                Close(true);
                m_bIsConnected = FALSE;
                LOG_DEBUG("%d,%s break",errno,strerror(errno));
                break;
            }
            else if (errno == EAGAIN) {
                LOG_DEBUG("%d,%s continue",errno,strerror(errno));
                continue;
            }
            else {
                Close(true);
                m_bIsConnected = FALSE;
                LOG_ERR("%d,%s break",errno,strerror(errno));
                break;
            }
        }
    }
    return byte>0;
#else

    //U8 testbuffer[] = {0xFE,0xFE,0x68,0x11 ,0x68 ,0x76 ,0x00 ,0x04 ,0x00 ,0x33 ,0x78 ,0x81 ,0x16 ,0x1F
     //                      ,0x90 ,0x01 ,0x00 ,0x01 ,0x00 ,0x00 ,0x2C ,0x00 ,0x00 ,0x00 ,0x00
     //                      ,0x2C ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0xA6 ,0x16};
   // int testbyte = 37;
//    U8 testbuffer[] = {0xFE, 0xFE, 0x68, 0x11, 0x68, 0x76, 0x00, 0x04, 0x00, 0x33, 0x78, 0x83, 0x03, 0x0A, 0x81, 0x01, 0x18, 0x16};
//    int testbyte = 16;
//    U8 testbuffer[] = {0x01, 0x03, 0x02, 0x00, 0x01, 0x79, 0x84};
//    U8 testbuffer[] = {0x01, 0x03,0x04, 0x00, 0x00, 0x00, 0x00, 0xFA, 0x33 };
//    int testbyte = 9;
//    U8 testbuffer[] = {0xFE,0xFE,0x68,0x10,0x14,0x36,0x21,0x06,0x00,0x33,0x78,0x83,0x03,0x0A,0x81,0x01,0xA6,0x16 };
//    int testbyte = 18;
//U8 testbuffer[]={0x89,0x03,0x04,0x43,0x0B,0xF3,0x33,0x92,0x98};
//int testbyte=9;

//    U8 testbuffer[] = {0x68,0x99,0x95,0x04,0x09,0x16,0x20,0x68,0x81,0x06,0x43,0xC3,0xA7,0xAC,0x39,0x33,0x8D,0x16};
//    int testbyte = 18;
//    U8 testbuffer[] = {0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,0xFE,
//                       0x68,0x25,0x95,0x48,0x34,0x31,0x00,0x11,0x11,0x81,0x2E,0x1F,0x90,0x01,0x00,0x00,0x00,0x00,
//                       0x05,0x00,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x17,0x00,0x00,0x00,0x00,0x35,0x00,0x00,
//                       0x00,0x00,0x2C,0x23,0x27,0x00,0x27,0x27,0x00,0x98,0x01,0x00,0x33,0x38,0x16,0x06,0x06,0x18,
//                       0x20,0x00,0x00,0xC8,0x16};
//    U8 testbuffer[] = {0x01,0x03,0x47,0x00,0x00,0x0B,0x01,0x03,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

//    U8 testbuffer[] = {0xB4,0x10,0x0D,0xB8,0x00,0x01,0x99,0x25};
//    U8 testbuffer[] = {0xFE,0xFE,0x68,0x16,0x69,0x74,0x00,0x07,0x00,0x33,0x78,0x84,0x05,0x17,0xA0,0x04,0xAA,0xAA,0xA5,0x16};
    U8 testbuffer[] = {0x01,0x03,0x04,0x00,0x00,0x49,0x6A,0xC0};
    byte =  sizeof(testbuffer)/sizeof(U8);
    memcpy(buffer,testbuffer,byte);

    return TRUE;
#endif
}

BOOL LocalConnect::RecvData(U8 buffer[],S32 &byte) {

    byte = Recv((char *)buffer,8);

    return byte>0;
}

BOOL LocalConnect::Test(const U8 cmd[], S32 byte)
{
    transmgr_t tmp=m_iTransAddress;
    S32 index = 0;
    transmgr_t trans = tmp;

    U8 recv[MAX_RECV_SIZE];
    S32 length = 0;
    while(autoGetTrans(index++,trans)) {
//        SetAddr(trans,true);
        if (AtomGetInstruction(trans,cmd,byte,recv,length)) {
//            SetAddr(tmp,true);
            return TRUE;
        }
    }
    LOG_ERR("all wrong");
//    SetAddr(tmp,true);
    return FALSE;
}

#undef TRANS_PRINT

