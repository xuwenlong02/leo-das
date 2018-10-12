/*************************************************************************
	> File Name: acquire.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年04月10日 星期二 17时01分16秒
 ************************************************************************/

#ifndef __ACQUIRE_H__
#define __ACQUIRE_H__

#include "dbms.h"
#include <defines.h>

class acquire:public dbms
{
public:
    acquire(S32 model, S32 acqId);
    acquire(S32 model);

    BOOL inorup(const string &name, S32 reglen, const string &params, const string &expr, F32 rate);
    BOOL exist();
protected:
    virtual string getcondition();
    virtual void assignvalue(int argc, char **argv, char **azColName);
    virtual void clearall();
public:
    struct acquiredata
    {
        S32       itemId;
        string    dataexpr;
        vector<string> listparams;
        S32    reglen;
        F32    fRate;
    };
    vector<acquiredata> acqdata;

    vector<acquiredata> &getAcqParams();
private:
    S32 m_model;
    S32 m_acqId;
};

#endif
