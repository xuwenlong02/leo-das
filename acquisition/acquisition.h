/*************************************************************************
	> File Name: acquisition.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月17日 星期三 13时43分45秒
 ************************************************************************/

#ifndef __ACQUISITION_H__
#define __ACQUISITION_H__

#include <baseinfo/baseinfo.h>
#include <transport/transport.h>
#include <utils/timer.h>
#include <utils/shared_ptr.h>
#include <utils/sqlite.h>
#include <vector>
#include "acquisitor.h"
#include "template.h"

using namespace std;

class Acquisitor;
class Template;

typedef utils::SharedPtr<Acquisitor> tAcquisitor;
typedef utils::SharedPtr<Template> tTemplate;



class AcquisitionManager:public ModuleInterface,public TimerTask,public JsonRpcInterface
{
public:
    AcquisitionManager();

    virtual ~AcquisitionManager();
    virtual void RunTimer();

    /*
     * inherid from ModuleInterface
     */
    virtual void OnReceiveJson(
            DAS_RESULT result,
            const std::string &method,
            const Json::Value &json);

    virtual void RegisterAcquisitor();

    virtual DAS_RESULT connectServer(
            const Json::Value &params);
    virtual DAS_RESULT clearData();
    virtual DAS_RESULT startAcquire();
    virtual DAS_RESULT stopAcquire();

    tAcquisitor GetAcquisitor(const deviceinfo_t& devinfo);
    void UpdateAcquisitor(const deviceinfo_t& devinfo);

    BOOL ExistTemplate(S32 model);
    tTemplate GetTemplate(S32 model);
    void UpdateTemplate(S32 model);

private:
    Lock  m_ilock;
    BOOL                   m_bConnected;
    vector<tAcquisitor>    m_listAcquisitor;
    vector<tTemplate>      m_listTemplate;
//    Lock                   m_iConfigLock;
//    Lock                   m_iAcquisitorLock;
    Timer *                m_pTimer;

private:

    MODULE_DECLARE(AcquisitionManager)
};

#endif//__ACQUISITION_H__
