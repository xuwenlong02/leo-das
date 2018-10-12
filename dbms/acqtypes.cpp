/*************************************************************************
	> File Name: acqtypes.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年04月10日 星期二 17时00分42秒
 ************************************************************************/

#include "acqtypes.h"

acqtypes::acqtypes(int id):
    dbms("acqtypes",id)
{

}

string acqtypes::getcondition()
{
    string cond;
    char condition[30] = {0};
    sprintf(condition,"id='%x'",m_iId);
    cond = condition;
    return cond;
}
