/*************************************************************************
	> File Name: config.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年04月28日 星期六 14时31分17秒
 ************************************************************************/
/**********************************************************************
* 版权所有 (C)2015, Zhou Zhaoxiong。
*
* 文件名称：GetConfig.c
* 文件标识：无
* 内容摘要：演示Linux下配置文件的读取方法
* 其它说明：无
* 当前版本：V1.0
* 作    者：Zhou Zhaoxiong
* 完成日期：20150507
*
**********************************************************************/

#ifndef __FILE_CONFIG_H__
#define __FILE_CONFIG_H__

#include <defines.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>

namespace utils {

using namespace std;

/**********************************************************************
* 功能描述： 获取配置文件完整路径(包含文件名)
* 输入参数： pszConfigFileName-配置文件名
             pszWholePath-配置文件完整路径(包含文件名)
* 输出参数： 无
* 返 回 值： 无
* 其它说明： 无
* 修改日期        版本号         修改人       修改内容
* ------------------------------------------------------------------
* 20150507       V1.0     Zhou Zhaoxiong     创建
********************************************************************/
std::string GetCompletePath();

BOOL GetSectionList(const string& pszSectionName,map<string,string> &mapList);

void WriteConfigFileStringValue(const string &pszSectionName,const string &pszKeyName,const string &pszValue);

void WriteConfigFileIntValue(const string &pszSectionName,const string &pszKeyName,S32 pszValue);


/**********************************************************************
* 功能描述： 从配置文件中获取字符串
* 输入参数： pszSectionName-段名, 如: GENERAL
             pszKeyName-配置项名, 如: EmployeeName
             pDefaultVal-默认值
             iOutputLen-输出缓存长度
             pszConfigFileName-配置文件名
* 输出参数： pszOutput-输出缓存
* 返 回 值： 无
* 其它说明： 无
* 修改日期        版本号         修改人       修改内容
* ------------------------------------------------------------------
* 20150507       V1.0     Zhou Zhaoxiong     创建
********************************************************************/
string GetConfigFileStringValue(const string &pszSectionName, const string &pszKeyName, const string &pDefaultVal);


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
S32 GetConfigFileIntValue(const string &pszSectionName, const string &pszKeyName, U32 iDefaultVal);

F32 GetConfigFileFloatValue(const string &pszSectionName, const string &pszKeyName, F32 iDefaultVal);

}
#endif
