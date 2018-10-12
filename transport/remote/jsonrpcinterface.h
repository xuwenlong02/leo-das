/*************************************************************************
	> File Name: jsonrpcinterface.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月29日 星期六 17时00分39秒
 ************************************************************************/
#ifndef __JSONRPCINTERFACE_H__
#define __JSONRPCINTERFACE_H__
#include <utils/atomic.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <macro.h>
#include <defines.h>
#include "../globalmsg.h"
#include <utils/shared_ptr.h>

class JsonRpcInterface:public utils::RefrenceObj
{
public:
    JsonRpcInterface();
    virtual ~JsonRpcInterface();

    virtual void OnReceiveJson(
            DAS_RESULT status,
            const std::string &method,
            const Json::Value &result) = 0;
};

typedef utils::SharedPtr<JsonRpcInterface> tJsonRpcInterface;

#endif
