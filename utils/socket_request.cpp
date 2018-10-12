/*************************************************************************
	> File Name: socket_request.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年07月18日 星期三 14时36分48秒
 ************************************************************************/
#include "socket_request.h"
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <vector>
#include <errno.h>
#include <logger.h>
#include <json/writer.h>
#include <json/reader.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <string>

using namespace std;

SocketRequest::SocketRequest(int port, const string &ipAddr, int timeout):
    m_ipAddr(ipAddr),
    m_iPort(port),
    m_iTimeout(timeout){

}

Json::Value SocketRequest::scriptRequest(const string &script,int mode )
{
    Json::Value params;
    params["mode"] = mode;
    params["script"] = script;
    return sendRequest("execute_script",params);
}

Json::Value SocketRequest::sendRequest(const string &method,const Json::Value &params)
{
    static int id = 1;
    Json::Value root;
    root["id"] = (id++)%10000;
    root["jsonrpc"] = "2.0";
    root["method"] = method;
    root["params"] = params;

    Json::StreamWriterBuilder jsbuilder;
    jsbuilder["commentStyle"] = "None";
    jsbuilder["indentation"] = "";  // or whatever you like
    Json::StreamWriter *writer = jsbuilder.newStreamWriter();
    std::ostringstream osout;
    writer->write(root,&osout);
    std::string json_file = osout.str();

    string retstr;
    delete writer;

    if (!sendRequest(json_file,retstr))
        return Json::nullValue;
    Json::Value result;
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    Json::CharReader *reader = builder.newCharReader();
    std::string err;
    const char *pStr = retstr.c_str();
    int  len = retstr.length();
    if (!reader->parse(pStr,pStr+len,&result,&err)) {
        LOG_ERR("error parse,%s,%s",retstr.c_str(),err.c_str());
        delete reader;
        return Json::nullValue;
    }
    delete reader;
    if (!result.isMember("result")) {
        LOG_ERR("json format error,%s",retstr.c_str());
        return Json::nullValue;
    }
    return result["result"];
}

void* SocketRequest::request_callback(void *arg) {
    req_param *req = (req_param*)arg;
    int fd = req->fd;

    int can;
    while((can = req->handle->canRead(fd))>0) {

        char buffer[64] = {0};
        int length = recv(fd, buffer, sizeof(buffer)-1, 0);
        if (length>0){
            req->buffer.append(buffer,length);
            req->ret = true;
        }
        else {
            if (errno == 0) {
                break;
            }
            else if(errno == EINTR ) {
                LOG_ERR("%d,%s break",errno,strerror(errno));
                req->ret = false;
                break;
            }
            else if (errno == EAGAIN) {

                continue;
            }
            else {
                LOG_ERR("%d,%s break",errno,strerror(errno));
                req->ret = false;
                break;
            }
        }
    }

    return NULL;
}

bool SocketRequest::sendRequest(const string &send,string &receive) {
    int fd = open_server(m_ipAddr.c_str(),m_iPort);

    if (fd == -1) {
        return false;
    }
    if (!sendData(fd,send.c_str(),send.length())) {
        close_server(fd);
        return false;
    }
    pthread_t pt;

    req_param req;
    req.handle = this;
    req.fd = fd;

    pthread_create(&pt,NULL,&request_callback,&req);
    pthread_join(pt,NULL);
    close_server(fd);
    if (req.ret)
        receive = req.buffer;
    return req.ret;
}

int SocketRequest::open_server(const string &ipAddr,int port) {
    int socketfd;
    struct sockaddr_in remote_addr;

    memset(&remote_addr,0,sizeof(remote_addr)); //数据初始化--清零
    remote_addr.sin_family=AF_INET; //设置为IP通信
    remote_addr.sin_addr.s_addr=inet_addr(ipAddr.c_str());//服务器IP地址
    remote_addr.sin_port=htons(port); //服务器端口号

    /* 创建客户端套接字--IPv4协议，面向连接通信，TCP协议 */
    if((socketfd = socket(PF_INET,SOCK_STREAM,0))<0) {
        return false;
    }

    /* 将套接字绑定到服务器的网络地址上 */
    if(connect(socketfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0)
    {
        close(socketfd);
        return -1;
    }

    fcntl(socketfd,F_SETFL,O_NONBLOCK);
    return socketfd;
}

void SocketRequest::close_server(int fd) {
    close(fd);
}

bool SocketRequest::sendData(int fd,const char *pData, int iLength)
{
    int total = 0;
    do {
          int iSent = send(fd,
                             (const char *)pData + total,
                             iLength - total, 0);
          if (-1 == iSent) {
            return false;
          }
          total += iSent;
    } while (total < iLength);
    if (total < iLength)
        return false;
    return true;
}

int SocketRequest::canRead(int fd)
{
    fd_set fset;
    int    ret;

    FD_ZERO(&fset);
    FD_SET(fd,&fset);

    timeval time;
    time.tv_sec=m_iTimeout;
    time.tv_usec=0;
    ret = select(fd+1,&fset,NULL,NULL,&time);
    if ( ret < 0){
        LOG_ERR("socket closed");
        return ret;
    }
    else if(ret == 0) {
        LOG_ERR("timeout");
        return ret;
    }
    if (FD_ISSET(fd,&fset)){
        return ret;
    }
    return 0;
}

