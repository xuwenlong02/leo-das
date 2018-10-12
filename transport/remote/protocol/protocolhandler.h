/*************************************************************************
	> File Name: protocolhandler.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月24日 星期三 17时08分13秒
 ************************************************************************/

#ifndef __PROTOCOLHANDLER_H__
#define __PROTOCOLHANDLER_H__

#include<stdio.h>
#include <utils/timer.h>
#include "../../transport.h"
#include "protocol.h"
#include <vector>
#include <map>

class ProtocolHandler:public RPCProtocol,public RPCTransportInterface,public TimerTask
{
public:
    ProtocolHandler(bool token = FALSE,bool needtimer = true,TransportInterface *interface = NULL);
    virtual ~ProtocolHandler();
    virtual void RunTimer();
    void StopTimer();
    /*
     * 响应消息
     */
    virtual void OnResult(int msgId,Json::Value &result);
    virtual void OnRequest(int msgId,Json::Value &request);
    virtual DAS_RESULT OnRequest(int msgId,
                           const std::string &method,
                           const Json::Value &params, Json::Value &result);

    /*
     * jsonrpc
     */
    virtual S32 SendJsonRpc(
            tJsonRpcInterface _interface,
            const std::string &method,
            const Json::Value &params);

    virtual void ReturnAsyncResult(DAS_RESULT status,int waitId, const Json::Value &result);

    virtual TransportInterface* TransportHandle(){return m_pTransport;}
private:
    int generateId(tJsonRpcInterface _interface, const std::string &method);
    void deleteId(S32 msgId);
    bool findMethod(const std::string &method);
private:
    LOG_MODULE_TRACE(ProtocolHandler);
    class RequestJson
    {
    public:
        RequestJson(const tJsonRpcInterface &_interface,const std::string &_method) {
            interface = _interface;
            method = _method;
            timeout = 0;
//            m_pTimer = new Timer(_method,this);
//            m_pTimer->Start(30000,kSingleShot);
        }

        ~RequestJson() {

        }

        bool IsTimeOut() {
            if (++timeout > 30)
                return true;
            return false;
        }

        /*
         * 定时检测超时请求
         */
        void handleJsonRpc(DAS_RESULT status,const Json::Value &result) {
            LOG_INFO("status = %d,method = %s",status,method.c_str());
            if (interface.valid())
                interface->OnReceiveJson(status,method,result);
        }

        tJsonRpcInterface   interface;
        std::string         method;
//        Timer              *m_pTimer;
        int timeout;
    };

    Timer              *m_pTimer;
    BOOL               m_bToken;
    Lock                m_state_lock;

    S32          m_iMaxId;
    std::map<S32,RequestJson *> m_pRequestJson;
    struct WaitJson {
        WaitJson(S32 msgId_,const std::string& method_,const Json::Value& result_):
            msgId(msgId_),
            method(method_),
            result(result_){

        }
        std::string method;
        Json::Value result;
        S32 msgId;
    };

    std::vector<WaitJson> m_vectorWaitJson;
    Lock                  m_wait_lock;
};

#endif//__PROTOCOLHANDLER_H__
