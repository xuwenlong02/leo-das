/*************************************************************************
	> File Name: clienthandler.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年10月09日 星期二 09时50分02秒
 ************************************************************************/
#include "clienthandler.h"
#include <transport/remote/client/sclient.h>

ClientHandler::ClientHandler(Sclient *interface):
    ProtocolHandler(true,true,interface){

}

ClientHandler::~ClientHandler()
{
}

DAS_RESULT ClientHandler::OnRequest(
        int msgId,
        const std::string &method,
        const Json::Value &params,
        Json::Value &result)
{
    if (method == "writeData") {

    }
    else if(method == "readData") {

    }
    else {
        return ProtocolHandler::OnRequest(
                    msgId,
                    method,
                    params,
                    result);
    }
}
