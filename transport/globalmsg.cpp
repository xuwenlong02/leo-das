/*************************************************************************
	> File Name: globalmsg.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月25日 星期四 09时58分18秒
 ************************************************************************/

#include<stdio.h>
#include "globalmsg.h"

static BOOL g_iMsgData[DAS_MAX] = {0};

static void *  g_pTask[DAS_TASK_MAX];
static const char  g_sError[DAS_RESULT_MAX][30] ={
    "INVALID",
    "SUCCESS",
    "TIMEOUT",
    "COLLISION",
    "FILE NOT FAILD",
    "FILE OPEN FAILD",
    "JSON PARSE FAILD",
    "JSON FIELD NOT EXIST",
    "MEMORY NOT ALLOC",
    "GATEWAY ERROR",
    "INFO NOT SET",
    "DEVICE EXIST",
    "DEVICE NOT EXIST",
    "DATA FAILD",
    "INVALID ADDR",
    "PARAMS ERROR",
    "CONNECT FAILD",
    "CONFIGURE FAILD",
    "PROTOCOL ERROR",
    "DWONLOAD FAILED",
    "BEING RESTART"
};

/*
 * 错误
 */
const char *GetError(DAS_RESULT status)
{
    if (status < DAS_RESULT_MAX && status>=0)
        return g_sError[status];
    else
        return g_sError[0];
}

void InitMsg()
{
    for (int i = 0; i< DAS_MAX;i++) {
        g_iMsgData[(DAS_TYPE)i] = FALSE;
    }
}

void SetMsgStatus(DAS_TYPE type,BOOL status)
{
    g_iMsgData[type] = status;
}

BOOL GetMsgStatus(DAS_TYPE type)
{
    return g_iMsgData[type];
}

void AddTask(DAS_TASK task,void *pTask)
{
    g_pTask[task] = pTask;
}

void* GetTask(DAS_TASK task)
{
    return g_pTask[task];
}


