/*************************************************************************
	> File Name: socket_request.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年07月18日 星期三 14时37分08秒
 ************************************************************************/
#ifndef __SOCKET_REQUEST_H__
#define __SOCKET_REQUEST_H__
#include <string>
#include <json/json.h>
#include <malloc.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <memory.h>
#include <vector>

using namespace std;

class SocketRequest
{
public:
    SocketRequest(int port = 4751,
                  const string &ipAddr = "127.0.0.1",
                  int timeout = 5);
    Json::Value scriptRequest(const string &script, int mode = 0);

    Json::Value sendRequest(const string &method,const Json::Value &params);

    class req_param {
    public:
        req_param() {
            ret = false;
        }

        SocketRequest *handle;
        bool ret;
        int fd;
        string buffer;
    };
private:
    int open_server(const string &ipAddr, int port);
    void close_server(int fd);
    int canRead(int fd);
    bool sendData(int fd,const char *pData, int iLength);
    bool sendRequest(const string &send,string &receive);

    static void* request_callback(void *arg);
private:
    const string m_ipAddr;
    int m_iPort;
    int m_iTimeout;
};

#endif
