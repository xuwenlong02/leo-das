/*************************************************************************
	> File Name: sqlite.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年02月01日 星期四 09时48分33秒
 ************************************************************************/
#ifndef __SQLITE_H__
#define __SQLITE_H__

#include <stdio.h>
#include <sqlite3.h>
#include <defines.h>
#include <string>

class SqliteInterface
{
public:
    virtual int OnSqliteResult(int searchId,void * data,int argc,char **argv,char **azColName) = 0;
};

class SqliteHelper
{
public:
    SqliteHelper(SqliteInterface *interface,const char *table);
    SqliteHelper(const char *table);
    virtual ~SqliteHelper();

    void SetTable(const char *table);
    BOOL Insert(const char *items,const char *values);
    BOOL Select(int searchId,void *data,const char *items,const char *condition = NULL);
    BOOL Delete(const char *condition);
    BOOL Update(const char *item,const char *condition);

    struct SqliteData{
        SqliteInterface *interface;
        void            *data;
        int              searchId;
    };
    static int callback(void *data,int argc,char **argv,char **azColName);
private:

    BOOL exeSQL(int searchId,void *data, const char *sql);
private:
    sqlite3 *      m_pSqlite3;
    std::string    m_pTable;
    SqliteInterface * m_pInterface;
};

#endif//__SQLITE_H__
