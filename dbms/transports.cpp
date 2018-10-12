/*************************************************************************
	> File Name: transport.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年04月12日 星期四 15时07分40秒
 ************************************************************************/

#include "transports.h"

transports::transports(int id):
    dbms("transport",id) {

}

BOOL transports::inorup(F32 sample, U64 regtime, S32 timeout, S32 mode, const char *addr)
{
    if (!exist()) {
        return insert(sample,regtime,timeout,mode,addr);
    }
    else
        return update(sample,regtime,timeout,mode,addr);
}

BOOL transports::insert(F32 sample, U64 regtime, S32 timeout, S32 mode, const char *addr)
{
    char cond[MAX_COND_LEN] = {0};

    sprintf(cond,"%d,%0.2f,%lld,%d,%d,'%s'",m_iId,sample,regtime,timeout,mode,addr);
    return m_sqlite.Insert("id,sample,regtime,timeout,mode,addr",cond);
}

BOOL transports::update(F32 sample, U64 regtime, S32 timeout, S32 mode, const char *addr)
{
    char cond[MAX_COND_LEN] = {0};

    sprintf(cond,"sample=%0.2f,regtime=%lld,timeout=%d,mode=%d,addr='%s'",sample,regtime,timeout,mode,addr);
    return m_sqlite.Update(cond,getcondition().c_str());
}
