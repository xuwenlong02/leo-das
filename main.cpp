#include <stdio.h>
#include <logger.h>
#include "baseinfo/baseinfo.h"

//#include "acquisition/acquisition.h"
#include "transport/transport_manager.h"
#include "devicemanager/devicemanager.h"
#include "remoteservice/remoteservice.h"
#include "utils/signals.h"
#include "transport/transport.h"
#include <defines.h>
#include <logger.h>
#include <mcheck.h>

#include <unistd.h>
#include <fcntl.h>

#ifdef RUN_TEST
#include <test/htcomhex.h>
#endif

extern void setVersion();
extern void sigint_catch(int sig);
extern void sig_handler(int sig);

TransportManager *pTransportManager;

int main(int argc, char *argv[])
{
    /*
     * 获取配置信息
     */
    int ch;

    while ((ch = getopt(argc,argv,"vsh"))) {
        switch(ch)
        {
        case 'h':
            printf("-v  获取版本号\n");
            printf("-s  绑定版本号\n");
            return 0;
        case 'v':
            printf(VERSION);
            return 0;
        case 's':
            setVersion();
            return 0;
        default:
            setVersion();
            break;
        }
        break;
    }


    /*
     * 初始化通讯
     */

    pTransportManager =
            new TransportManager();

    pTransportManager->SetupRemoteTask();
    pTransportManager->SetupLocalTask();
    /*
     * 初始化各功能模块
     */
    AcquisitionManager acquireMgr;
    DeviceManager  devMgr;
    ModuleInterface *pModules[DAS_TASK_MAX]={
        &acquireMgr,
        &devMgr,
        NULL,
        NULL
    };

    for (S32 i = 0;i<DAS_TASK_MAX;i++){
        AddTask((DAS_TASK)i,pModules[i]);
        if (pModules[i])
            pTransportManager->RegisterModules(pModules[i]);
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

    /*
     * 等待信号
     */
//    signal(SIGINT,sigint_catch);
//    signal(SIGINT,sigint_catch);
    int ret = 0;
    if(!utils::WaitTerminationSignals(&sig_handler)){
        LOG_ERR("failed to catch signal");
        ret = 1;
    }

    pTransportManager->ExitTask();

    /*
     * 释放模块
     */
    for (S32 i = 0;i < DAS_TASK_MAX; i++) {
        LOG_INFO("delete task %d",i);
        if (pModules[i])
            pModules[i]->Contruct(FALSE);
        LOG_INFO("delete task %d ok",i);
    }

    delete pTransportManager;

    LOG_ERR("exit %d",ret);
    return 0;
}

void sigint_catch(int sig) {
    LOG_INFO("sigint catch %d",sig);


//    exit(EXIT_FAILURE);
//    sig_handler(sig);
}

void sig_handler(int sig) {
  switch (sig) {
    case SIGINT:
      LOG_DEBUG("SIGINT signal has been caught");
      sigint_catch(sig);
      break;
    case SIGTERM:
      LOG_DEBUG("SIGTERM signal has been caught");
      sigint_catch(sig);
      break;
    case SIGSEGV:
      LOG_DEBUG( "SIGSEGV signal has been caught");
      // exit need to prevent endless sending SIGSEGV
      // http://stackoverflow.com/questions/2663456/how-to-write-a-signal-handler-to-catch-sigsegv
      abort();
    default:
      LOG_DEBUG("Unexpected signal has been caught");
      sigint_catch(sig);
  }
}

void setVersion()
{
    char script[255];
    FILE *fp = NULL;
    char buffer[64] = {0};

    sprintf(script,"echo %s > %s/VERSION",VERSION,getenv(CONFIG_DIR_ENV));

    fp = popen(script,"r");

    LOG_INFO("shell:%s",script);

    while(fgets(buffer,sizeof(buffer),fp) != NULL) {
        LOG_INFO("%s",buffer);
    }
    pclose(fp);
}

