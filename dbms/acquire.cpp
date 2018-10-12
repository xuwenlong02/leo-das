/*************************************************************************
	> File Name: acquire.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年04月10日 星期二 17时01分08秒
 ************************************************************************/

#include "acquire.h"
#include <logger.h>
#include <utils/custom.h>

acquire::acquire(S32 model,S32 acqId):
    dbms("acquire",0),
    m_model(model),
    m_acqId(acqId){

}

acquire::acquire(S32 model):
    dbms("acquire",0),
    m_model(model),
    m_acqId(-1)
{
}

BOOL acquire::inorup(const string& name,S32 reglen,const string& params,const string& expr,F32 rate)
{
    if (!exist()) {
        char cond[MAX_COND_LEN] = {0};
        sprintf(cond,"%d,'%s',%d,'%s',%d,'%s',%f",m_acqId,
                name.c_str(),
                m_model,
                params.c_str(),
                reglen,
                expr.c_str(),
                rate);
        return m_sqlite.Insert("acqId,name,model,params,reglen,expr,rate",cond);
    }
    else {
        char cond[MAX_COND_LEN] = {0};

        sprintf(cond,"name='%s',params='%s',reglen=%d,expr='%s',rate=%f",
                name.c_str(),
                params.c_str(),
                reglen,
                expr.c_str(),
                rate);
        return m_sqlite.Update(cond,getcondition().c_str());
    }
}

BOOL acquire::exist()
{
    string result;
    if (m_sqlite.Select(1,&result,"count(acqId)",getcondition().c_str())){
        return atoi(result.c_str())>0;
    }
    return FALSE;
}

string acquire::getcondition()
{
    string cond;
    char condition[50] = {0};
    if (m_acqId != -1)
        sprintf(condition,"model=%d and acqId=%d",m_model,m_acqId);
    else
        sprintf(condition,"model=%d",m_model);
    cond = condition;
    return cond;
}

void acquire::assignvalue(int argc, char **argv, char **azColName)
{
#define STR_EQUAL(s1,s2) !strcmp(s1,s2)
    acquiredata acq;
    for (int i = 0;i < argc;i++) {
        if (STR_EQUAL(azColName[i],"acqId")) {
            acq.itemId=atoi(argv[i]);
        }
        else if (STR_EQUAL(azColName[i],"reglen")) {
            acq.reglen = atoi(argv[i]);
        }
        else if (STR_EQUAL(azColName[i],"expr")) {
            acq.dataexpr = argv[i];
        }
        else if (STR_EQUAL(azColName[i],"rate")) {
            acq.fRate = atof(argv[i]);
        }
        else if (STR_EQUAL(azColName[i],"params")) {
            acq.listparams = utils::Split(argv[i],";");
        }
    }
    acqdata.push_back(acq);
}

void acquire::clearall()
{
    acqdata.clear();
}

vector<acquire::acquiredata> &acquire::getAcqParams()
{
    return acqdata;
}
