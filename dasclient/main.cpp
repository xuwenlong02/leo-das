/*************************************************************************
	> File Name: main.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月17日 星期一 14时03分04秒
 ************************************************************************/
#include "dasclient.h"
#include <logger.h>
#include <utils/signals.h>
#include <baseinfo/baseinfo.h>
#include <remoteservice/remoteservice.h>

void sighandler(int sig);

int main(int argc, char *argv[])
{
//    LOG_ENABLE(false);

    DasClient dasclient;
    /*
     * 初始化各功能模块
     */
    BaseInfo baseInfo;
    RemoteService  rmtService;
    ModuleInterface *pModules[DAS_TASK_MAX]={
        NULL,
        NULL,
        &baseInfo,
        &rmtService
    };

    for (S32 i = 0;i<DAS_TASK_MAX;i++){
        AddTask((DAS_TASK)i,pModules[i]);
        if (pModules[i]) {
            pModules[i]->RegisterModule(dasclient.Handler());
            pModules[i]->Contruct(TRUE);
        }
    }

    /*
     * 初始化采集模块
     */
    for (S32 i = 0;i < DAS_TASK_MAX; i++) {
        LOG_DEBUG("%d",i);
        if (pModules[i])
            pModules[i]->Contruct(TRUE);
        LOG_DEBUG("%d ok",i);
    }

    utils::WaitTerminationSignals(sighandler);

    for (S32 i = 0;i<DAS_TASK_MAX;i++){
        if (pModules[i]) {
            pModules[i]->Contruct(FALSE);
            LOG_DEBUG("%d destroy",i);
        }
    }

    return 0;
}

void sighandler(int sig)
{

}
