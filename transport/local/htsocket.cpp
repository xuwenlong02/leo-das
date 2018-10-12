/*************************************************************************
	> File Name: htsocket.c
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月18日 星期四 14时26分44秒
 ************************************************************************/

#include<stdio.h>
#include <fcntl.h>
#include "htsocket.h"
#include <errno.h>

SocketConnect::SocketConnect():
    LocalConnect(){
    m_iSockFd = -1;
}

SocketConnect::~SocketConnect()
{
    Close(true);
}

BOOL SocketConnect::ReConnect()
{
    if (m_iSockFd > 0)
        return TRUE;
    Close();

    LOG_DEBUG("ip = %s,port = %d, timeout = %d",m_iTransAddress.trans_addr.socketaddr.ip_addr,
              m_iTransAddress.trans_addr.socketaddr.port,
              m_iTransAddress.timeout);

    m_iSockFd = Open();
    if (m_iSockFd < 0){
        LOG_ERR("%s : %d open failed",m_iTransAddress.trans_addr.socketaddr.ip_addr,
                m_iTransAddress.trans_addr.socketaddr.port);
        return FALSE;
    }

    return TRUE;
}

void SocketConnect::ClearWriteBuffer()
{

}

void SocketConnect::ClearReadBuffer()
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(m_iSockFd,&fds);
    struct timeval tm;
    tm.tv_sec = 1;
    tm.tv_usec=0;
    S32 nRet = select(FD_SETSIZE,&fds,NULL,NULL,&tm);
    if (nRet == 0)
        return;
    char *tmp = (char *)malloc(nRet+1);
    recv(m_iSockFd,tmp,nRet,0);
    free(tmp);
}

S32 SocketConnect::Send(const void *__buf, S32 __n)
{
    return send(m_iSockFd,(const char *)__buf,__n,0);
}

S32 SocketConnect::Recv(void *__buf, S32 __n)
{
    return recv(m_iSockFd,__buf,__n,0);
}

S32 SocketConnect::Open()
{
    if (m_iSockFd > 0)
        return m_iSockFd;
    struct sockaddr_in remote_addr;

    memset(&remote_addr,0,sizeof(remote_addr)); //数据初始化--清零
    remote_addr.sin_family=AF_INET; //设置为IP通信
    remote_addr.sin_addr.s_addr=inet_addr(m_iTransAddress.trans_addr.socketaddr.ip_addr);//服务器IP地址
    remote_addr.sin_port=htons(m_iTransAddress.trans_addr.socketaddr.port); //服务器端口号

    /* 创建客户端套接字--IPv4协议，面向连接通信，TCP协议 */
    if((m_iSockFd = socket(PF_INET,SOCK_STREAM,0))<0) {
        return m_iSockFd;
    }

    /* 将套接字绑定到服务器的网络地址上 */
    if(connect(m_iSockFd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0)
    {
        LOG_ERR("%s",strerror(errno));
        m_iSockFd = -1;
        return -1;
    }

    int fl = fcntl(m_iSockFd,F_GETFL,0);
    fcntl(m_iSockFd,F_SETFL,fl|O_NONBLOCK);

    LOG_DEBUG("use:%s:%d",m_iTransAddress.trans_addr.socketaddr.ip_addr,
              m_iTransAddress.trans_addr.socketaddr.port);
    return m_iSockFd;
}

void SocketConnect::Close(bool force)
{
    if (m_iSockFd!=-1) {
        if (force){
            close(m_iSockFd);
            m_iSockFd = -1;
        }
    }

}

