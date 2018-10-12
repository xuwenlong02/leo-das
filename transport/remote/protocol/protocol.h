/*************************************************************************
	> File Name: protocol.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月23日 星期二 16时33分29秒
 ************************************************************************/

#include<stdio.h>
#include "jsonbuffer.h"
#include "../../transport.h"
#include <logger.h>
#include <utils/timer.h>

class RPCProtocol:public ProtocolInterface
{
public:
    RPCProtocol(TransportInterface *interface);
    virtual ~RPCProtocol();
    void SetTransport(TransportInterface *interface);

    virtual void ClearRecvData();
    virtual void RpcParse();
    void OnReceiveData(void *data,int length);
    void OnMessage(Json::Value &jsonObj);

    virtual void SendError(int msgId,const std::string& method, bool result, const char *error);
    virtual BOOL SendRequest(int msgId,const std::string& method, const Json::Value &params);
    virtual void SendResult(int msgId,const std::string& method, const Json::Value &result);

protected:

    BOOL sendJson(Json::Value &data);
private:
    LOG_MODULE_TRACE(RPCProtocol);
    JsonBuffer m_jsonBuffer;

protected:
    TransportInterface *m_pTransport;
};
