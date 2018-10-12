/*************************************************************************
	> File Name: operator.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年02月06日 星期二 12时42分09秒
 ************************************************************************/
#ifndef __OPERATOR_H__
#define __OPERATOR_H__

#include<stdio.h>
#include <string>
#include <defines.h>
#include <vector>
#include <stack>
#include <map>
#include <logger.h>
#include "opitem.h"
#include <utils/shared_ptr.h>
#include "template.h"
#include "modbus.h"

using namespace std;

class Operator
{
public:
    Operator(OpInterface *interface = NULL);
//    Operator(const Operator& obj,S32 flag);
    virtual ~Operator();

    OpInterface *Interface();

    /*
     * 命令解析
     */
    OpValue GetFirstCmd(U16 itemid);
    OpValue GetNextCmd();
    OpValue GenerateCmd(U16 itemid);
    OpValue GenerateNext();
    RECV_RESULT ParseRecvCmd(const CmdValue &recvCmd);

    OpValue operator[](const S8 *var);

    U16 GetItemId();
    string &GetItemName();
    string &GetItemUnit();

    /*
     * 命令生产
     */
    S32 InsertCmd(int index,const OpValue& value);
    OpValue GetCmd(S32 start = 0, S32 length = -1);
    S32     GetLength();
    /* 必须按顺序添加参数 */
    void AddParam(OpValue value);
    OpValue GetParam(S32 index);

    /* 相关计算 */
    tExpress FindDefinition(const char *def);
    OpValue  VarCall(const char *var,
                     S32 pos = 0,
                     const CmdValue &cmdValue = CmdValue::Null());
    OpValue  FunctionCall(const char *func,
                          S32 pos = 0,
                          const CmdValue &recvCmd = CmdValue::Null());

    OpValue EncodeCalc(const char *func,const CmdValue &recvCmd);

    OpValue CmdCalc(const char *func, OpValue &data);

    OpValue KeyFunctionCall(const char *keyfunc,S32 byte = 0,
                            const CmdValue& cmdValue = CmdValue::Null());
    OpValue  BaseCall(const char c,OpValue &v1,OpValue &v2);

private:

    class OperatorData
    {
    public:
        OperatorData() {

        }
        vector<OpValue>     opResult;

    };
    Operator::OperatorData& topData();

    class OperatorParam
    {
    public:
        OperatorParam() {}
        vector<OpValue>  opParams;
    };
    Operator::OperatorParam& topParam();

    OpValue             m_opDitch;

public:
    enum {AUTO_CMD = 0,AUTO_PARAM};
    class AutoOperator {
    public:

        /* mode:0直接入栈，1延迟入栈 */
        AutoOperator(Operator& op,U32 type,U32 mode = 0):
            opData(op),
            stacktype(type),
            stackmode(mode),
            push(mode == 0){
            if (mode == 0)
                opData.PushStack(stacktype);
        }

        void AddList(const OpValue &value) {
            templist.push_back(value);
        }

        S32 StackAll() {
            opData.PushStack(stacktype);
            opData.AddtoStack(stacktype,templist);
            push = TRUE;
        }

        virtual ~AutoOperator() {
            if (push)
                opData.PopStack(stacktype);
        }

    private:
        Operator& opData;
        BOOL      push;
        U32       stacktype;
        U32       stackmode;
        vector<OpValue> templist;
    };

    void PushStack(U32 type);
    void PopStack(U32 type);
    void AddtoStack(U32 type,vector<OpValue> &list);

private:
    OpInterface *          m_pOperatorInterface;
    DataIterface*              m_pItemData;
    map<string,OpValue>    m_mapOut;
    stack<OperatorData>    m_stackData;
    stack<OperatorParam>   m_stackParam;
    int                    m_iFuncId;
};

#endif //__OPERATOR_H__
