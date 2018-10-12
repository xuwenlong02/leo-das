/*************************************************************************
	> File Name: configuration.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年02月05日 星期一 15时29分59秒
 ************************************************************************/

#ifndef __TEMPLATE_H__
#define __TEMPLATE_H__

#include <stdio.h>
#include <stack>
#include <string>
#include <vector>
#include <utils/lock.h>
#include <map>
#include "modbus.h"
#include "operator.h"
#include "dataitem.h"

using namespace std;
class DataItem;

class Template:public utils::RefrenceObj
{
public:
    Template(S32 model);
    Template();
    virtual ~Template();

    BOOL IsParse();
    BOOL Parse();
    BOOL Parse(const string &temp);
    void ClearAll();

    BOOL IsEqual(S32 model);

    virtual tExpress LocateFunction(const char* def, int funcId = -1);

    BOOL    UpdateTemplate();
private:
    BOOL parseLine(const string &line);

private:
    map<std::string,tExpress> m_mapFunctions;
    Lock                      m_iLock;
    S32                       m_model;
};

#endif //__CONFIGURATION_H__
