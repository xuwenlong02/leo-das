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
#include <utils/config.h>
#include "htcomhex.h"

using namespace std;

SerialConnect::SerialConnect():
    LocalConnect(){
    m_iComFd = -1;
}

SerialConnect::~SerialConnect()
{
    Close(true);
}

BOOL SerialConnect::ReConnect()
{
    Close();

    portinfo_t portinfo ={
        '0',                            // print prompt after receiving
        m_iTransAddress.trans_addr.portaddr.baudrate,                         // baudrate: 9600
        (char)('0'+m_iTransAddress.trans_addr.portaddr.databit),                            // databit: 8
        '0',                            // debug: off
        '0',                            // echo: off
        '2',                            // flow control: none
        (char)('0'+m_iTransAddress.trans_addr.portaddr.parity),
                                               // parity: none
        (char)('0'+m_iTransAddress.trans_addr.portaddr.stopbit),                            // stopbit: 1
         0                          // reserved
    };
    LOG_DEBUG("com = %s,baudrate = %d, databit = %c, parify = %c, stopbit = %c ,timeout = %d",
              m_iTransAddress.trans_addr.portaddr.com,
              portinfo.baudrate,
              portinfo.databit,
              portinfo.parity,
              portinfo.stopbit,
              m_iTransAddress.timeout);

    m_iComFd = Open();
    if (m_iComFd < 0){
        LOG_ERR("%s open failed",m_iTransAddress.trans_addr.portaddr.com);
        return FALSE;
    }

    PortSet(m_iComFd,&portinfo);
    return TRUE;
}

void SerialConnect::ClearWriteBuffer()
{
    FlushBuffer(m_iComFd,0);
}

void SerialConnect::ClearReadBuffer()
{
    FlushBuffer(m_iComFd,1);
}

S32 SerialConnect::Send(const void *__buf, S32 __n)
{
    return PortSend(m_iComFd,(const char *)__buf,__n);
}

S32 SerialConnect::Recv(void *__buf, S32 __n)
{
    return PortRecv(m_iComFd,(char *)__buf,__n);
}

S32 SerialConnect::Open()
{
    string strcom = utils::GetConfigFileStringValue("COM",m_iTransAddress.trans_addr.portaddr.com,m_iTransAddress.trans_addr.portaddr.com);
    LOG_DEBUG("use %s",strcom.c_str());
    return OpenPort(strcom.c_str());
//    return open(m_iTransAddress.trans_addr.portaddr.com, O_RDWR | O_NOCTTY | O_NONBLOCK);
}

void SerialConnect::Close(bool force)
{
    if (m_iComFd!=-1)
        close(m_iComFd);
    m_iComFd = -1;
}

