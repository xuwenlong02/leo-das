/*************************************************************************
	> File Name: protocolhandler.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月24日 星期三 17时08分04秒
 ************************************************************************/

#include<stdio.h>
#include "protocolhandler.h"
#include "../../globalmsg.h"
#include <utils/socket_request.h>

ProtocolHandler::ProtocolHandler(bool token,bool needtimer, TransportInterface *interface):
    RPCProtocol(interface),
    m_iMaxId(-1),
    m_state_lock(TRUE),
    m_wait_lock(TRUE),
    m_pTimer(NULL),
    m_bToken(token){

    if (needtimer) {
        m_pTimer = new Timer("protocolhandler",this);
        m_pTimer->Start(1000,kPeriodic);
    }

}

ProtocolHandler::~ProtocolHandler()
{
    if (m_pTimer) {
        m_pTimer->Stop();
        delete m_pTimer;
    }
    {
        AutoLock auto_lock(m_state_lock);
        for (int index = 0;index <= m_iMaxId ;index++) {
            if (m_pRequestJson.find(index) == m_pRequestJson.end()) {
                continue;
            }
            RequestJson *request = m_pRequestJson[index];
            delete request;
            //                m_pRequestJson[index]=NULL;
            m_pRequestJson.erase(index);
        }
    }while(0);
}

void ProtocolHandler::OnResult(int msgId, Json::Value &result)
{
    LOG_TRACE();
    if (msgId-1000 < 0 || msgId-1000 >= MAX_REQUEST_SIZE){
        LOG_ERR("msgId %d is out of index",msgId);
        return;
    }

    AutoLock auto_lock(m_state_lock);
    if (m_pRequestJson.find(msgId-1000) == m_pRequestJson.end()){
        LOG_ERR("msgId %d not found",msgId);
        return;
    }
    if (RequestJson *request = m_pRequestJson[msgId-1000]){
        if (!request){
            LOG_WARN("%d has been time out",msgId);
            return;
        }
        request->handleJsonRpc(DAS_SUCCESS,result);
        delete request;
        m_pRequestJson.erase(msgId-1000);

        if (msgId-1000 == m_iMaxId)
            m_iMaxId--;
    }
    else{
        LOG_ERR("not useful");
    }
}

void ProtocolHandler::OnRequest(int msgId, Json::Value &request)
{
    LOG_TRACE();

    Json::Value result;
    Json::Value params;
    DAS_RESULT ret = DAS_INVALID;

    std::string method;

    if (request.isMember("method"))
        method = request["method"].asString();

    if (!GetMsgStatus(DAS_REGISTERACQUISITOR) && m_bToken == FALSE) {
        SendError(msgId,method,false,GetError(DAS_INVALID));
        return;
    }

    if (request.isMember("params"))
        params = request["params"];

    ret = OnRequest(msgId,method,params,result);

    if (ret == DAS_SUCCESS) {
//        if (result.isNull())
//            result = true;
        SendResult(msgId,method,result);
    }
    else if (ret == DAS_RESULT_MAX) {
        m_wait_lock.Acquire();
        WaitJson wJson(msgId,method,result);
        m_vectorWaitJson.push_back(wJson);
        m_wait_lock.Release();
    }
    else
        SendError(msgId,method,false,GetError(ret));
}

DAS_RESULT ProtocolHandler::OnRequest(int msgId,const std::string &method, const Json::Value &params, Json::Value &result)
{
#define JSON_METHOD_EQUAL(name) (method==name)

    DAS_RESULT ret = DAS_INVALID;
    if (JSON_METHOD_EQUAL("checkHeartbeat")) {
        result = true;
        ret = DAS_SUCCESS;
    }
    else if(JSON_METHOD_EQUAL("updateDeviceList")) {
        ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_DEVICEMANAGER));
        if (!module) {
            result = SocketRequest(4750).sendRequest(method,params);
            return DAS_SUCCESS;
        }
        ret = module->updateDeviceList(params,result);
    }
    else if(JSON_METHOD_EQUAL("updatePackets")) {
        ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_REMOTESERVICE));
        if (!module) {
            return ret;
        }
        ret = module->updatePackets(msgId,params,result);
    }
    else if (!strncmp(method.c_str(),"get",3) || !strncmp(method.c_str(),"debug",5)) {
        if (JSON_METHOD_EQUAL("getDeviceStatus")) {
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_DEVICEMANAGER));
            if (!module) {
                result = SocketRequest(4750).sendRequest(method,params);
                return DAS_SUCCESS;
            }
            ret = module->getDeviceStatus(
                        params,result);
        }
        else if (JSON_METHOD_EQUAL("getAcquireValues")) {
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_DEVICEMANAGER));
            if (!module) {
                result = SocketRequest(4750).sendRequest(method,params);
                return DAS_SUCCESS;
            }
            ret = module->getAcquireValues(
                        params,result);
        }
        else if (JSON_METHOD_EQUAL("getDataCenter")) {
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_BASE_INFO));
            if (!module) {
                return ret;
            }
            ret = module->getDataCenter(result);
        }
        else if (JSON_METHOD_EQUAL("getAcquisitorInfo")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_BASE_INFO));
            if (!module) {
                return ret;
            }
            ret = module->getAcquisitorInfo(
                        result);
        }
        else if (JSON_METHOD_EQUAL("getGateway")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_BASE_INFO));
            if (!module) {
                return ret;
            }
            ret = module->getGateway(result);
        }
        else if (JSON_METHOD_EQUAL("getDeviceIds")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_DEVICEMANAGER));
            if (!module) {
                result = SocketRequest(4750).sendRequest(method,params);
                return DAS_SUCCESS;
            }
            ret = module->getDeviceIds(
                        params,result);
        }
        else if (JSON_METHOD_EQUAL("getChnList")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_DEVICEMANAGER));
            if (!module) {
                result = SocketRequest(4750).sendRequest(method,params);
                return DAS_SUCCESS;
            }
            ret = module->getChnList(result);
        }
        else if (JSON_METHOD_EQUAL("getDeviceList")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_DEVICEMANAGER));
            if (!module) {
                result = SocketRequest(4750).sendRequest(method,params);
                return DAS_SUCCESS;
            }
            ret = module->getDeviceList(
                        params,result);
        }
        else if (JSON_METHOD_EQUAL("getDeviceInfo")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_DEVICEMANAGER));
            if (!module) {
                result = SocketRequest(4750).sendRequest(method,params);
                return DAS_SUCCESS;
            }
            ret = module->getDeviceInfo(
                        params,result);
        }
        else if (JSON_METHOD_EQUAL("debugDevice")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_DEVICEMANAGER));
            if (!module) {
                result = SocketRequest(4750).sendRequest(method,params);
                return DAS_SUCCESS;
            }
            ret = module->debugDevice(
                        params,result);
        }
        //getAcquireData
        else if (JSON_METHOD_EQUAL("getAcquireData")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_DEVICEMANAGER));
            if (!module) {
                result = SocketRequest(4750).sendRequest(method,params);
                return DAS_SUCCESS;
            }
            ret = module->getAcquireData(
                        params,result);
        }
    }
    else {

        if (JSON_METHOD_EQUAL("setDataCenter")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_BASE_INFO));
            if (!module)
                return ret;
            ret = module->setDataCenter(
                        params);
        }
        else if (JSON_METHOD_EQUAL("setAcquisitorInfo")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_BASE_INFO));
            if (!module)
                return ret;
            ret = module->setAcquisitorInfo(
                        params);
        }
        else if (JSON_METHOD_EQUAL("setGateway")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_BASE_INFO));
            if (!module)
                return ret;
            ret = module->setGateway(
                        params);
        }
        else if (JSON_METHOD_EQUAL("connectServer")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_ACQUISITOR));
            if (!module) {
                result = SocketRequest(4750).sendRequest(method,params);
                return DAS_SUCCESS;
            }
            ret = module->connectServer(params);
        }
        else if (JSON_METHOD_EQUAL("clearData")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_ACQUISITOR));
            if (!module) {
                result = SocketRequest(4750).sendRequest(method,params);
                return DAS_SUCCESS;
            }
            ret = module->clearData();
        }
        else if (JSON_METHOD_EQUAL("startAcquire")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_ACQUISITOR));
            if (!module) {
                result = SocketRequest(4750).sendRequest(method,params);
                return DAS_SUCCESS;
            }
            ret = module->startAcquire();
        }
        else if (JSON_METHOD_EQUAL("stopAcquire")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_ACQUISITOR));
            if (!module) {
                result = SocketRequest(4750).sendRequest(method,params);
                return DAS_SUCCESS;
            }
            ret = module->stopAcquire();
        }
        else if (JSON_METHOD_EQUAL("addDevice")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_DEVICEMANAGER));
            if (!module) {
                result = SocketRequest(4750).sendRequest(method,params);
                return DAS_SUCCESS;
            }
            ret = module->addDevice(
                        params);
        }
        else if (JSON_METHOD_EQUAL("deleteDevice")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_DEVICEMANAGER));
            if (!module) {
                result = SocketRequest(4750).sendRequest(method,params);
                return DAS_SUCCESS;
            }
            ret = module->deleteDevice(
                        params);
        }
        else if (JSON_METHOD_EQUAL("modifyDevice")){
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_DEVICEMANAGER));
            if (!module) {
                result = SocketRequest(4750).sendRequest(method,params);
                return DAS_SUCCESS;
            }
            ret = module->modifyDevice(
                        params);
        }
        else if(JSON_METHOD_EQUAL("indicateDevice")) {
            ModuleInterface *module = ((ModuleInterface*)GetTask(DAS_DEVICEMANAGER));
            if (!module) {
                result = SocketRequest(4750).sendRequest(method,params);
                return DAS_SUCCESS;
            }
            ret = module->indicateDevice(
                        params,result);
        }
    }
    return ret;

#undef JSON_METHOD_EQUAL
}

void ProtocolHandler::RunTimer()
{
    AutoLock auto_lock(m_state_lock);
    int index = 0;
    int max = index;
    for (index = 0;index <= m_iMaxId ;index++) {
        if (m_pRequestJson.find(index) == m_pRequestJson.end()) {
            continue;
        }
        RequestJson *request = m_pRequestJson[index];

        if (request->IsTimeOut()) {
            request->handleJsonRpc(DAS_TIMEOUT,Json::Value());
            delete request;
            //                m_pRequestJson[index]=NULL;
            m_pRequestJson.erase(index);
        }
        else {
            max = index;
        }
    }
    m_iMaxId = max;
}

void ProtocolHandler::StopTimer()
{
    if (m_pTimer)
        m_pTimer->Stop();
}

S32 ProtocolHandler::SendJsonRpc(tJsonRpcInterface _interface, const std::string &method, const Json::Value &params)
{
    if (!m_bToken) {
        if (method != "registerAcquisitor") {
            if (!GetMsgStatus(DAS_REGISTERACQUISITOR))
                return -1;
        }
        else {
            if (findMethod(method))
                return -1;
        }
    }

    int id = generateId(_interface,method);

    if ((id == -1) || (!SendRequest(id,method,params))) {
        deleteId(id);
        return -1;
    }
    return id;
}

void ProtocolHandler::ReturnAsyncResult(DAS_RESULT status,int waitId,const Json::Value &result)
{
    AutoLock lock(m_wait_lock);
    std::vector<WaitJson>::iterator iter;
    for (iter = m_vectorWaitJson.begin();iter != m_vectorWaitJson.end();iter++) {
        WaitJson& wJson = *iter;
        if (wJson.msgId == waitId) {
            if (status == DAS_SUCCESS) {
                SendResult(wJson.msgId,wJson.method,result);
            }
            else
                SendError(wJson.msgId,wJson.method,false,GetError(status));
            m_vectorWaitJson.erase(iter);
            break;
        }
    }
}

int ProtocolHandler::generateId(tJsonRpcInterface _interface,const std::string &method)
{
    AutoLock auto_lock(m_state_lock);

    int index = 0;
    while (m_pRequestJson.find(index)
           != m_pRequestJson.end()) {
        index = (index+1+MAX_REQUEST_SIZE)%MAX_REQUEST_SIZE;
        if (index == 0) {
            return -1;
        }
    }
    if (index>m_iMaxId)
        m_iMaxId = index;
    m_pRequestJson.insert(std::pair<S32,RequestJson*>(index,new RequestJson(_interface,method)));

    return (index+1000);
}

void ProtocolHandler::deleteId(S32 msgId)
{
    AutoLock auto_lock(m_state_lock);

    int index = msgId -1000;
    if (m_pRequestJson.find(index)
           != m_pRequestJson.end()) {
        RequestJson *pReqJson = m_pRequestJson[index];
        LOG_WARN("handle:%s",pReqJson->method.c_str());
        delete pReqJson;
        m_pRequestJson.erase(index);
    }
}

bool ProtocolHandler::findMethod(const std::string &method)
{
    AutoLock auto_lock(m_state_lock);

    int index = 0;
    for (index = 0;index <= m_iMaxId ;index++) {
        if (m_pRequestJson.find(index) == m_pRequestJson.end()) {
            continue;
        }
        RequestJson *request = m_pRequestJson[index];
        if (method == request->method) {
            return true;
        }
    }
    return false;
}
