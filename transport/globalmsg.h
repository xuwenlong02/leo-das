/*************************************************************************
	> File Name: globalmsg.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月25日 星期四 09时45分26秒
 ************************************************************************/
#ifndef __GLOBALMSG_H__
#define __GLOBALMSG_H__

#include <stdio.h>
#include <defines.h>
#include <json/json.h>

/*
 * 状态
 */
typedef enum
{
    DAS_INVALID = 0,
    DAS_SUCCESS ,
    DAS_TIMEOUT ,
    DAS_COLLISION,
    DAS_FILE_NOTFOUND,
    DAS_FILE_OPENFAILD,
    DAS_JSON_PARSEFAILD,
    DAS_JSON_REFNOTEXIST,
    DAS_MEM_NOTALLOC,
    DAS_GATEWAYID_ERROR,
    DAS_NOINFO,
    DAS_DEVICEEXIST,
    DAS_DEVICENOTEXIST,
    DAS_STOREERROR,
    DAS_INVALIDADDR,
    DAS_INVALIDPARAMETER,
    DAS_FAILDCONNECTION,
    DAS_FAILDSET,
    DAS_TEMPLATE_ERROR,
    DAS_DOWNPACKET_FAILED,
    DAS_RESTART,
    DAS_RESULT_MAX
}DAS_RESULT;

const char *GetError(DAS_RESULT status);

/*
 * 业务功能
 */
typedef enum{
    DAS_REGISTERACQUISITOR = 0,
    DAS_GETCONFIGFILES,
    DAS_COMMITDATA,
    DAS_SHOWSTATUS,
	/*************************************
	 * ADD before
     ************************************/
	DAS_MAX
}DAS_TYPE;

void InitMsg();
/*
* 设置消息内容
*/
void SetMsgStatus(DAS_TYPE type,BOOL status);

/*
* 获取消息内容
*/
BOOL GetMsgStatus(DAS_TYPE type);

/*
 * 业务功能
 */
typedef enum
{
    DAS_ACQUISITOR = 0,
    DAS_DEVICEMANAGER,
    DAS_BASE_INFO,
    DAS_REMOTESERVICE,
    DAS_TASK_MAX
}DAS_TASK;

/*
 * 增加业务
 */
void AddTask(DAS_TASK task,void *pTask);

/*
 * 获取业务
 */
void* GetTask(DAS_TASK task);

#endif//__GLOBALMSG_H__
