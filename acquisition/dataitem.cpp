/*************************************************************************
	> File Name: dataitem.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年02月24日 星期六 16时00分30秒
 ************************************************************************/
#include "dataitem.h"
#include <stdio.h>
#include <utils/custom.h>

DataItem::DataItem(){
}

DataItem::~DataItem()
{
}

BOOL DataItem::IsEmpty()
{
    if (m_strname.empty() ||
            !m_tSendExpr.valid() ||
            !m_tRecvExpr.valid() ||
            !m_tDataExpr.valid())
        return TRUE;
    return FALSE;
}

string &DataItem::GetName()
{
    return m_strname;
}

string &DataItem::GetUnit()
{
    return m_strunit;
}

U16 DataItem::GetItemId()
{
    return m_iItemId;
}

S32 DataItem::GetRegLength()
{
    return m_reglen;
}

tExpress DataItem::GetSendExpr()
{
    return m_tSendExpr;
}

tExpress DataItem::GetRecvExpr()
{
    return m_tRecvExpr;
}

tExpress DataItem::GetDataExpr()
{
    return m_tDataExpr;
}

F32 DataItem::GetDataRate()
{
    return m_fRate;
}

void DataItem::SetName(const string &name)
{
    m_strname = name;
}

void DataItem::SetUnit(const string &unit)
{
    m_strunit = unit;
}

void DataItem::SetItemId(U16 id)
{
    m_iItemId = id;
}

void DataItem::SetRegLength(S32 reglen)
{
    m_reglen = reglen;
}

void DataItem::ParseParams(const vector<string> &params)
{
    string strsend,strrecv;
    string paralist;


    for(int i = 0;i<params.size();i++) {
        vector<string> list = utils::Split(params[i],", ");
        string paras;
        for (int j = 0;j<list.size();j++) {
            paras += list.at(j);
            if (j != list.size()-1)
                paras += " ";
        }

        if (list.size()>1)
            paralist +="["+paras+"]";
        else
            paralist += paras;

        if (i != params.size()-1)
            paralist+=",";
    }

    if (m_iItemId < 100 && m_iItemId != 6) {
        strsend = "$indict";
        strrecv = "$undict";
    }
    else{
        strsend = "$send";
        strrecv = "$recv";
    }

    strsend += "(";
    strsend += paralist;
    strsend += ")";
    m_tSendExpr = wrapExpress(strsend);

    strrecv += "(";
    strrecv += paralist;
    strrecv += ")";
    m_tRecvExpr = wrapExpress(strrecv);
}

void DataItem::ParseDataExpr(const string &expr, F32 rate)
{
    string strdata;

    strdata = expr;
    m_fRate = rate;
    m_tDataExpr = wrapExpress(strdata);
}

string DataItem::wrapStr(string &str)
{
    string result;

    while(!str.empty()) {
        char c = *str.begin();
        str.erase(0,1);

        if (c == ' ') {
            if (result.empty()) {
                continue;
            }
            else {
                return result;
            }
        }
        else {
            result.append(1,c);
        }
    }
    return result;
}

tExpress DataItem::wrapExpress(const string &str)
{
    return ExprOperator::WrapExpress(str);
}
