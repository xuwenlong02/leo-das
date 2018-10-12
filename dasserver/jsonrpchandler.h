/*************************************************************************
    > File Name: jsonrpchandler.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月24日 星期三 17时08分13秒
 ************************************************************************/

#ifndef __JSONRPCHANDLER_H__
#define __JSONRPCHANDLER_H__

#include <stdio.h>
#include <utils/timer.h>
#include <transport/transport.h>
#include <transport/remote/protocol/protocolhandler.h>
#include <vector>
#include <map>
class User;
typedef utils::SharedPtr<User> tUser;

class JsonRpcHandler:public ProtocolHandler
{
public:
    JsonRpcHandler(User *interface);
    virtual ~JsonRpcHandler();

    /*
     * 响应消息
     */
    virtual DAS_RESULT OnRequest(int msgId,
                           const std::string &method,
                           const Json::Value &params, Json::Value &result);
private:

};

#endif//__PROTOCOLHANDLER_H__
