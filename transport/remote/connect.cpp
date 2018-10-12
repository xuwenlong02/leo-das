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
#include <errno.h>

#include <logger.h>

using namespace std;

RemoteConnect::RemoteConnect(TransportInterface *interface):
    m_iBufSize(MAX_RECV_SIZE),
    m_iSockfd(-1),
    m_bIsConnected(FALSE),
    m_iConnLock(TRUE),
    m_pTransportInterface(interface)
{
    memset(&m_iSockInfo,0,sizeof(m_iSockInfo));
}

RemoteConnect::~RemoteConnect()
{
//    m_iWriteCondition.NotifyOne();
    CloseConnection();
}

BOOL RemoteConnect::SetupConnection(socketinfo_t &si)
{
    m_iSockInfo = si;
    return TRUE;
//    return ReConnect();
}

BOOL RemoteConnect::ReConnect()
{
    struct sockaddr_in remote_addr;

    AutoLock lock(m_iConnLock);

    if (m_bIsConnected && m_iSockfd > 0)
        return m_bIsConnected;
    m_bIsConnected = FALSE;
    if (m_iSockfd>0)
    {
//        shutdown(m_iSockfd,SHUT_RDWR);
        close(m_iSockfd);
        m_iSockfd = -1;
    }
    LOG_INFO("socket(%s:%d) connect begin",m_iSockInfo.ip_addr,m_iSockInfo.port);

    memset(&remote_addr,0,sizeof(remote_addr)); //数据初始化--清零
    remote_addr.sin_family=AF_INET; //设置为IP通信
    remote_addr.sin_addr.s_addr=inet_addr(m_iSockInfo.ip_addr);//服务器IP地址
    remote_addr.sin_port=htons(m_iSockInfo.port); //服务器端口号

    /* 创建客户端套接字--IPv4协议，面向连接通信，TCP协议 */
    if((m_iSockfd = socket(PF_INET,SOCK_STREAM,0))<0) {
        LOG_DEBUG("socket(%s:%d) create failed",m_iSockInfo.ip_addr,m_iSockInfo.port);
        m_bIsConnected = FALSE;
        return FALSE;
    }

    /* 将套接字绑定到服务器的网络地址上 */
    if(connect(m_iSockfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0)
    {
        close(m_iSockfd);
//        shutdown(m_iSockfd,SHUT_RDWR);
        m_iSockfd = -1;
        m_bIsConnected = FALSE;
        LOG_DEBUG("socket(%s:%d) connect failed",m_iSockInfo.ip_addr,m_iSockInfo.port);
        return FALSE;
    }
//    int alive = 1;
//    setsockopt(m_iSockfd,SOL_SOCKET,SO_KEEPALIVE,(void*)&alive,sizeof(alive));
    fcntl(m_iSockfd,F_SETFL,O_NONBLOCK);
    m_bIsConnected = TRUE;

    LOG_INFO("socket(%s:%d) connect success",m_iSockInfo.ip_addr,m_iSockInfo.port);
    return TRUE;
}

void RemoteConnect::CloseConnection()
{
    AutoLock lock(m_iConnLock);
    LOG_ERR("socket error:%d, %s",errno,strerror(errno));
    close(m_iSockfd);
    m_iSockfd = -1;
    m_bIsConnected = FALSE;
}

BOOL RemoteConnect::IsConnected()
{
    AutoLock lock(m_iConnLock);
    return m_bIsConnected;
}

void RemoteConnect::MonitorSocketRead()
{
    S32 ret;

    AutoLock lock(m_iConnLock);
    if (!m_bIsConnected)
        return;
    while((ret = canRead()) > 0) {

        if (!RecvData()) {
            if (errno == 0) {
                return;
            }
            else if(errno == EINTR ) {
                LOG_DEBUG("%d,%s break",errno,strerror(errno));
                CloseConnection();
                break;
            }
            else if (errno == EAGAIN) {
                LOG_DEBUG("%d,%s continue",errno,strerror(errno));
                continue;
            }
            else {
                LOG_ERR("%d,%s break",errno,strerror(errno));
                CloseConnection();
                break;
            }
        }
    }
}

BOOL RemoteConnect::SendData(const char *pData,S32 length)
{
    AutoLock autolock(m_iConnLock);
    if (!m_bIsConnected)
        return FALSE;
    return sendData(pData,length);
}

BOOL RemoteConnect::RecvData() {
    S32 bytes_read = 0;
    char buf[64];

    bytes_read = recv(m_iSockfd,(char*)buf,sizeof(buf),0);
    if (bytes_read > 0) {
        m_pTransportInterface->OnReceiveData(buf, bytes_read);
        return TRUE;
    }
    else {
        return FALSE;
    }
}

S32 RemoteConnect::canSend()
{
    fd_set fd;
    int    ret;

    FD_ZERO(&fd);
    FD_SET(m_iSockfd,&fd);

    timeval tm;
    tm.tv_sec=2;
    tm.tv_usec=0;
    ret = select(m_iSockfd+1,NULL,&fd,NULL,NULL);
    if ( ret < 0){
        CloseConnection();
        return ret;
    }
    else if(ret == 0){
        LOG_DEBUG("socket timeout");
        return ret;
    }
    if (FD_ISSET(m_iSockfd,&fd)){
        return ret;
    }
    return 0;
}

S32 RemoteConnect::canRead()
{
    fd_set fd;
    int    ret;

    FD_ZERO(&fd);
    FD_SET(m_iSockfd,&fd);

    timeval tm;
    tm.tv_sec=2;
    tm.tv_usec=0;
    ret = select(m_iSockfd+1,&fd,NULL,NULL,&tm);

    if ( ret < 0) {
        CloseConnection();
        return ret;
    }
    else if (ret == 0) {
        return 0;
    }
    /* 写的时候不允许读 */
//    AutoLock autolock(m_iSendLock);

    if (FD_ISSET(m_iSockfd,&fd)){
        return ret;
    }
    return 0;
}

BOOL RemoteConnect::sendData(const void *pData, S32 iLength)
{
    S32 bytes_left;
    S32 write_bytes;

    char *ptr = (char *)pData;
    bytes_left = iLength;

    while(bytes_left > 0) {
        if (canSend()<=0)
            return FALSE;
        write_bytes = ::send(m_iSockfd,ptr,bytes_left,0);

        if (write_bytes <= 0) {
            if (errno == EINTR) {
                LOG_DEBUG("retry");
                continue;
            }
            else if (errno == EAGAIN) {
                sleep(1);
                continue;
            }
            else {
                CloseConnection();
                return FALSE;
            }
        }
        bytes_left -= write_bytes;
        ptr += write_bytes;
    }
    return TRUE;
}

