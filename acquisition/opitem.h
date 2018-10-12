/*************************************************************************
	> File Name: opitem.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年02月23日 星期五 15时01分34秒
 ************************************************************************/
#ifndef __OPITEM_H__
#define __OPITEM_H__

#include <stdio.h>
#include <defines.h>
#include <string>
#include <stack>
#include <vector>
#include <queue>
#include <logger.h>
#include <utils/shared_ptr.h>
#include "operator.h"
#include "modbus.h"

using namespace std;

typedef enum
{
    OP_NONE =0,
    OP_NUM ,
    OP_BASE,//基本运算符，+-×/,& 运算符
    OP_VAR,
    OP_FUNC,
    OP_KEYFUNC,  //关键函数,ditch,fill
    OP_COMMA,/* 逗号 */
    OP_BRACE,
    OP_BRACKET, /* 中括号 */
    OP_PARENTHESS /* 小括号 */
}OP_TYPE;

class OpItem
{
public:
    OpItem(const S8 ch);

    OpItem(const S8 *var, BOOL isfunc = false);

    OpItem(const OpItem &item);

    OpItem();

    virtual ~OpItem();

    OpItem& operator=(const OpItem& item);
    /*外 优先级 ,数字优先级设为-1 */
    int priority();

    void show();

    union OpData{
        OpData(){}
        ~OpData(){}
        S8 ch;
        S8 var[MAX_VAR_LEN];
    };

    OpValue digit;
    OpData op;
    OP_TYPE     type;
    S32         params;/* 参数个数 */


private:
    void release();
    void copyitem(const OpItem &item);
private:


};

class Express;
typedef utils::SharedPtr<Express> tExpress;

class ExprOperator
{
public:
    static BOOL IsAvailiable(const string &express);
    static tExpress WrapExpress(const string &str);
    static vector<OpItem> GetOpts(const string &str);

private:
    static BOOL generatePostExpress(vector<OpItem> &queue_list,
                             stack<OpItem> &stackOperators);
    static Express* parseExpress(stack<OpItem> &stackOperators);
    static Express* parseExpress(queue<OpItem> &queueOperators);

    static int topNum(stack<int> &params,int add);
    static string wrapWord(string &str, BOOL &digit);
};

class Operator;

class Express:public utils::RefrenceObj
{
public:
    Express(OpItem &op);
    virtual ~Express();

    void SetRecurse(BOOL r);
    void Insert(Express *expr);
    void Add(Express *expr);
    tExpress RefChildAt(int index);
    Express *ChildAt(int index);

    S32 ChildrenSize();

    OP_TYPE Type();

    inline OpItem &Value();

    void showItem();

    OpValue GenCmd(Operator& iOperator);

    OpValue ParseCmd(Operator& iOperator,
                     S32 pos = 0,
                     const CmdValue &recvCmd = CmdValue::Null());
private:
    OpItem    opItem;
    vector<Express *> children;
    BOOL      recurse;
};


#endif //__OPITEM_H__
