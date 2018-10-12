/*************************************************************************
	> File Name: opitem.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年02月23日 星期五 15时01分24秒
 ************************************************************************/

#include <stdio.h>
#include <string>
#include <defines.h>
#include <stack>
#include <vector>
#include <queue>
#include <logger.h>
#include <string.h>
#include <stdlib.h>
#include <utils/custom.h>
#include "opitem.h"

OpItem::OpItem(const S8 ch) {
    type = OP_NONE;
    params = 0;
    switch(ch) {
    case '(':
    case ')':
        type = OP_PARENTHESS;
        break;
    case '[':
    case ']':
        type = OP_BRACKET;
        break;
    case '{':
    case '}':
        type = OP_BRACE;
        break;
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '&':
        type = OP_BASE;
        break;
    default:
        type = OP_COMMA;
        break;
    }

    op.ch = ch;
}

OpItem::OpItem(const S8 *var,BOOL isfunc) {
#define STREQUAL(str) !strcmp(var,str)

    type = OP_NONE;
    params = 0;
    if (!isfunc) {
        bool isfloat = false;
        if (utils::isDecimal(var,isfloat)) {
            type = OP_NUM;
            if (isfloat) {
                digit =atof(var);
            }
            else {
                digit =atoi(var);
            }
        }
        else if (utils::isHexDigit(var)) {
            type = OP_NUM;
            U32 v = utils::strtohex(var);
            if (v<=255)
                digit = (U8)v;
            else
                digit = v;
        }
    }
    else {
        if (STREQUAL("pos") ||
                STREQUAL("len") ||
                STREQUAL("addr"))
            type = OP_VAR;
        else if(STREQUAL("chk")  ||
                STREQUAL("ref"))
            type = OP_FUNC;
        else if(STREQUAL("ditch") || STREQUAL("fill"))
            type = OP_KEYFUNC;
        else if (utils::isInteger(var))
            type = OP_VAR;
        else
            type = OP_FUNC;

        memset(op.var,0,MAX_VAR_LEN);
        strncpy(op.var,var,MAX_VAR_LEN);
    }


#undef STREQUAL
}

OpItem::OpItem(const OpItem &item)
{
    type = OP_NONE;
    copyitem(item);
}

OpItem::OpItem()
{
    params = 0;
    type = OP_NONE;
}

OpItem::~OpItem()
{
//    if (type == OP_VAR ||type == OP_FUNC ||type == OP_KEYFUNC)
//        free(op.var);
    release();
}

OpItem& OpItem::operator =(const OpItem &item)
{
    release();
    copyitem(item);
    return *this;
}

void OpItem::copyitem(const OpItem &item) {
    params = item.params;
    type = item.type;
    if (type == OP_VAR ||type == OP_FUNC || type == OP_KEYFUNC) {
        memset(op.var,0,MAX_VAR_LEN);
        strncpy(op.var,item.op.var,MAX_VAR_LEN);
    }
    else {
        if (type == OP_NUM) {
            digit = item.digit;
        }
        else
            op.ch = item.op.ch;
    }
}

/*外 优先级 ,数字优先级设为-1 */
int OpItem::priority() {
    switch (type) {
    case OP_COMMA: /* 逗号或空格 */
        return 2;
    case OP_BASE://基本运算符，+-×/等
        if (op.ch == '+' || op.ch == '-')
            return 4;
        else if (op.ch == '*' || op.ch == '/' || op.ch == '%')
            return 6;
        else if (op.ch == '&')
            return 7;
    case OP_VAR:
    case OP_FUNC:
    case OP_KEYFUNC:
        return 8;
    case OP_BRACKET: /* 中括号 */
    case OP_PARENTHESS: /* 小括号 */
    case OP_BRACE:
        if (op.ch == '(' || op.ch == '[' || op.ch == '{')
            return 0;
        else
            return 10;
    default:
        return -1;
    }
}

void OpItem::show() {
    if (type == OP_NUM) {
        if (digit.IsInteger())
            LOG_DEBUG("%02X ",(U32)digit.Integer());
        else
            LOG_DEBUG("%0.2f",digit.Float());
    }
    else if (type == OP_BASE ||
             type == OP_BRACKET ||
             type == OP_BRACE ||
             type == OP_PARENTHESS ||
             type == OP_COMMA)
        LOG_DEBUG("%c ",op.ch);
    else if (type == OP_VAR ||
             type == OP_FUNC ||
             type == OP_KEYFUNC)
        LOG_DEBUG("%s ",op.var);
}

void OpItem::release()
{
//    if (type == OP_NUM && (op.digit != NULL)) {
//        delete op.digit;
//        op.digit = NULL;
//    }
    type = OP_NONE;
}

BOOL ExprOperator::IsAvailiable(const string &express)
{
    string str = express;
    tExpress expr = ExprOperator::WrapExpress(str);
    return expr.valid();
}

tExpress ExprOperator::WrapExpress(const string &str)
{
    vector<OpItem> queue_list;
    stack<OpItem> stackOperators;

    queue_list = ExprOperator::GetOpts(str);

    if (!ExprOperator::generatePostExpress(queue_list,stackOperators))
        return NULL;

    Express *expr = ExprOperator::parseExpress(stackOperators);

    return expr;
}

BOOL ExprOperator::generatePostExpress(vector<OpItem> &queue_list, stack<OpItem> &stackOperators)
{
    stack<OpItem> stackOpera;
    stack<int>    params;

    while (!queue_list.empty()) {
        OpItem out = queue_list.front();

        queue_list.erase(queue_list.begin());
//        out.show();

        if (out.type == OP_NUM) {
            stackOperators.push(out);
        }
        else if (out.type == OP_COMMA) {
            while (!stackOpera.empty()) {
                OpItem top = stackOpera.top();

                if (out.priority() <= top.priority()) {
                    stackOperators.push(top);
                    stackOpera.pop();
                }
                else {
                    break;
                }
            }
            topNum(params,1);
        }
        else if (out.type == OP_VAR ||
                 out.type == OP_FUNC ||
                 out.type == OP_KEYFUNC) {
            if (queue_list.empty() ||
                    (!(((OpItem&)queue_list.front()).type == OP_PARENTHESS  &&
                       ((OpItem&)queue_list.front()).op.ch == '('))) {
                out.params = 0;
                stackOperators.push(out);
            }
            else {
                stackOpera.push(out);
                topNum(params,0);
            }
        }
        else if (out.type == OP_PARENTHESS) {
            if (out.op.ch == '(') {
                stackOpera.push(out);
            }
            else {
                while(!stackOpera.empty() ){
                    OpItem &top = stackOpera.top();
                    if (top.type != OP_PARENTHESS) {
                        stackOperators.push(top);
                        stackOpera.pop();
                    }
                    else {
                        stackOpera.pop();
                        break;
                    }
                }

                if ( !stackOpera.empty()) {
                    OpItem &top = stackOpera.top();
                    if (top.type == OP_VAR ||
                            top.type == OP_FUNC ||
                            top.type == OP_KEYFUNC ||
                            top.type == OP_BRACKET ||
                            top.type == OP_BRACE) {
                        topNum(params,1);
                        top.params = topNum(params,-1);

                        stackOperators.push(top);
                        stackOpera.pop();
                    }
                }
            }
        }
        else if (out.type == OP_BRACKET) { /* 中括号 */
            if ( out.op.ch == '[' ) {
                stackOpera.push(out);
                topNum(params,0);
            }
            else { //']','}'
                while(!stackOpera.empty() ){
                    OpItem &top = stackOpera.top();
                    if (top.type != OP_BRACKET) {
                        stackOperators.push(top);
                        stackOpera.pop();
                    }
                    else {
                        topNum(params,1);
                        top.params = topNum(params,-1);

                        stackOperators.push(top);
                        stackOpera.pop();
                        break;
                    }
                }
            }
        }
        else if (out.type == OP_BRACE) { /* 大括号 */
            if ( out.op.ch == '{') {
                stackOpera.push(out);
                topNum(params,0);
            }
            else { //']','}'
                while(!stackOpera.empty() ){
                    OpItem &top = stackOpera.top();
                    if (top.type != OP_BRACE) {
                        stackOperators.push(top);
                        stackOpera.pop();
                    }
                    else {
                        topNum(params,1);
                        top.params = topNum(params,-1);

                        stackOperators.push(top);
                        stackOpera.pop();
                        break;
                    }
                }
            }
        }
        else if (out.type == OP_BASE) {
            while (!stackOpera.empty()) {
                OpItem &top = stackOpera.top();

                if (out.priority() <= top.priority()) {
                    stackOperators.push(top);
                    stackOpera.pop();
                }
                else {
                    break;
                }
            }

            if (out.op.ch != '&')
                out.params = 2;
            else
                out.params = 1;
            stackOpera.push(out);
        }
    }

    while (!stackOpera.empty()) {
        OpItem &top = stackOpera.top();
        if (top.type == OP_VAR ||
                top.type == OP_FUNC ||
                top.type == OP_KEYFUNC ||
                top.type == OP_BRACKET||
                top.type == OP_BRACE) {
            topNum(params,1);
            top.params = topNum(params,-1);
        }
        stackOperators.push(top);
        stackOpera.pop();
    }

    return !stackOperators.empty();
}

Express *ExprOperator::parseExpress(stack<OpItem> &stackOperators)
{

    if (stackOperators.empty()) {
        return NULL;
    }

    OpItem item = stackOperators.top();
    stackOperators.pop();

    if (item.type == OP_NUM) {
        return new Express(item);
    }
    else if (item.type == OP_VAR ||
             item.type == OP_FUNC ||
             item.type == OP_KEYFUNC ||
             item.type == OP_BRACKET ||
             item.type == OP_BRACE){
        Express *expr = new Express(item);

        Express *child = NULL;
        int params = 0;
        while (params++ < item.params){
            if ((child = parseExpress(stackOperators)))
                expr->Insert(child);
            else {
                LOG_ERR("parse error,params is less (%d < %d)",params,item.params);
                delete expr;
                return NULL;
            }
        }

        return expr;
    }
    else if (item.type == OP_BASE) {
        Express *expr = new Express(item);

        //'+','-','*','/'
        Express *child = NULL;

        int params = 0;
        while (params++ < item.params){
            if ((child = parseExpress(stackOperators)))
                expr->Insert(child);
            else {
                if (params == 1 && item.op.ch == '-')
                    break;
                LOG_ERR("parse error,params is less (%d < 2)",params);
                delete expr;
                return NULL;
            }
        }
        return expr;
    }
    else {
        LOG_ERR("parse error,no this operator: ");
        item.show();
        return NULL;
    }
}

int ExprOperator::topNum(stack<int> &params, int add)
{
    if (add == -1) {
        if (!params.empty()) {
            int num = params.top();
            params.pop();
            return num;
        }
        return -1;
    }
    else if (add == 0) {
        int num = 0;
        params.push(num);
        return num;
    }
    else {
        if (params.empty())
            return -1;
        int &num = params.top();
        num +=add;
        return num;
    }
}

vector<OpItem> ExprOperator::GetOpts(const string &str)
{
//    queue<OpItem> queue_list;
    vector<OpItem> queue_list;
    int i = 0;
    int charator  = -1;
    int parentheses = 0;
    int  brackets = 0;
    int brace = 0;
    BOOL  isfunc = FALSE;

//    str = utils::trim(str);
    while (i <str.length() && str[i] == ' ')
        i++;

    while (i < str.length()) {
        char c  = str.c_str()[i];

        if (utils::isTerminator(c)) {

            if (charator != -1 ){

                OpItem op(str.substr(charator,i-charator).c_str(), isfunc);
                queue_list.push_back(op);

                isfunc = FALSE;
                charator = -1;
            }

            if (c == '(' || c == ')' || c == '[' || c== ']'||c == '{' ||c == '}'
                    ||c=='+'||c=='-'||c == '%'||c=='*'||c=='/' || c== '&') {
                OpItem op(c);

                if (!queue_list.empty()) {
                    OpItem &top = queue_list.back();
                    if (top.type == OP_COMMA && (c == ')' || c==']' || c== '}'))
                        queue_list.pop_back();
                }
                queue_list.push_back(op);
                if (c == '(') {
                    parentheses++;
                }
                else if(c == ')') {
                    parentheses--;
                }
                else if (c == '{') {
                    brace++;
                }
                else if (c == '}') {
                    brace--;
                }
                else if (c == '[') {
                    brackets++;
                }
                else if(c == ']') {
                    brackets--;
                }
                else if(c== '&') {
                    isfunc = TRUE;
                }
            }
            else if (c == '$') {
                isfunc = TRUE;
            }
            else{
                //空格
                if ((brackets == 0) && (parentheses == 0) && (brace == 0)) {
                    charator = -1;
                    break;
                }
                else {
                    if (!queue_list.empty()) {
                        OpItem& top = queue_list.back();
                        if (!(top.type == OP_COMMA ||
                                (top.type == OP_BRACKET && top.op.ch == '[') ||
                                (top.type == OP_PARENTHESS && top.op.ch == '(') ||
                              (top.type == OP_BRACE && top.op.ch == '{'))){
                            OpItem op(c);
                            queue_list.push_back(op);
                        }
                    }
                }
            }
        }
        else{
            if (charator == -1)
                charator = i;
        }
        i++;
    }
    if (charator != -1 ){
        OpItem op(str.substr(charator,i-charator).c_str(),isfunc);
        queue_list.push_back(op);
    }
//    str = str.erase(0,i);

    return queue_list;
}

Express::Express(OpItem &op){
    recurse = true;
    opItem = op;
    children.clear();
}

Express::~Express()
{
    if (recurse) {
        for (int i = 0;i < children.size();i++) {
            delete (Express*)children.at(i);
        }
    }
    children.clear();
}

void Express::SetRecurse(BOOL r)
{
    recurse = r;
}

void Express::Insert(Express *expr)
{
    if (!expr) {
        LOG_ERR("null expr");
        return;
    }
    children.insert(children.begin(),expr);
}

void Express::Add(Express *expr)
{
    if (!expr) {
        LOG_ERR("null expr");
        return;
    }
    children.push_back(expr);
}

tExpress Express::RefChildAt(int index)
{
    return ChildAt(index);
}

Express *Express::ChildAt(int index)
{
    if (index < children.size())
        return children.at(index);
    return NULL;
}

S32 Express::ChildrenSize()
{
    return children.size();
}

OP_TYPE Express::Type()
{
    return opItem.type;
}

OpItem& Express::Value()
{
    return opItem;
}

void Express::showItem()
{
    opItem.show();

    for (int i = 0;i < children.size();i++) {
        ((Express*)children.at(i))->showItem();
    }
}

OpValue Express::GenCmd(Operator &iOperator)
{
    return ParseCmd(iOperator);
}

OpValue Express::ParseCmd(Operator &iOperator,S32 pos,const CmdValue &recvCmd)
{
    if (Type() == OP_NUM) {
        return Value().digit;
    }
    else if (Type() == OP_BASE) {
        if (Value().op.ch == '&') {
            if( 0 == ChildrenSize())
                return OpValue::Null();
            return ChildAt(0)->Value().op.var;
        }
        else {
            OpValue v1;
            OpValue v2 ;

            do{
                if (ChildrenSize() == 2) {
                    v1 = ChildAt(0)->ParseCmd(iOperator,pos);
                    v2 = ChildAt(1)->ParseCmd(iOperator,pos);
                }
                else if(ChildrenSize() == 1) {
                    v1 = 0;
                    v2 = ChildAt(0)->ParseCmd(iOperator,pos);
                }
                else
                {
                    LOG_ERR("%c:error params count",Value().op.ch);
                    return OpValue::Null();
                }
            }while(0);

            return iOperator.BaseCall(Value().op.ch,v1,v2);
        }
    }
    else if (Type() == OP_VAR) {
        if (utils::isInteger(Value().op.var)) {
            S32 v = atoi(Value().op.var);
            if (v == 0)
                return iOperator.GetParam(0);
            return iOperator.GetParam(v-1);
        }
        else{
            tExpress expr = iOperator.FindDefinition(Value().op.var);
            if (expr.valid()) {
                return expr->ParseCmd(iOperator,pos,recvCmd);
            }
            else {
                return iOperator.VarCall(Value().op.var,pos,recvCmd);
            }
        }

    }
    else if (Type() == OP_FUNC) {
        Operator::AutoOperator autoOp(iOperator,Operator::AUTO_PARAM,1);
        for (S32 i = 0;i < ChildrenSize();i++) {
            OpValue value;
            if (ChildAt(i)->Type() != OP_KEYFUNC)
                value =
                    ChildAt(i)->ParseCmd(iOperator,pos);
            else
                value =
                    ChildAt(i)->ParseCmd(iOperator,pos,recvCmd);
            if (value.IsEmpty()){
                LOG_DEBUG("func %s:param %d caculate error",Value().op.var,i);
                return value;
            }
            autoOp.AddList(value);
        }

        autoOp.StackAll();

        tExpress expr = iOperator.FindDefinition(Value().op.var);
        if (expr.valid()) {
            return expr->ParseCmd(iOperator,pos,recvCmd);
        }
        else
            return iOperator.FunctionCall(Value().op.var,pos,recvCmd);
    }
    else if (Type() == OP_KEYFUNC ) {
        if ( ChildrenSize() < 1) {
            LOG_ERR("keyfunc parameter is less");
            return OpValue::Null();
        }
        Operator::AutoOperator autoOp(iOperator,Operator::AUTO_PARAM);
        OpValue v1 =
                ChildAt(0)->ParseCmd(iOperator,pos);

        if (!v1.IsInteger()) {
            LOG_ERR("ditch first parameter must be a integer");
            return OpValue::Null();
        }
        if (!strcmp(Value().op.var,"ditch")) {
            OpValue get = iOperator.KeyFunctionCall(Value().op.var,v1.Integer(),recvCmd);
            iOperator.AddParam(get);
            if (ChildrenSize() > 1 ) {
                ChildAt(1)->ParseCmd(iOperator,pos,recvCmd);
            }
            return get;
        }
        else if (!strcmp(Value().op.var,"fill")) {
            OpValue fill;
            if (ChildrenSize() > 1 ) {
                fill = ChildAt(1)->ParseCmd(iOperator,pos,recvCmd);
            }
            iOperator.AddParam(fill);
            if (ChildrenSize() >= 3) {
                OpValue v2 =
                        ChildAt(2)->ParseCmd(iOperator,pos);
                if (v2.IsInteger())
                    iOperator.AddParam(v2);
            }
            return iOperator.KeyFunctionCall(Value().op.var,v1.Integer(),recvCmd);
        }

        return OpValue::Null();
    }
    else if (Type() == OP_BRACKET) {
        S32 start = 0;
        Operator::AutoOperator autoOp(iOperator,Operator::AUTO_CMD);
        CmdValue recv = recvCmd;
        /*
         * [] 表达式逆向运算
         *
         */
        for (int num = 0; num < ChildrenSize();num++) {
            OpValue value =
                    ChildAt(num)->ParseCmd(iOperator,start,recv);

            if (value.IsEmpty()) {
                LOG_DEBUG("[]:param %d caculate error",num);
                return OpValue::Null();
            }

            /* [] 逆向计算 */
            if (!recv.IsEmpty()){
                CmdValue cmdValue = value.GenCmd();
                S32 shift = recv.Contains(cmdValue);

                if (shift == -1) {
                    LOG_DEBUG("[]:param %d not match",num);
                    return OpValue::Null();
                }
                if (num != 0) {
                    if (shift == cmdValue.Length())
                        recv.TrimLeft(shift);
                }
                else
                    recv.TrimLeft(shift);

                if (recv.IsEmpty())
                    break;
            }
            start += iOperator.InsertCmd(num,value);
        }
        OpValue result = iOperator.GetCmd();
        return result;
    }
    else if (Type() == OP_BRACE) {
        OpValue result;
        for (S32 i = 0;i < ChildrenSize();i++) {
            OpValue value;
            if (ChildAt(i)->Type() != OP_KEYFUNC)
                value =
                    ChildAt(i)->ParseCmd(iOperator,pos);
            else
                value =
                    ChildAt(i)->ParseCmd(iOperator,pos,recvCmd);
            if (value.IsEmpty()){
                LOG_DEBUG("func %s:param %d caculate error",Value().op.var,i);
                return result;
            }
            if (result.IsEmpty())
                result = value;
            else {
                result.AllocNext(value);
            }
        }
        return result;
    }
    else {
        LOG_ERR("error operator %u",Type());
        return OpValue::Null();
    }
}
