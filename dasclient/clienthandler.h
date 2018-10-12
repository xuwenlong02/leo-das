/*************************************************************************
	> File Name: clienthandler.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年10月09日 星期二 09时50分08秒
 ************************************************************************/
#ifndef __CLIENTHANDLER_H__
#define __CLIENTHANDLER_H__
#include <stdio.h>
#include <transport/transport.h>
#include <transport/remote/protocol/protocolhandler.h>
#include <vector>
#include <map>

class Sclient;

class ClientHandler:public ProtocolHandler
{
public:
    ClientHandler(Sclient *interface);
    virtual ~ClientHandler();

    /*
     * 响应消息
     */
    virtual DAS_RESULT OnRequest(
            int msgId,
            const std::string &method,
            const Json::Value &params,
            Json::Value &result);
private:

};
#endif
