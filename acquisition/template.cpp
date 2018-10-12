/*************************************************************************
	> File Name: configuration.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年02月05日 星期一 15时29分52秒
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <iterator>
#include <stdlib.h>
#include <dlfcn.h>
#include <map>

#include <defines.h>
#include <logger.h>
#include <utils/custom.h>
#include <dbms/models.h>
#include <modbus.h>

#include "template.h"

Template::Template(S32 model):
    m_iLock(TRUE),
    m_model(model){
    //    Parse(file.c_str(),NULL);
}

Template::Template():
    m_iLock(TRUE),
    m_model(-1)
{
}

Template::~Template()
{
    ClearAll();
}

BOOL Template::IsParse()
{
    AutoLock autolock(m_iLock);
    return !m_mapFunctions.empty();
}

BOOL Template::Parse()
{
    AutoLock autolock(m_iLock);
    ClearAll();

    models model(m_model);
    string temp = model["template"];

    return Parse(temp);
}

BOOL Template::Parse(const string &temp)
{
    AutoLock autolock(m_iLock);
    if (temp.empty())
        return FALSE;

    vector<string> lines = utils::Split(temp.c_str(),"\n");

    for (vector<string>::iterator iter = lines.begin();
         iter != lines.end();iter++) {
        string line = *iter;
        line = utils::trim(line);

        if (line.empty())
            continue;
        if (*line.begin() == '#')
            continue;
        else {
            parseLine(line);
        }
    }
    return TRUE;
}

void Template::ClearAll()
{
    AutoLock autolock(m_iLock);
    m_mapFunctions.clear();
}

BOOL Template::IsEqual(S32 model)
{
    return m_model == model;
}

tExpress Template::LocateFunction(const char *def,int funcId)
{
    AutoLock autolock(m_iLock);
    std::map<std::string,tExpress>::iterator iter;

    if (funcId == -1) {
        if ((iter = m_mapFunctions.find(def)) !=
                m_mapFunctions.end()) {
            return (tExpress)iter->second;
        }
    }
    else {
        char func[64] = {0};
        sprintf(func,"%s[%d]",def,funcId);
        if ((iter = m_mapFunctions.find(func)) !=
                m_mapFunctions.end()) {
            return (tExpress)iter->second;
        }

        if (funcId == 0) {
            if ((iter = m_mapFunctions.find(def)) !=
                    m_mapFunctions.end()) {
                return (tExpress)iter->second;
            }
        }
    }
    return NULL;
}

BOOL Template::UpdateTemplate()
{
    return Parse();
}

BOOL Template::parseLine(const string& line)
{
    if (strncmp(line.c_str(),"def ",4) == 0) {

        std::string strname;
        std::string strcmd;

        int index = line.find_first_of(':');
        if (index<0) {
            LOG_ERR("format error,%s",line.c_str());
            return FALSE;
        }
        strname = line.substr(4,index-4);
        strcmd = line.substr(index+1);

        strname = utils::trim(strname);
        strcmd = utils::trim(strcmd);

        if (strname.empty()|| strcmd.empty()){
            LOG_ERR("format error:%s",line.c_str());
            return FALSE;
        }

        int bl = strname.find_first_of('[');
        int br = strname.find_first_of(']');
        if (bl>0 && br > bl) {
            tExpress expr = ExprOperator::WrapExpress(strcmd);
            expr->SetRecurse(FALSE);
            std::string func = strname.substr(0,bl);
            std::string an = strname.substr(bl+1,br-bl-1);
            int s = atoi(an.c_str());
            if (expr->Type() == OP_BRACE && expr->ChildrenSize() == s) {
                for(int i = 0;i<s;i++) {
                    char name[128] = {0};
                    sprintf(name,"%s[%d]",func.c_str(),i);
                    m_mapFunctions.insert(
                                std::pair<std::string,tExpress>(
                                    name,expr->ChildAt(i)));
                }
            }
        }
        else {
            m_mapFunctions.insert(
                        std::pair<std::string,tExpress>(
                            strname,
                            ExprOperator::WrapExpress(strcmd)));
        }
        return TRUE;
    }
    else {
        LOG_ERR("format error,%s",line.c_str());
        return FALSE;
    }
    return TRUE;
}


