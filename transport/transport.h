/*************************************************************************
	> File Name: transport.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月22日 星期一 15时43分21秒
 ************************************************************************/
#ifndef __TRANSPORT_H__
#define __TRANSPORT_H__

#include <defines.h>
#include <json/json.h>
#include "globalmsg.h"
#include "local/htcomhex.h"
#include <utils/shared_ptr.h>
#include <utils/lock.h>
#include <string.h>
#include <utils/custom.h>
#include "remote/jsonrpcinterface.h"

/*
 * socket address
 */
typedef struct{
    S8  ip_addr[MAX_NETADDR_LEN];/* such as: 192.168.0.1 */
    U16 port;
}socketinfo_t;
typedef socketinfo_t *psocketinfo_t;

/*
 * 串口地址
 */
typedef struct{
    S8         com[MAX_SERIALADDR_LEN];
    S32        baudrate;		           //baudrate
    S8         databit;		               //data bits, 5, 6, 7, 8
    S8	       parity;		               //parity 0: none, 1: odd, 2: even
    S8	       stopbit;		               //stop bits, 1, 2
}serialinfo_t;
typedef serialinfo_t *pserialinfo_t;

/***********************************
 * transport mode
 * 传输方式
 **********************************/
typedef enum{
    TM_TCP=0,
    TM_SEARIAL,
    TM_MAX
}TM_MODE;

/***********************************
 * transport
 * 传输管理
 ***********************************/
class transmgr{
public:
    transmgr(){
        trans_mode = TM_MAX;
        timeout=0;
        bzero(&trans_addr,sizeof(trans_addr));
    }
    BOOL empty() {
        return trans_mode==TM_MAX;
    }

    BOOL operator==(const transmgr& trans) {
        if (trans_mode != trans.trans_mode)
            return FALSE;
        if (trans_mode == TM_SEARIAL) {
            if (!strcmp(trans_addr.portaddr.com,trans.trans_addr.portaddr.com))
                return TRUE;
        }
        else if(trans_mode == TM_TCP ){
            if (!strcmp(trans.trans_addr.socketaddr.ip_addr,trans_addr.socketaddr.ip_addr) &&
                    trans.trans_addr.socketaddr.port == trans_addr.socketaddr.port)
                return TRUE;
        }
        return FALSE;
    }

    TM_MODE trans_mode;
    union{
        serialinfo_t   portaddr;
        socketinfo_t   socketaddr;
    }trans_addr;
    S32     timeout;
};
typedef class transmgr transmgr_t;
typedef class transmgr *ptransmgr_t;

typedef BOOL (*Transport_Callback)(U8 recv[],S32 length);
class AcquireInterface
{
public:
    virtual BOOL AcquireData(U8 buffer[],S32 len,void *pri) = 0;
};
/*
 * 本地连接接口
 */
class Connection:public utils::RefrenceObj
{
public:
    virtual ~Connection() {}
    virtual void SetAddr(const transmgr_t &transaddr,BOOL force = FALSE) = 0;
    virtual BOOL IsConnected() = 0;
    virtual BOOL IsAddressEqual(const transmgr_t &transaddr) = 0;
    virtual BOOL LockConnection(const transmgr_t& transaddr) = 0;
    virtual void UnlockConnection() = 0;
    virtual BOOL Test(const U8 cmd[],S32 byte)=0;
    virtual BOOL AtomGetInstruction(const transmgr_t &trans,const U8 cmd[], S32 byte, AcquireInterface *iface,void *pri) = 0;
    virtual BOOL AtomGetInstruction(const transmgr_t &trans,const U8 cmd[], S32 byte, U8 recv[],S32 &length) = 0;
    virtual BOOL SendInstruction(const U8 cmd[],S32 byte) = 0;
    virtual BOOL ReceiveData(U8 buffer[],S32 &byte) = 0 ;
};
typedef utils::SharedPtr<Connection> tConnection;

class ConnectorAutoLock {
public:
    ConnectorAutoLock(Connection *tc,const transmgr_t& trans):
        m_tc(tc) {
        if (m_tc)
            m_tc->LockConnection(trans);
    }

    virtual ~ConnectorAutoLock() {
        if (m_tc)
            m_tc->UnlockConnection();
    }

    Connection* connector() {
        return m_tc;
    }

    inline BOOL IsConnected() {
        return (m_tc && m_tc->IsConnected());
    }
private:
    Connection* m_tc;
};

class DevTransportInterface
{
public:
    /*
     * 增加连接
     */
    virtual tConnection GetConnection(const transmgr_t &transaddr) = 0;
    virtual S32 Count() = 0;
};

/*
 * 传输控制接口
 */
class TransportInterface
{
public:
    virtual void OnReceiveData(void *data,S32 bytes) = 0;
    virtual BOOL SendData(const char *data,S32 bytes) = 0;
    virtual void ChangeStatus(bool connect) {UNUSED(connect);}
    virtual void OnStatus(bool connect) {UNUSED(connect);}
};

/*
 * 远程协议接口
 */
class ProtocolInterface
{
public:
    virtual ~ProtocolInterface(){}
    virtual void OnResult(int msgId,Json::Value &result){UNUSED(msgId);UNUSED(result);}
    virtual void OnRequest(int msgId,Json::Value &request){UNUSED(msgId);UNUSED(request);}
    virtual void SendError(int msgId, const std::string& method,bool result, const char *error){UNUSED(msgId);UNUSED(result);UNUSED(error);}
    virtual BOOL SendRequest(const std::string &method, const Json::Value &params){UNUSED(method);UNUSED(params);}
    virtual void SendResult(int msgId,const std::string &method,const Json::Value &result){UNUSED(msgId);UNUSED(result);}
};

class RPCTransportInterface;
class ModuleInterface
{
public:
    virtual ~ModuleInterface() {}
    /*
     * 初始化
     */
    virtual void Contruct(BOOL ret) = 0;

    /*
     * 网络重连
     */
    virtual void RedoNetwork(BOOL isconected) = 0;

    /*
     * 远程通讯模块
     */
    virtual void RegisterModule(RPCTransportInterface *interface) = 0;


    /*
     * 远程升级
     */
    virtual DAS_RESULT updatePackets(
            int msgId,
            const Json::Value &params,
            Json::Value &result) {
        UNUSED(msgId);UNUSED(params);UNUSED(result);
        return DAS_INVALID;
    }

    /*
     * 网关设置
     */
    virtual DAS_RESULT getDeviceStatus(
            const Json::Value &params,
            Json::Value& result) {
        UNUSED(params);UNUSED(result);
        return DAS_INVALID;
    }
    virtual DAS_RESULT getAcquireValues(
            const Json::Value &params,
            Json::Value& result) {
        UNUSED(params);UNUSED(result);
        return DAS_INVALID;
    }
    virtual DAS_RESULT setDataCenter(
            const Json::Value &params) {
        UNUSED(params);
        return DAS_INVALID;
    }
    virtual DAS_RESULT getDataCenter(
            Json::Value &result) {
        UNUSED(result);
        return DAS_INVALID;
    }
    virtual DAS_RESULT setAcquisitorInfo(
            const Json::Value &params) {
        UNUSED(params);
        return DAS_INVALID;
    }
    virtual DAS_RESULT getAcquisitorInfo(
            Json::Value &result) {
        UNUSED(result);
        return DAS_INVALID;
    }
    virtual DAS_RESULT setGateway(
            const Json::Value &params) {
        UNUSED(params);
        return DAS_INVALID;
    }
    virtual DAS_RESULT getGateway(
            Json::Value &result) {
        UNUSED(result);
        return DAS_INVALID;
    }

    /*
     * 设备管理
     */
    virtual DAS_RESULT updateDeviceList(
            const Json::Value &params,
            Json::Value& result) {
        UNUSED(params);UNUSED(result);
        return DAS_INVALID;
    }
    virtual DAS_RESULT getDeviceIds(
            const Json::Value &params,
            Json::Value &result) {
        UNUSED(params);UNUSED(result);
        return DAS_INVALID;
    }
    virtual DAS_RESULT getChnList(Json::Value &result) {
        UNUSED(result);
        return DAS_INVALID;
    }
    virtual DAS_RESULT getDeviceList(
            const Json::Value &params,
            Json::Value &result){
        UNUSED(params);UNUSED(result);
        return DAS_INVALID;
    }
    virtual DAS_RESULT getDeviceInfo(
            const Json::Value &params,
            Json::Value &result){
        UNUSED(params);UNUSED(result);
        return DAS_INVALID;
    }
    virtual DAS_RESULT addDevice(
            const Json::Value &params){
        UNUSED(params);
        return DAS_INVALID;
    }
    virtual DAS_RESULT deleteDevice(
            const Json::Value &params){
        UNUSED(params);
        return DAS_INVALID;
    }
    virtual DAS_RESULT modifyDevice(
            const Json::Value &params){
        UNUSED(params);
        return DAS_INVALID;
    }
    virtual DAS_RESULT debugDevice(
            const Json::Value &params,
            Json::Value &result){
        UNUSED(params);UNUSED(result);
        return DAS_INVALID;
    }
    virtual DAS_RESULT indicateDevice(
            const Json::Value &params,
            Json::Value &result){
        UNUSED(params);UNUSED(result);
        return DAS_INVALID;
    }
    virtual DAS_RESULT getAcquireData(
            const Json::Value &params,
            Json::Value &result) {
        UNUSED(params);UNUSED(result);
        return DAS_INVALID;
    }
    virtual DAS_RESULT connectServer(
            const Json::Value &params) {
        UNUSED(params);
        return DAS_INVALID;
    }
    virtual DAS_RESULT clearData(){return DAS_INVALID;}
    virtual DAS_RESULT startAcquire() {return DAS_INVALID;}
    virtual DAS_RESULT stopAcquire() {return DAS_INVALID;}
    /*
     * 本地通讯
     */
    virtual void RegisterModule(DevTransportInterface *interface) = 0;
};

#define MODULE_DECLARE(MODULE) \
    public:\
        virtual void Contruct(BOOL ret);\
        virtual void RedoNetwork(BOOL isconected);\
        virtual void RegisterModule(RPCTransportInterface *interface);\
        virtual void RegisterModule(DevTransportInterface *interface);\
        RPCTransportInterface *RpcInterface();\
        DevTransportInterface *DevInterface();\
    private:\
        LOG_MODULE_TRACE(MODULE);\
        RPCTransportInterface * m_pRPCInterface;\
        DevTransportInterface * m_pDevInterface;

#define MODULE_DEFINE_BEGIIN(MODULE) \
    void MODULE::RegisterModule(RPCTransportInterface *interface)\
    {\
        m_pRPCInterface = interface;\
    } \
    void MODULE::RegisterModule(DevTransportInterface *interface)\
    {\
        m_pDevInterface = interface;\
    }\
    RPCTransportInterface *MODULE::RpcInterface()\
    {\
        return m_pRPCInterface;\
    }\
    \
    DevTransportInterface *MODULE::DevInterface()\
    {\
        return m_pDevInterface;\
    }\
    void MODULE::Contruct(BOOL ret) \
    {

#define MODULE_DEFINE_END \
    }

#define MODULE_NETWORK_BEGIN(MODULE) \
    void MODULE::RedoNetwork(BOOL isconected) \
    {

#define MODULE_NETWORK_END \
    }



class RPCTransportInterface
{
public:
    /*
     * jsonrpc
     */
    virtual S32 SendJsonRpc(
            tJsonRpcInterface _interface,
            const std::string &method,
            const Json::Value &params) = 0;
    /*
     *
     */
    virtual void ReturnAsyncResult(DAS_RESULT status,int waitId,const Json::Value &result) = 0;

    virtual TransportInterface* TransportHandle() = 0;
};

#endif//__TRANSPORT_H__
