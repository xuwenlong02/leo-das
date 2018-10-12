/*************************************************************************
	> File Name: dataitem.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年02月24日 星期六 16时00分52秒
 ************************************************************************/
#ifndef __DATAITEM_H__
#define __DATAITEM_H__

#include <stdio.h>
#include <string>
#include "operator.h"
#include "opitem.h"

using namespace std;

class DataItem:public DataIterface
{
public:
    DataItem();
    virtual ~DataItem();

    BOOL IsEmpty();

    /*
     * 解析表达式
     */

    virtual std::string& GetName();
    virtual std::string& GetUnit();
    virtual U16         GetItemId();
    virtual S32 GetRegLength();
    virtual tExpress    GetSendExpr();
    virtual tExpress    GetRecvExpr();
    virtual tExpress    GetDataExpr();
    virtual F32  GetDataRate();

    virtual void SetName(const std::string& name);
    virtual void SetUnit(const std::string& unit);
    virtual void SetItemId(U16 id);
    virtual void SetRegLength(S32 reglen);
    virtual void ParseParams(const vector<string>& params);
    virtual void ParseDataExpr(const string& expr,F32 rate);

private:
    string wrapStr(string &str);
    tExpress wrapExpress(const string &str);
private:
    tExpress    m_tSendExpr;
    tExpress    m_tRecvExpr;
    U16         m_iItemId;
    std::string m_strname;
    std::string m_strunit;
    S32         m_reglen;
    tExpress    m_tDataExpr;
    F32         m_fRate;
};

#endif
