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
    DataItem(vector<string> pa);
    ~DataItem();

    BOOL IsEmpty();

    /*
     * 解析表达式
     */
    BOOL parseExpr(string &str);
    virtual tExpress GetExprExpr();

    virtual tExpress GetSendExpr();
    virtual tExpress GetRecvExpr();
    virtual std::string& GetName();
    virtual std::string& GetUnit();
    virtual U16         GetItemId();
private:
    string wrapStr(string &str);
    tExpress wrapExpress(string &str);
private:

    U16         m_iItemId;
    std::string m_strname;

    tExpress    m_exprSend;

    tExpress    m_exprRecv;
    tExpress    m_exprExpr;
    std::string m_strunit;
};

#endif
