/*************************************************************************
	> File Name: acquisitor.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年02月02日 星期五 14时22分48秒
 ************************************************************************/
#ifndef __ACQUISITOR_H__
#define __ACQUISITOR_H__

#include<stdio.h>
#include <utils/timer.h>
#include <utils/shared_ptr.h>
#include <transport/transport.h>
#include <utils/sqlite.h>
#include <string>
#include "template.h"
#include <utils/fixlist.cpp>
#include <devicemanager/deviceinfo.h>
#include "acqdata.h"
#include "modbus.h"
//class Acquisitor;
//class Configration;

class Acquisitor:public OpInterface,public TimerTask,public JsonRpcInterface
{
public:
    Acquisitor(const deviceinfo_t& devinfo);
    virtual ~Acquisitor();
    virtual void RunTimer();

    void StopTimer();

    BOOL IsEqualId(S32 id);

    OpValue IndicateDataItem(U16 itemid);

    virtual DataItem *GetDataItem(U16 itemid);
    virtual DataIterface *GetSpecifyItem(U16 itemid);

    virtual tExpress LocateFunction(const char* def, int funcId = -1);
    virtual const ID_DEV &DevAddr();

    S32 commitId();

    void handleJsonResult(BOOL ret);

    virtual void OnReceiveJson(DAS_RESULT result,
                               const std::string &method,
                               const Json::Value &json);

    BOOL updateAquireData();

    Template* getTemplate();

    tConnection getConnector();
    BOOL IsConnected();

    vector<AcqData> getAcquireData();

    BOOL updateConnector();
    void commimtData();
    void clearData();
private:
    void resetCommit();
    void commimtError(const std::string &err);

    class CommitTimer
    {
    public:
        CommitTimer(Timer *timer_,int sample_) {
            timer = timer_;
            sample = sample_;
        }

        ~CommitTimer() {
            time_t now = time(NULL);
            S64 secs = (S64)(60*sample);

            if (secs == 0)
                return;
            S64 wait = secs-((S64)(now))%secs;

            timer->set_timeout(wait*1000);
        }
    private:
        Timer *timer;
        int  sample;
    };

private:
    const deviceinfo_t&     m_idevinfo;
    Timer *                 m_pTimer;
    Lock                    m_iLock;

    S32                     m_iCommitId;
    std::vector<DataItem>   m_listDataItems;

    Lock                    m_iDataLock;
    FixList<AcqData,5000>   m_fixlistData;

    Lock                    m_iTmpLock;
    vector<AcqData>         m_listAcquire;
};

#endif //__ACQUISITOR_H__

