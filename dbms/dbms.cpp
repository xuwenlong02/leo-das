/*************************************************************************
	> File Name: dbms.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年04月10日 星期二 16时33分30秒
 ************************************************************************/

#include "dbms.h"
#include <stdio.h>
#include <logger.h>

dbms::dbms(const string &table, int id):
    m_sqlite(this,table.c_str()),
    m_iId(id){
}

void dbms::StepId()
{
    m_iId++;
}

int dbms::OnSqliteResult(int searchId, void *data, int argc, char **argv, char **azColName)
{
    #define STR_EQUAL(s1,s2) !strcmp(s1,s2)

    if (searchId == 1 && argc == 1) {
        string *str = (string*)data;
        *str = argv[0]?argv[0]:"none";
        return 0;
    }
    else if(searchId == 2) {
        vector<string> *list = (vector<string> *)data;
        for (int i = 0;i < argc;i++) {
            list->push_back(argv[i]);
        }
        return 0;
    }
    else if (searchId == 3) {

        assignvalue(argc,argv,azColName);
        return 0;
    }
    else {
        LOG_ERR("not used searchId = %d, argc = %d",searchId,argc);
    }
    return -1;
}

string dbms::operator [](const string &item)
{
    string result;

    m_sqlite.Select(1,&result,item.c_str(),getcondition().c_str());
    return result;
}

BOOL dbms::update(const string &column, const string &value)
{
    char data[MAX_NAME_LEN+20];

    sprintf(data,"%s=%s",column.c_str(),value.c_str());
    return m_sqlite.Update(data,getcondition().c_str());
}

BOOL dbms::deletethis()
{
    return m_sqlite.Delete(getcondition().c_str());
}
BOOL dbms::FetchItems()
{
    clearall();
    return m_sqlite.Select(3,NULL,"*",getcondition().c_str());
}

BOOL dbms::exist()
{
    string result;
    if (m_sqlite.Select(1,&result,"count(id)",getcondition().c_str())){
        return atoi(result.c_str())>0;
    }
    return FALSE;
}

string dbms::getcondition()
{
    string strcon;

    char condition[30] = {0};
    sprintf(condition,"id=%d",m_iId);
    strcon=condition;
    return strcon;
}

