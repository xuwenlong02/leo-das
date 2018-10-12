/*************************************************************************
	> File Name: dasserver.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月17日 星期一 14时00分10秒
 ************************************************************************/

#ifndef __DASCLIENT_H__
#define __DASCLIENT_H__
#include <event.h>
#include <vector>
#include <utils/shared_ptr.h>
#include <transport/remote/client/sclient.h>
#include <utils/timer.h>

using namespace std;

class DasClient:public Sclient,public TimerTask
{
public:
    DasClient();
    virtual ~DasClient();

    virtual void RunTimer();

    virtual void OnStatus(bool connect);
private:
    Timer *m_pTimer;
};

#endif
