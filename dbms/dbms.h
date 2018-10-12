/*************************************************************************
	> File Name: dbms.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年04月10日 星期二 16时39分03秒
 ************************************************************************/
#ifndef __DBMS_H__
#define __DBMS_H__

#include <stdio.h>
#include <utils/sqlite.h>
#include <string>
#include <vector>

using namespace std;

class dbms:public SqliteInterface
{
public:
    dbms(const string& table,int id = -1);

    void StepId();

    virtual int OnSqliteResult(int searchId,void * data,int argc,char **argv,char **azColName);

    string operator[](const string& item);
//    vector<string> operator[](const string& item);

    BOOL FetchItems();

    BOOL exist();
    BOOL update(const string& column,const string& value);
    BOOL deletethis();
protected:
    virtual string getcondition();
    virtual void assignvalue(int argc, char **argv, char **azColName) {
        UNUSED(argc);UNUSED(argv);UNUSED(azColName);
    }
    virtual void clearall() {}

protected:
    int m_iId;
    SqliteHelper m_sqlite;
};

#endif //__DBMS_H__
