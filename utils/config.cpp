/*************************************************************************
	> File Name: config.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年04月28日 星期六 14时31分09秒
 ************************************************************************/

#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <malloc.h>
#include <defines.h>
#include <stdlib.h>
#include <map>
#include <sstream>
#include <inifile2/src/inifile.h>

namespace utils
{

using namespace std;
using namespace inifile;

string GetCompletePath()
{
    string path;
    path = getenv(CONFIG_DIR_ENV);
    path +="/config/Config.ini";
    return path;
}

BOOL GetSectionList(const string& pszSectionName,map<string, string> &mapList)
{
    IniFile ini;
    if (-1 == ini.load(GetCompletePath()))
        return FALSE;
    int ret = ini.getValues(pszSectionName,mapList);
    return ret == 0;
}

void WriteConfigFileStringValue(const string &pszSectionName,const string &pszKeyName,const string &pszValue)
{
    IniFile ini;
    if (RET_ERR == ini.load(GetCompletePath()))
        return;
    ini.setValue(pszSectionName,pszKeyName,pszValue);
}

void WriteConfigFileIntValue(const string &pszSectionName,const string &pszKeyName,S32 pszValue)
{
    IniFile ini;
    if (RET_ERR == ini.load(GetCompletePath()))
        return;
    stringstream ss;
    ss<<pszValue;
    ini.setValue(pszSectionName,pszKeyName,ss.str());
}

string GetConfigFileStringValue(const string &pszSectionName, const string &pszKeyName, const string &pDefaultVal)
{
    IniFile ini;
    int ret;
    if (RET_ERR == ini.load(GetCompletePath()))
        return pDefaultVal;
    string value = ini.getStringValue(pszSectionName,pszKeyName,ret);
    if (ret == RET_OK)
        return value;
    return pDefaultVal;
}

/**********************************************************************
* 功能描述： 从配置文件中获取整型变量
* 输入参数： pszSectionName-段名, 如: GENERAL
             pszKeyName-配置项名, 如: EmployeeName
             iDefaultVal-默认值
             pszConfigFileName-配置文件名
* 输出参数： 无
* 返 回 值： iGetValue-获取到的整数值   -1-获取失败
* 其它说明： 无
* 修改日期         版本号       修改人        修改内容
* ------------------------------------------------------------------
* 20150507       V1.0     Zhou Zhaoxiong     创建
********************************************************************/
S32 GetConfigFileIntValue(const string &pszSectionName, const string &pszKeyName, U32 iDefaultVal)
{
    IniFile ini;
    int ret;
    if (RET_ERR == ini.load(GetCompletePath()))
        return iDefaultVal;
    int value = ini.getIntValue(pszSectionName,pszKeyName,ret);
    if (ret == RET_OK)
        return value;
    return iDefaultVal;
}

/**********************************************************************
* 功能描述： 从配置文件中获取整型变量
* 输入参数： pszSectionName-段名, 如: GENERAL
             pszKeyName-配置项名, 如: EmployeeName
             iDefaultVal-默认值
             pszConfigFileName-配置文件名
* 输出参数： 无
* 返 回 值： iGetValue-获取到的整数值   -1-获取失败
* 其它说明： 无
* 修改日期         版本号       修改人        修改内容
* ------------------------------------------------------------------
* 20150507       V1.0     Zhou Zhaoxiong     创建
********************************************************************/
F32 GetConfigFileFloatValue(const string &pszSectionName, const string &pszKeyName, F32 iDefaultVal)
{
    IniFile ini;
    int ret;
    if (RET_ERR == ini.load(GetCompletePath()))
        return iDefaultVal;
    double value = ini.getDoubleValue(pszSectionName,pszKeyName,ret);
    if (ret == RET_OK)
        return value;
    return iDefaultVal;
}
}
