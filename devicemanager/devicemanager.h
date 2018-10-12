/*************************************************************************
	> File Name: devicemanager.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月17日 星期三 12时03分33秒
 ************************************************************************/
#ifndef __DEVICEMANAGER_H__
#define __DEVICEMANAGER_H__

#include<stdio.h>
#include <transport/transport.h>
#include <logger.h>
#include <utils/threads/thread.h>
#include <vector>
#include <queue>
#include <utils/lock.h>
#include <utils/sqlite.h>
#include "deviceinfo.h"
#include <acquisition/acquisition.h>

using namespace std;

class DeviceManager:public ModuleInterface,public SqliteInterface
{
public:
    DeviceManager();
    virtual ~DeviceManager();

    virtual int OnSqliteResult(int searchId,void * data,int argc,char **argv,char **azColName);

    /*
     * remote request
     * 远程请求
     */
    virtual DAS_RESULT updateDeviceList(const Json::Value &params, Json::Value &result);
    virtual DAS_RESULT getDeviceIds(const Json::Value &params,Json::Value &result);
    virtual DAS_RESULT getDeviceStatus(const Json::Value &params,Json::Value &result);
    virtual DAS_RESULT getAcquireValues(const Json::Value &params,Json::Value& result);
    virtual DAS_RESULT getChnList(Json::Value &result);
    virtual DAS_RESULT getDeviceList(const Json::Value &params, Json::Value &result);
    virtual DAS_RESULT getDeviceInfo(const Json::Value &params,Json::Value &result);
    virtual DAS_RESULT addDevice(const Json::Value &params);
    virtual DAS_RESULT deleteDevice(const Json::Value &params);
    virtual DAS_RESULT modifyDevice(const Json::Value &params);
    virtual DAS_RESULT debugDevice(const Json::Value &params,Json::Value &result);
    virtual DAS_RESULT indicateDevice(const Json::Value &params,Json::Value &result);
    virtual DAS_RESULT getAcquireData(const Json::Value &params,Json::Value &result);
private:

    Json::Value getDeviceInfo(S32 devId);
    Json::Value getCommunication(const transmgr_t& trans);
    transmgr_t getCommunication(const Json::Value &jsonTrans);
    DeviceInfo *itemOfContain(S32 id);
    int         indexOfContain(S32 id);

private:
    Lock                  m_lockDevInfo;
    vector<DeviceInfo *>  m_listDeviceInfo;

    MODULE_DECLARE(DeviceManager)
};

#endif//__REMOTESERVICE_H__
