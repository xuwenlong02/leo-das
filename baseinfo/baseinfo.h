/*************************************************************************
	> File Name: baseinfo.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月17日 星期三 13时44分06秒
 ************************************************************************/
#ifndef __BASEINFO_H__
#define __BASEINFO_H__

#include "static_config.h"
#include <transport/transport.h>
#include <logger.h>
#include <utils/timer.h>
//gateway_config_t  g_gateway_config;

class BaseInfo:public ModuleInterface,public TimerTask,public JsonRpcInterface
{
public:
    BaseInfo();
    virtual ~BaseInfo();
    virtual void RunTimer();

    BOOL IsGateWayIdEqual(const char *id);
    /*
     * inherid from ModuleInterface
     */
    virtual void OnReceiveJson(DAS_RESULT result,const std::string &method, const Json::Value &json);

    virtual void RegisterAcquisitor();

    virtual DAS_RESULT setDataCenter(const Json::Value &params);
    virtual DAS_RESULT getDataCenter(Json::Value &result);
    virtual DAS_RESULT setAcquisitorInfo(const Json::Value &params);
    virtual DAS_RESULT getAcquisitorInfo(Json::Value &result);
    virtual DAS_RESULT setGateway(const Json::Value &params);
    virtual DAS_RESULT getGateway(Json::Value &result);

    void getGatewayConfig(gateway_config_t &gateway);

private:
    std::string getConfigInfo(const char *ref);

    Timer *m_pTimer;
    BOOL  m_bConnected;
    int   m_ifdw;
    Lock  m_ilock;
private:
//    gateway_config_t m_gateway;

    MODULE_DECLARE(BaseInfo)
};

#endif
