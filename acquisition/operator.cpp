/*************************************************************************
	> File Name: operator.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年02月06日 星期二 12时42分00秒
 ************************************************************************/

#include<stdio.h>
#include "operator.h"
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <map>
#include <logger.h>
#include <utils/custom.h>
#include <vector>
#include <iterator>
#include "modbus.h"

Operator::Operator(OpInterface *interface):
    m_pOperatorInterface(interface),
    m_pItemData(NULL){
    PushStack(AUTO_CMD);
}

Operator::~Operator()
{
    PopStack(AUTO_CMD);
}

OpInterface *Operator::Interface()
{
    return m_pOperatorInterface;
}

OpValue Operator::GetFirstCmd(U16 itemid)
{
    m_mapOut.clear();
    m_iFuncId = 0;
    m_pItemData = m_pOperatorInterface->GetDataItem(itemid);
    if (!m_pItemData)
        return CmdValue::Null();
    if (!m_pItemData->GetSendExpr().valid()) {
        LOG_ERR("express is invalid");
        return CmdValue::Null();
    }

    return m_pItemData->GetSendExpr()->GenCmd(*this);

}

OpValue Operator::GetNextCmd()
{
    if (m_pItemData == NULL)
        return CmdValue::Null();
    return GetFirstCmd(m_pItemData->GetItemId()+1);
}

OpValue Operator::GenerateCmd(U16 itemid)
{
    LOG_TRACE();
    m_mapOut.clear();
    m_iFuncId = 0;
    m_pItemData = m_pOperatorInterface->GetSpecifyItem(itemid);
    if (!m_pItemData)
        return CmdValue::Null();
    if (!m_pItemData->GetSendExpr().valid()) {
        LOG_ERR("express is invalid");
        return CmdValue::Null();
    }
    return m_pItemData->GetSendExpr()->GenCmd(*this);
}

OpValue Operator::GenerateNext()
{
    m_iFuncId++;
    if (!m_pItemData)
        return CmdValue::Null();
    if (!m_pItemData->GetSendExpr().valid()) {
        LOG_ERR("express is invalid");
        return CmdValue::Null();
    }
    tExpress expr = FindDefinition("send");
    if (expr.valid())
        return m_pItemData->GetSendExpr()->GenCmd(*this);
    return OpValue::Null();
}

RECV_RESULT Operator::ParseRecvCmd(const CmdValue &recvCmd)
{
//    m_mapOut.clear();
    if (!m_pItemData || recvCmd.IsEmpty())
        return RECV_NOT_AVAILIABLE;
    if (!m_pItemData->GetRecvExpr().valid()) {
        LOG_ERR("express is invalid");
        return RECV_NOT_AVAILIABLE;
    }
    OpValue result = m_pItemData->GetRecvExpr()->ParseCmd(*this,0,recvCmd);
    if (result.IsEmpty())
        return RECV_NOT_AVAILIABLE;
    return RECV_COMPLETE;
}

OpValue Operator::operator[](const S8 *var)
{
    std::map<std::string,OpValue>::iterator iter;
    if ( (iter = m_mapOut.find(var)) != m_mapOut.end()) {
        tExpress expr = m_pItemData->GetDataExpr();
        if (!expr.valid())
            return OpValue::Null();
        if (m_pItemData->GetItemId()>=100)
            return expr->ParseCmd(*this)/m_pItemData->GetDataRate();
        else
            return expr->ParseCmd(*this);
    }
    return OpValue::Null();
}

U16 Operator::GetItemId()
{
    return m_pItemData->GetItemId();
}

string& Operator::GetItemName()
{
    return m_pItemData->GetName();
}

string& Operator::GetItemUnit()
{
    return m_pItemData->GetUnit();
}

S32 Operator::InsertCmd(int index,const OpValue& value)
{
    topData().opResult.insert(topData().opResult.begin()+index,value);
    if (value.type == TYPE_CMD)
        return value.cmd.Length();
    else if (value.IsDigital())
        return 1;
    return 0;
}

OpValue Operator::GetCmd(S32 start,S32 length)
{
    OpValue opValue;
    for (vector<OpValue>::iterator iter=
         topData().opResult.begin();
         iter != topData().opResult.end();
         iter++) {
        opValue << *iter;
    }
    return opValue.GenCmd(start,length);
}

S32 Operator::GetLength()
{
    S32 len = 0;
    for (vector<OpValue>::iterator iter=
         topData().opResult.begin();
         iter != topData().opResult.end();
         iter++) {
        if (((OpValue)*iter).type == TYPE_CMD)
            len += ((OpValue)*iter).cmd.Length();
        else if (((OpValue)*iter).IsDigital())
            len += 1;
    }
    return len;
}

void Operator::AddParam(OpValue value)
{
    topParam().opParams.push_back(value);
}

OpValue Operator::GetParam(S32 index)
{
    if (topParam().opParams.empty()||index<0 ||
            index >= topParam().opParams.size())
        return OpValue::Null();

    return topParam().opParams.at(index);
}

tExpress Operator::FindDefinition(const char *def)
{
    if (!m_pOperatorInterface)
        return NULL;
    tExpress expr;
    if ((strcmp(def,"send") == 0) ||
            (strcmp(def,"recv") == 0) ||
            (strcmp(def,"indict") == 0) ||
                        (strcmp(def,"undict") == 0))
        expr = m_pOperatorInterface->LocateFunction(def,m_iFuncId);
    else
        expr = m_pOperatorInterface->LocateFunction(def);
    if (expr.valid())
        return expr;
    else if (strcmp(def,"expr") == 0) {
        return m_pItemData->GetDataExpr();
    }
    return NULL;
}

OpValue Operator::VarCall(const char *var,S32 pos,const CmdValue& cmdValue)
{
    #define STREQUAL(str) !strcmp(var,str)

    /* 先使用局部定义变量 */
    std::map<std::string,OpValue>::iterator iter;

    if ( (iter = m_mapOut.find(var)) != m_mapOut.end()) {
        OpValue &opLen = iter->second;
        return opLen;
    }
    else if (STREQUAL("pos")) {
        return pos;
    }
    else if(STREQUAL("len")) {
        std::map<std::string,OpValue>::iterator iter;

        if ( (iter = m_mapOut.find("len")) != m_mapOut.end()) {
            OpValue &opLen = iter->second;
            return opLen;
        }
        return m_pItemData->GetRegLength();
    }
    else if (STREQUAL("addr")) {
        U8 *cmd;
        S32 len;
//
//        char devId[20] ={"7147491B"};
        len = utils::strtocmd(m_pOperatorInterface->DevAddr().c_str(),cmd);
        if (len == 0){
            LOG_ERR("str to hex cmd error");
            return OpValue::Null();
        }
        OpValue value(cmd,len);
        free(cmd);

        return value;
    }
    else {
        LOG_FIXME("var %s not defined",var);

        return OpValue::Null();
    }

#undef STREQUAL
}

OpValue Operator::FunctionCall(const char *func, S32 pos, const CmdValue &recvCmd)
{
    #define STREQUAL(str) !strcmp(func,str)

    std::map<std::string,OpValue>::iterator iter;

    if ( (iter = m_mapOut.find(func)) != m_mapOut.end()) {
        OpValue &opLen = iter->second;
        return CmdCalc(func,opLen);
    }
    else if (STREQUAL("group")) {
        if (topParam().opParams.size() < 1 ){
            LOG_ERR("group parameter error");
            return OpValue::Null();
        }
        OpValue v1;
        OpValue v2;
        if (topParam().opParams.size() >1 ) {
            v1 = GetParam(0);
            v2 = GetParam(1);
        }
        else {
            v1 = 0;
            v2 = GetParam(0);
        }
        if (!v1.IsInteger() || !v2.IsInteger()) {
            LOG_ERR("group parameter must be integer");
            return OpValue::Null();
        }

        if (v1.Integer() < 0 ||
                v1.Integer()+v2.Integer()>GetLength()) {
            LOG_ERR("group length is too long" );
            return OpValue::Null();
        }
        return GetCmd(v1.Integer(),v2.Integer());
    }
    else if (STREQUAL("assign")) {
        if (topParam().opParams.size()!=2){
            LOG_ERR("ditch parameter error");
            return OpValue::Null();
        }
        OpValue v1 = GetParam(0);
        OpValue v2 = GetParam(1);

        if (!v1.IsString()) {
            LOG_ERR("assign parameter must be integer");
            return OpValue::Null();
        }

        if (!strcmp(v1.data.str,"addr")) {
            CmdValue cmd = v2.GenCmd();
            string str;
            for (int i = 0;i<cmd.Length();i++) {
                str += utils::digittochar((cmd[i]&0xf0)>>4);
                str += utils::digittochar(cmd[i]&0x0f);
            }
            m_mapOut.insert(
                        std::pair<string,OpValue>(
                            v1.data.str,
                            OpValue(str.c_str())));
        }
        else if (!strcmp(v1.data.str,"len")) {
            m_mapOut.insert(
                        std::pair<string,OpValue>(
                            v1.data.str,
                            OpValue(v2.Integer())));
        }
        else {
            m_mapOut.insert(
                        std::pair<string,OpValue>(
                            v1.data.str,
                            v2));
        }

        return v2;
    }
    else if (STREQUAL("rbcd")) {
        if (topParam().opParams.size()  != 1){
            LOG_ERR("rbcd parameter error");
            return OpValue::Null();
        }
        OpValue v1 = GetParam(0);
        if (v1.type == TYPE_STR) {
            LOG_ERR("bcd parameter must be string");
            return OpValue::Null();
        }
        CmdValue cmd = v1.GenCmd();
        string str = utils::hexcmdtostr(cmd.Cmd(),cmd.Length());
        return OpValue(str.c_str());
    }
    else if (STREQUAL("chk")) {
        if (topParam().opParams.size()!=2) {
            LOG_ERR("chk parameter count error");
            return OpValue::Null();
        }

        OpValue v1 = GetParam(0);
        OpValue v2 = GetParam(1);

        if (v1 != v2) {
            LOG_ERR("chk error");
            return OpValue::Null();
        }
        return v2;
    }
    else if (STREQUAL("reverse")) {
        if (topParam().opParams.size()!=1) {
            LOG_ERR("chk parameter count error");
            return OpValue::Null();
        }
        CmdValue cmd = GetParam(0).GenCmd();
        for (int i = 0;i < cmd.Length()/2;i++) {
            U8 tmp = cmd[i];
            cmd[i] = cmd[cmd.Length()-i-1];
            cmd[cmd.Length()-i-1] = tmp;
        }
        return cmd;
    }
    else if (STREQUAL("crc")) {
        if (topParam().opParams.size()!=5) {
            LOG_ERR("crc parameter count error");
            return OpValue::Null();
        }
        OpValue v0 = GetParam(0);
        OpValue v1 = GetParam(1);
        OpValue v2 = GetParam(2);
        OpValue v3 = GetParam(3);
        OpValue v4 = GetParam(4);
        if (!v1.IsInteger() || !v2.IsInteger() || !v3.IsInteger() ||!v4.IsInteger()) {
            LOG_ERR("crc parameter must be 4 integer");
            return OpValue::Null();
        }
        U16 wCRCin = v2.Integer();
        U16 wCPoly = v1.Integer();
        BOOL low = (v4.Integer()==0);
        U8  wChar = 0;

        CmdValue usData = v0.GenCmd();

        for(int i = 0;i < usData.Length();i++)
        {
              wChar = usData[i];
              if (low)
                utils::InvertUint8(&wChar,&wChar);
              wCRCin ^= (wChar << 8);
              for(int i = 0;i < 8;i++)
              {
                if(wCRCin & 0x8000)
                  wCRCin = (wCRCin << 1) ^ wCPoly;
                else
                  wCRCin = wCRCin << 1;
              }
        }
        if (low)
            utils::InvertUint16(&wCRCin,&wCRCin);

        wCRCin ^= (U16)v3.Integer();

        CmdValue result;
        while (wCRCin&0x00ff) {
            result +=(U8)(wCRCin&0x00ff);
            wCRCin >>= 8;
        }
        return result;
    }
    else if(STREQUAL("data")) {

        if (topParam().opParams.size() == 2) {
            OpValue v1 = GetParam(0);
            OpValue v2 = GetParam(1);

            if (!v2.IsInteger() || v1.IsInteger()) {
                LOG_ERR("itom parameter must be integer");
                return OpValue::Null();
            }

            CmdValue data = ((*this)["data"]).GenCmd();
            S32 start = v1.Integer();
            S32 len = v2.Integer();

            if (data.IsEmpty() || len > data.Length() ||
                    start < 0 || start+len>data.Length()) {
                LOG_ERR("len(%d) bigger than cmd(%d)",len,data.Length());
                return OpValue::Null();
            }
            CmdValue hex;
            for (S32 i = 0;i<len;i++) {
                hex += data[start+i];
            }
            return hex;
        }
        else if(topParam().opParams.size() == 1){
            OpValue v1 = GetParam(0);
            m_mapOut.insert(
                        std::pair<string,OpValue>(
                            "data",
                            v1));
            return v1;
        }
        else{
            return recvCmd;
        }

    }
    else if(STREQUAL("itom")) {
        if (topParam().opParams.size()!=2) {
            LOG_ERR("data parameter count error");
            return OpValue::Null();
        }

        OpValue v1 = GetParam(0);
        OpValue v2 = GetParam(1);
        if (!v1.IsInteger() || !v2.IsInteger()) {
            LOG_ERR("itom parameter must be integer");
            return OpValue::Null();
        }

        if (v1.Integer()>4 || v1.Integer()<=0) {
            LOG_ERR("itom first parameter must be 1~4");
            return OpValue::Null();
        }
        union
        {
            U32 u32;
            U8  u8[4];
        }urt;
        urt.u32 = v2.Integer();
        S32 byte = v1.Integer();

        CmdValue result;
        for (S32 i = byte-1;i >= 0;i--) {
            result += urt.u8[i];
        }
        return result;
    }
    else
        return EncodeCalc(func,recvCmd);

#undef STREQUAL
}

OpValue Operator::EncodeCalc(const char *func, const CmdValue &recvCmd)
{
#define STREQUAL(str) !strcmp(func,str)
    OpValue v1;
    OpValue v2;
    OpValue v3;

    CmdValue data;
    if (topParam().opParams.size() == 1) {
        v1 = GetParam(0);
        data = v1.GenCmd();
        v2 = 0;
        v3 = data.Length();

    }
    else if (topParam().opParams.size() == 3) {
        v1 = GetParam(0);
        v2 = GetParam(1);
        v3 = GetParam(2);
        data = v1.GenCmd();
    }
    else {
        LOG_ERR("%s parameter count error",func);
        return recvCmd;
    }

    if (v1.IsEmpty() || !v2.IsInteger() || !v3.IsInteger()) {
        LOG_ERR("params type error");
        return OpValue::Null();
    }

    S32 start = v2.Integer();
    S32 len = v3.Integer();

    if (data.IsEmpty() || len > data.Length() ||
            start < 0 || start+len>data.Length()) {
        LOG_ERR("len(%d) bigger than cmd(%d)",len,data.Length());
        return OpValue::Null();
    }


    if (STREQUAL("int")) {
        S32 v = 0;
        for (int i = 0;i < len;i++) {
            v = (((U32)v)<<8)|(U8)data[start+i];
        }
        return v;
    }
    else if(STREQUAL("float")) {
        S32 v = 0;
        for (int i = 0;i < len;i++) {
            v = (((U32)v)<<8)|(U8)data[start+i];
        }
        F32 f = *(F32*)&v;
        return f;
    }
    else if(STREQUAL("sum")) {
        S32 vt = 0;
        for (S32 i = 0;i<len;i++) {
            vt += data[start+i];
        }
        return vt;
    }
    else if(STREQUAL("product")) {
        S32 vt = 1;
        for (S32 i = 0;i<len;i++) {
            vt *= data[start+i];
        }
        return vt;
    }
    else if(STREQUAL("bcd")) {
        S32 vt = 0;
        for (S32 i = 0;i<len;i++) {
            vt =vt*100+((data[start+i]&0x0f)+((data[start+i]&0xf0)>>4)*10);
        }
        return vt;
    }
    else if (STREQUAL("hex")){
        //hex
        CmdValue hex;
        for (S32 i = 0;i<len;i++) {
            hex += data[start+i];
        }
        return hex;
    }
    else {
        m_mapOut.insert(
                    std::pair<string,OpValue>(
                        func,
                        v1));
        return m_opDitch;
    }
}

OpValue Operator::CmdCalc(const char *func,OpValue &data)
{
    if (data.type == TYPE_CMD) {
        if (m_stackParam.empty() || topParam().opParams.size() == 0) {
            return data;
        }
        else {
            S32 start = 0;
            S32 len = 0;
            if (topParam().opParams.size() == 1) {
                OpValue v1 = GetParam(0);
                if (!v1.IsInteger()) {
                    LOG_ERR("%s first parameter error",func);
                    return OpValue::Null();
                }
                len = v1.Integer();
            }
            else {
                OpValue v1,v2;
                v1 = GetParam(0);
                v2 = GetParam(1);

                if (!v1.IsInteger() || !v2.IsInteger()) {
                    LOG_ERR("%s parameters error",func);
                    return OpValue::Null();
                }
                start = v1.Integer();
                len = v2.Integer();
            }

            if (start < 0 || (len!=-1 && start+len > data.cmd.Length())) {
                LOG_ERR("%s parameters is too long or error",func);
                return OpValue::Null();
            }
            return data.GenCmd(start,len);
        }
    }
    else {
        return data;
    }
}

OpValue Operator::KeyFunctionCall(const char *keyfunc, S32 byte, const CmdValue &recvCmd)
{
    if (!strcmp(keyfunc,"ditch")) {
        if (recvCmd.IsEmpty()) {
            LOG_ERR("ditch parameter is less");
            return OpValue::Null();
        }
        if (byte >recvCmd.Length()||byte <= 0) {
            LOG_ERR("ditch too long or too short");
            return OpValue::Null();
        }

        if (byte == 1)
            m_opDitch = OpValue(recvCmd[0]);
        else
            m_opDitch = OpValue(recvCmd.Cmd(),byte);
        return m_opDitch;
    }
    else if (!strcmp(keyfunc,"fill")) {
        OpValue v1 = GetParam(0);
        OpValue v2 = 0;
        if (v1.IsEmpty())
            return OpValue::Null();
        CmdValue cmd = v1.GenCmd();
        if (byte <= 0) {
            LOG_ERR("fill length cannot be zero");
            return OpValue::Null();
        }
        if (byte == 1)
            return OpValue(cmd[0]);

        if (topParam().opParams.size()>=2)
            v2 = GetParam(1);
        CmdValue result;
        for (int i = 0;i<byte && i<cmd.Length();i++) {
            result += cmd[i];
        }

        for (int i = cmd.Length();i < byte;i++) {
            if (v2.Integer()==0)
                result += 0;
            else
                result.Insert(0,0);
        }
        return result;
    }
    return OpValue::Null();
}

OpValue Operator::BaseCall(const char c,OpValue &v1,OpValue &v2)
{
    if (v1.IsEmpty() || v2.IsEmpty())
        return OpValue::Null();
    switch(c) {
    case '+':
        return v1+v2;
    case '-':
        return v1-v2;
    case '*':
        return v1*v2;
    case '/':
        return v1/v2;
    case '%':
        return v1%v2;
    default:
        return OpValue::Null();
    }
}

Operator::OperatorData &Operator::topData()
{
    return (Operator::OperatorData &)m_stackData.top();
}

Operator::OperatorParam &Operator::topParam()
{
    return (Operator::OperatorParam &)m_stackParam.top();
}

void Operator::PushStack(U32 type)
{
    if (type == AUTO_CMD)
        m_stackData.push(OperatorData());
    else if (type == AUTO_PARAM)
        m_stackParam.push(OperatorParam());
}

void Operator::PopStack(U32 type)
{
    if (type == AUTO_CMD)
        m_stackData.pop();
    else if (type == AUTO_PARAM)
        m_stackParam.pop();
}

void Operator::AddtoStack(U32 type, vector<OpValue> &list)
{
    if (type == AUTO_CMD)
        topData().opResult.insert(topData().opResult.begin(),
                                  list.begin(),list.end());
    else if (type == AUTO_PARAM) {
        vector<OpValue>::iterator iter;
        for (iter = list.begin(); iter != list.end();iter++) {
            topParam().opParams.push_back(*iter);
        }
    }
}
