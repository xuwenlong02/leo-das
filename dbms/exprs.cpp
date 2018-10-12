/*************************************************************************
	> File Name: expr.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年04月10日 星期二 16时58分57秒
 ************************************************************************/

#include "exprs.h"

exprs::exprs(int id):
    dbms("exprs",id) {

}

string exprs::arithexpr()
{
    return (*this)["expr"];
}

string exprs::comment()
{
    return (*this)["comment"];
}
