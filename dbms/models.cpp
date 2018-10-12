/*************************************************************************
	> File Name: models.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年04月17日 星期二 13时41分21秒
 ************************************************************************/
#include "models.h"
#include <malloc.h>

models::models(S32 id):
    dbms("models",id) {

}

BOOL models::inorup(const string &protocol)
{
    if (!exist()) {
        return insert(protocol);
    }
    else {
        return update(protocol);
    }
}

BOOL models::insert(const string &protocol)
{
    char *cond;

    cond = (char*)malloc(protocol.length()+MAX_COND_LEN);

    sprintf(cond,"%d,'%s','%s'",m_iId,"unknown",protocol.c_str());
    BOOL ret =  m_sqlite.Insert("id,name,template",cond);

    free(cond);
    return ret;
}

BOOL models::update(const string &protocol)
{
    char *cond;

    cond = (char*)malloc(protocol.length()+MAX_COND_LEN);

    sprintf(cond,"template='%s'",protocol.c_str());
    BOOL ret = m_sqlite.Update(cond,getcondition().c_str());
    free(cond);
    return ret;
}

BOOL models::updateName(const string &name)
{
    char *cond;

    cond = (char*)malloc(name.length()+MAX_COND_LEN);

    sprintf(cond,"name='%s'",name.c_str());
    BOOL ret = m_sqlite.Update(cond,getcondition().c_str());
    free(cond);
    return ret;
}
