/*************************************************************************
	> File Name: sqlite.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年02月01日 星期四 09时48分23秒
 ************************************************************************/

#include<stdio.h>
#include <logger.h>
#include <string>
#include "sqlite.h"

#define MAX_SQL_LEN    128

SqliteHelper::SqliteHelper(SqliteInterface *interface, const char *table):
    m_pInterface(interface)
{
    char sql_file[256];

    m_pTable = table;

    sprintf(sql_file,"%s/config/das.db",getenv(CONFIG_DIR_ENV));
    if (sqlite3_open(sql_file,&m_pSqlite3)){
        LOG_ERR("can't open database %s,%s",sql_file,sqlite3_errmsg(m_pSqlite3));
        return;
    }
}

SqliteHelper::SqliteHelper(const char *table):
    m_pInterface(NULL)
{
    char sql_file[256];

    m_pTable = table;

    sprintf(sql_file,"%s/config/das.db",getenv(CONFIG_DIR_ENV));
    if (sqlite3_open(sql_file,&m_pSqlite3)){
        LOG_ERR("can't open database %s,%s",sql_file,sqlite3_errmsg(m_pSqlite3));
        return;
    }
}

SqliteHelper::~SqliteHelper()
{
    sqlite3_close(m_pSqlite3);
}

void SqliteHelper::SetTable(const char *table)
{
    m_pTable = table;
}

BOOL SqliteHelper::Insert(const char *items, const char *values)
{
    BOOL ret;
    int len = strlen(items)+strlen(values);
    char *sql = (char*)malloc(MAX_SQL_LEN+len);

    sprintf(sql,"insert into %s(%s) values(%s);",
            m_pTable.c_str(),items,values);
    ret = exeSQL(0,NULL,sql);
    free(sql);
    return ret;
}

BOOL SqliteHelper::Select(int searchId,void *data, const char *items, const char *condition)
{
    char *sql;
    int len = strlen(items);
    BOOL ret;

    sql = (char*)malloc(len+MAX_SQL_LEN);
    bzero(sql,len+MAX_SQL_LEN);

    if (!condition)
        sprintf(sql,"select %s from %s;",items,m_pTable.c_str());
    else
        sprintf(sql,"select %s from %s where %s;",
                items,m_pTable.c_str(),condition);
    ret = exeSQL(searchId,data,sql);
    free(sql);
    return ret;
}

BOOL SqliteHelper::Delete(const char *condition)
{
    BOOL ret;
    int len = strlen(condition);
    char *sql = (char*)malloc(MAX_SQL_LEN+len);

    sprintf(sql,"delete from %s where %s;",
            m_pTable.c_str(),condition);
    ret = exeSQL(0,NULL,sql);
    free(sql);
    return ret;
}

BOOL SqliteHelper::Update(const char *item, const char *condition)
{
    BOOL ret;
    int len = strlen(item)+strlen(condition);
    char *sql = (char*)malloc(MAX_SQL_LEN+len);

    sprintf(sql,"update %s set %s where %s;",
            m_pTable.c_str(),item,condition);
    ret=  exeSQL(0,NULL,sql);
    free(sql);
    return ret;
}

int SqliteHelper::callback(void *data, int argc, char **argv, char **azColName)
{
    SqliteData *sd = static_cast<SqliteData*>(data);
//    for(int i=0; i<argc; i++){
//       LOG_INFO("searchid=%d ,%s = %s\n",sd->searchId, azColName[i], argv[i] ? argv[i] : "NULL");
//    }
    if (sd->interface && sd->searchId != 0)
        return sd->interface->OnSqliteResult(sd->searchId,sd->data,argc,argv,azColName);
    else {
        if (argc==0)
            return 0;
        std::string* result = (std::string *)sd->data;
        *result = argv[0];
    }
    return 0;
}

BOOL SqliteHelper::exeSQL(int searchId,void *data, const char *sql)
{
    char *zErrMsg = NULL;
    SqliteData sd = {m_pInterface,data,searchId};

    try
    {
    if (sqlite3_exec(m_pSqlite3,sql,SqliteHelper::callback,&sd,&zErrMsg) != SQLITE_OK){
        LOG_ERR("sqlite error: %s",zErrMsg);
        sqlite3_free(zErrMsg);
        return FALSE;
    }
    }
    catch(char* e) {
        LOG_ERR("exception %s,%s",sql,e);
        return FALSE;
    }

    return TRUE;
}
