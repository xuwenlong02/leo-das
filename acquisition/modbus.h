#ifndef __MODBUS_H__
#define __MODBUS_H__

#include <stdio.h>
#include <defines.h>
#include <time.h>
#include <sys/time.h>
#include <logger.h>
#include <utils/shared_ptr.h>
#include <string>
#include <vector>

#define TRANS_GET_LEN(d)    (strlen(d))
#define TRANS_GET_V16(p)    (((p)[0]<<8)|(p)[1])
#define TRANS_GET_V32(p)    ((p)[0]<<24|(p)[1]<<16|(p)[2]<<8|(p)[3])
#define TRANS_PRINT(p,len)  \
{\
    char *info=(char*)malloc(3*len+1);\
    for (int i = 0; i < len; i++){\
    sprintf(&info[3 * i], "%02X ", (p)[i]); \
    }\
    info[3*len]='\0';\
    printf("%s\n",info); \
    free(info);\
}while (0)

/*接收结果*/
#define RECV_COMPLETE       0
#define RECV_NOT_COMPLETE   1
#define RECV_NOT_AVAILIABLE 2
#define RECV_FAILED         3
typedef S8  RECV_RESULT;

class OpValue;
class Express;
typedef utils::SharedPtr<Express> tExpress;
class ParseOperator;

class DataIterface
{
public:
    virtual std::string& GetName() = 0;
    virtual std::string& GetUnit() = 0;
    virtual U16         GetItemId() = 0;
    virtual S32 GetRegLength() = 0;
    virtual tExpress    GetSendExpr() = 0;
    virtual tExpress    GetRecvExpr() = 0;
    virtual tExpress    GetDataExpr() = 0;
    virtual F32  GetDataRate() = 0;
    virtual void SetName(const std::string& name) = 0;
    virtual void SetUnit(const std::string& unit) = 0;
    virtual void SetItemId(U16 id) = 0;
    virtual void ParseParams(const std::vector<std::string>& params) = 0;
    virtual void ParseDataExpr(const std::string& expr,F32 rate) = 0;
};

class OpInterface
{
public:
    virtual tExpress LocateFunction(const char* def,int funcId = -1) = 0;
    virtual const ID_DEV& DevAddr() = 0;
    virtual DataIterface *GetDataItem(U16 itemid) = 0;
    virtual DataIterface *GetSpecifyItem(U16 itemid) = 0;
};

template<typename T>
class StrandValue
{
public:
    StrandValue() {
        len = 0;
        cmd = NULL;
    }

    StrandValue(T *cmd_,S32 len_){
        construct(cmd_,len_);
    }

    StrandValue(const StrandValue &value) {
        construct(value.Cmd(),value.Length());
    }

    virtual ~StrandValue() {
        distruct();
    }

    static StrandValue Null() {
        return StrandValue();
    }

    BOOL IsEmpty() const{
        return len == 0;
    }

    BOOL operator !=(const StrandValue &value) const {
        return !operator ==(value);
    }

    BOOL operator ==(const StrandValue &value) const {
        if (len == value.Length()) {
            if (len == 0)
                return TRUE;
            return cmpdata(cmd,value.Cmd(),len);
        }
        return FALSE;
    }

    void getNext(S32 *next,T sub[],S32 len) {
        S32 i = 0;
        S32 j = -1;

        next[0] = -1;

        while (i < len-1) {
            if (j == -1 || sub[i] == sub[j]) {
                ++i;++j;
                next[i] = j;
            }
            else
                j = next[j];
        }
    }

    S32 Contains(const StrandValue &value) {
        S32 ipos,jpos;
        S32 *next = NULL;

        if (len < value.Length() || value.Length() == 0)
            return -1;
        next = (S32 *)malloc(value.Length()*sizeof(S32));
        getNext(next,value.Cmd(),value.Length());
        ipos = 0;
        jpos = 0;
        while (ipos < len &&
               jpos < value.Length()) {
            if (jpos == -1 || cmd[ipos] == value[jpos]) {
                ipos++;jpos++;
            }
            else
                jpos = next[jpos];
        }
        free(next);

        if (jpos == value.Length())
            return ipos;
        return -1;
    }

    inline void Show() {
        LOG_INFO("len = %d,cmd:",len);
        TRANS_PRINT(cmd,len);
    }

    inline T* Cmd() const{
        return cmd;
    }

    inline S32 Length() const {
        return len;
    }

    T* CmdAt(S32 pos) const{
        ASSERT((pos >=0) && (pos < len));
        return &cmd[pos];
    }

    T& operator [](S32 pos) const{
        ASSERT((pos >= 0) && (pos < len));
        return cmd[pos];
    }

    StrandValue & operator=(const StrandValue & value) {
        distruct();
        construct(value.Cmd(),value.Length());
        return *this;
    }

    StrandValue & Insert(S32 index,const T value) {
        if (cmd) {
            S32 len_ = len;
            T* newcmd = new T[len_+1];
            if (index > 0)
                copydata(&newcmd[0],cmd,index);
            newcmd[index] = value;
            copydata(&newcmd[index+1],&cmd[index],len_-index);
            distruct();
            cmd = newcmd;
            len = len_+1;
        }
        else {
            cmd = new T[1];
            cmd[0] = value;
            len = 1;
        }
        return *this;
    }

    StrandValue & operator +=(const StrandValue &value) {
        if (value == Null())
            return *this;

        if (cmd) {
            S32 len_ = len;
            T* newcmd = new T[len_+value.Length()];
            copydata(newcmd,cmd,len_);
            copydata(&newcmd[len_],value.Cmd(),value.Length());
            distruct();
            cmd = newcmd;
            len = len_+ value.Length();
        }
        else {
            cmd = new T[value.Length()];
            copydata(cmd,value.Cmd(),value.Length());
            len = value.Length();
        }

        return *this;
    }

    StrandValue & operator +=(const T value) {
        if (cmd) {
            S32 len_ = len;
            T* newcmd = new T[len_+1];
            copydata(newcmd,cmd,len_);
            copydata(&newcmd[len_],&value,1);
            distruct();
            cmd = newcmd;
            len = len_+1;
        }
        else {
            cmd = new T[1];
            cmd[0] = value;
            len = 1;
        }
        return *this;
    }

    StrandValue & operator -=(const StrandValue &value) {
        if (value == Null() || !cmd)
            return *this;

        S32 pos = Contains(value);
        return TrimLeft(pos);
    }

    StrandValue & operator -=(const T value) {
        if (!cmd)
            return *this;

        if (cmd[0] == value)
            return TrimLeft(1);
        return *this;
    }

    StrandValue& TrimLeft(S32 length) {
        if (length <= 0 )
            return *this;
        if (length >= len) {
            distruct();
            return *this;
        }
        copydata(cmd,&cmd[length],len-length);
        len -= length;
        return *this;
    }

private:
    inline void copydata(T* dst,const T* src,S32 len) {
        for (int i = 0;i < len;i++) {
            dst[i] = src[i];
        }
    }

    BOOL cmpdata(const T* dst,const T* src,S32 len) const{
        for (int i = 0;i < len;i++) {
            if (dst[i] != src[i])
                return FALSE;
        }
        return TRUE;
    }

    inline void construct(const T *cmd_,S32 len_) {
        len = len_;
        if (len_ <= 0) {
            cmd = NULL;
            return;
        }
        cmd = new T[len];
        copydata(cmd,cmd_,len);
    }

    inline void distruct() {
        if (cmd && len>0)
            delete[] cmd;
        cmd = NULL;
        len = 0;
    }

private:
    S32 len;
    T  *cmd;
};

typedef StrandValue<U8> CmdValue;
//typedef StrandValue<CustomValue> AutoValue;

#define AT_VALUE(pos,len) ((pos)*1000+(len))

class OpValue
{
public:
    OpValue() {
        type = TYPE_NONE;
        data.s64 = 0;
        next = NULL;
    }

    OpValue(const OpValue &value) {
        next = NULL;
//        operator =(value);
        *this = value;
    }

    static OpValue Null() {
        return OpValue();
    }

    OpValue& operator =(const OpValue & value) {
//        release();
//        next = value.next;
        type = value.type;
        copydata(value);
        OpValue *p = copynext(value.next);
        if (next)
            delete next;
        next = p;

        return *this;
    }

    BOOL IsEmpty() const{
        return ((type == TYPE_NONE) ||
                (type == TYPE_CMD && cmd.IsEmpty()) ||
                (type == TYPE_STR && data.str == NULL) ||
                (type == TYPE_POINTER && data.pointer == NULL));
    }

    BOOL operator!=(const OpValue &value) {
        return !operator ==(value);
    }

    BOOL operator!=(const TYPE_DEFINE type) const{
        return !operator ==(OpValue(type));
    }

    BOOL operator==(const OpValue &value) const {
        if (type == TYPE_NONE ||value.type==TYPE_NONE){
            if (type != TYPE_NONE ||value.type!=TYPE_NONE)
                return FALSE;
            return TRUE;
        }
        else if (IsInteger() && value.IsInteger())
            return Integer() == value.Integer();
        else if (IsFloat() && value.IsFloat())
            return (Float() - value.Float() > -0.000001) &&
                    (Float() - value.Float() < 0.000001);
        else if (type == TYPE_CMD && value.type == TYPE_CMD)
            return cmd == value.cmd;
        else if (type == TYPE_POINTER)
            return data.pointer==value.data.pointer;
        else
            return !strcmp(data.str,value.data.str);
    }

    OpValue(S8 v) {
        type = TYPE_S8;
        data.s8 = v;
        next = NULL;
    }

    OpValue(U8 v) {
        type = TYPE_U8;
        data.u8 = v;
        next = NULL;
    }

    OpValue(S16 v) {
        type = TYPE_S16;
        data.s16 = v;
        next = NULL;
    }

    OpValue(U16 v) {
        type = TYPE_U16;
        data.u16 = v;
        next = NULL;
    }

    OpValue(S32 v) {
        type = TYPE_S32;
        data.s32 = v;
        next = NULL;
    }

    OpValue(U32 v) {
        type = TYPE_U32;
        data.u32 = v;
        next = NULL;
    }

    OpValue(S64 v) {
        type = TYPE_S64;
        data.s64 = v;
        next = NULL;
    }

    OpValue(U64 v) {
        type = TYPE_U64;
        data.u64 = v;
        next = NULL;
    }

    OpValue(F32 v) {
        type = TYPE_F32;
        data.f32 = v;
        next = NULL;
    }

    OpValue(F64 v) {
        type = TYPE_F64;
        data.f64 = v;
        next = NULL;
    }

    OpValue(const S8* str) {
        type = TYPE_STR;
        int len = strlen(str);
        data.str = (S8*)malloc(len+1);
        strcpy(data.str,str);
        data.str[len] = '\0';
        next = NULL;
    }

    OpValue(void *p) {
        type == TYPE_POINTER;
        data.pointer = p;
        next = NULL;
    }

    OpValue(U8 *cmd_,S32 len) {
        type = TYPE_CMD;
        cmd = CmdValue(cmd_,len);
        next = NULL;
    }

    OpValue(const CmdValue& cmd_) {
        type = TYPE_CMD;
        cmd = CmdValue(cmd_.Cmd(),cmd_.Length());
        next = NULL;
    }

    /* operator=*/
    OpValue& operator =(S8 v) {
        release();
        type = TYPE_S8;
        data.s8 = v;
        next = NULL;
        return *this;
    }

    OpValue& operator =(U8 v) {
        release();
        type = TYPE_U8;
        data.u8 = v;
        next = NULL;
        return *this;
    }

    OpValue& operator =(S16 v) {
        release();
        type = TYPE_S16;
        data.s16 = v;
        next = NULL;
        return *this;
    }

    OpValue& operator =(U16 v) {
        release();
        type = TYPE_U16;
        data.u16 = v;
        next = NULL;
        return *this;
    }

    OpValue& operator =(S32 v) {
        release();
        type = TYPE_S32;
        data.s32 = v;
        next = NULL;
        return *this;
    }

    OpValue& operator =(U32 v) {
        release();
        type = TYPE_U32;
        data.u32 = v;
        next = NULL;
        return *this;
    }

    OpValue& operator =(S64 v) {
        release();
        type = TYPE_S64;
        data.s64 = v;
        next = NULL;
        return *this;
    }

    OpValue& operator =(U64 v) {
        release();
        type = TYPE_U64;
        data.u64 = v;
        next = NULL;
        return *this;
    }

    OpValue& operator =(F32 v) {
        release();
        type = TYPE_F32;
        data.f32 = v;
        next = NULL;
        return *this;
    }

    OpValue& operator =(F64 v) {
        release();
        type = TYPE_F64;
        data.f64 = v;
        next = NULL;
        return *this;
    }

    OpValue& operator =(S8* str) {
        release();
        type = TYPE_STR;
        int len = strlen(str);
        data.str = (S8*)malloc(len+1);
        strcpy(data.str,str);
        data.str[len] = '\0';
        next = NULL;
    }

    OpValue& operator =(void *p) {
        release();
        type = TYPE_POINTER;
        data.pointer = p;
        next = NULL;
        return *this;
    }

    OpValue& operator <<(const OpValue &value) {
        ASSERT(type != TYPE_STR && value.type != TYPE_POINTER) ;

        if (type != TYPE_CMD && type != TYPE_NONE) {
            cmd += (U8)Integer();
        }
//        if (type != TYPE_CMD)
//            data.cmd = new CmdValue();
        type = TYPE_CMD;
        if (value.type == TYPE_CMD)
            cmd += value.cmd;
        else if (value.type != TYPE_NONE)
            cmd += (U8)value.Integer();

        return *this;
    }

    OpValue& operator <<(const U8 value) {
        ASSERT(type != TYPE_STR &&
                type != TYPE_POINTER);

        if (type != TYPE_CMD && type != TYPE_NONE) {
            cmd += (U8)Integer();
        }
        cmd += value;

        return *this;
    }

    OpValue operator >>(const S32 at) {
        S32 pos = at / 1000;
        S32 len = at % 1000;
        return OpValue(GenCmd(pos,len));
    }

    CmdValue GenCmd(S32 pos_ = 0,S32 len_ = -1) {
        ASSERT(type != TYPE_STR &&
                type != TYPE_POINTER);

        if (type == TYPE_NONE)
            return CmdValue::Null();
        if (type != TYPE_CMD) {
            U8 c[1] ={(U8)Integer()};
            return CmdValue(c,1);
        }
        else if (pos_ < 0)
            return CmdValue::Null();
        else if(pos_ == 0 && len_ == -1) {
            return cmd;
        }
        else {
            if (len_ < 0 || pos_+len_ > cmd.Length() )
                len_ = cmd.Length()-pos_;
            return CmdValue(cmd.CmdAt(pos_),len_);
        }
    }

    virtual ~OpValue() {
        release();
    }

    BOOL IsString() const {
        return type == TYPE_STR;
    }

    BOOL IsDigital() const{
        return ((type > TYPE_NONE && type <= TYPE_F64) ||
                (type == TYPE_CMD && cmd.Length() == 1));
    }

    BOOL IsInteger() const {
        return (IsDigital() && !IsFloat());
    }

    BOOL IsFloat() const{
        return (type == TYPE_F32 ||
                type == TYPE_F64);
    }

    S32 Integer() const{
        ASSERT(((type > TYPE_NONE && type <= TYPE_F64) ||
                (type == TYPE_CMD && cmd.Length() == 1)));

        switch (type) {
        case TYPE_U8:
            return data.u8;
        case TYPE_S8:
            return data.s8;
        case TYPE_U16:
            return data.u16;
        case TYPE_S16:
            return data.s16;
        case TYPE_U32:
            return data.u32;
        case TYPE_S32:
            return data.s32;
        case TYPE_U64:
            return data.u64;
        case TYPE_S64:
            return data.s64;
        case TYPE_CMD:
            return cmd[0];
        }

        if (IsFloat())
            return (S64)Float();

        return 0;
    }

    F64 Float() const {
        ASSERT(type != TYPE_NONE &&
                type != TYPE_STR &&
                type != TYPE_POINTER);

        switch(type) {
        case TYPE_F32:
            return data.f32;
        case TYPE_F64:
            return data.f64;
        }
        if (IsInteger())
            return Integer();
        return 0;
    }

    OpValue *Next() {
        return next;
    }

    OpValue *AllocNext(const OpValue &next_) {
        if (next) {
            next->AllocNext(next_);
        }
        else {
            next = new OpValue(next_);
        }
    }

    OpValue operator +(const OpValue &value) {
        if (!IsDigital() || !value.IsDigital()) {
            CmdValue v1 = cmd;
            if (type != TYPE_CMD) {
                LOG_ERR("error value %u",type);
                return OpValue::Null();
            }

            if (value.IsDigital() ||(value.cmd.Length() == 1)) {
                for (S32 i = 0;i<v1.Length();i++) {
                    *(v1.CmdAt(i)) += value.Float();
                }
                return v1;
            }
            else if (value.type == TYPE_CMD) {
                if (v1.Length() != value.cmd.Length()) {
                    LOG_ERR("error value %u",value.type);
                    return OpValue::Null();
                }
                for (S32 i = 0;i<v1.Length();i++) {
                    *(v1.CmdAt(i)) += *(value.cmd.CmdAt(i));
                }
                return v1;
            }
            else {
                LOG_ERR("error value %u",value.type);
                return OpValue::Null();
            }
        }

        if (IsInteger() && value.IsInteger())
            return Integer()+value.Integer();
        else {
            F64 result = 0;
            if (IsInteger())
                result = Integer();
            else
                result = Float();

            if (value.IsInteger())
                result +=value.Integer();
            else
                result +=value.Float();
            return result;
        }
    }

    OpValue operator -(const OpValue &value) {
        if (!IsDigital() || !value.IsDigital()) {
            CmdValue v1 = cmd;
            if (type != TYPE_CMD) {
                LOG_ERR("error value %u",type);
                return OpValue::Null();
            }

            if (value.IsDigital() ||(value.cmd.Length() == 1)) {
                for (S32 i = 0;i<v1.Length();i++) {
                    *(v1.CmdAt(i)) -= value.Float();
                }
                return v1;
            }
            else if (value.type == TYPE_CMD) {
                if (v1.Length() != value.cmd.Length()) {
                    LOG_ERR("error value %u",value.type);
                    return OpValue::Null();
                }
                for (S32 i = 0;i<v1.Length();i++) {
                    *(v1.CmdAt(i)) -= *(value.cmd.CmdAt(i));
                }
                return v1;
            }
            else {
                LOG_ERR("error value %u",value.type);
                return OpValue::Null();
            }
        }

        if (IsInteger() && value.IsInteger())
            return Integer()-value.Integer();
        else {
            F64 result = 0;
            if (IsInteger())
                result = Integer();
            else
                result = Float();

            if (value.IsInteger())
                result -=value.Integer();
            else
                result -=value.Float();
            return result;
        }
    }

    OpValue operator *(const OpValue &value) {
        if (!IsDigital() || !value.IsDigital()) {
            CmdValue v1 = cmd;
            if (type != TYPE_CMD) {
                LOG_ERR("error value %u",type);
                return OpValue::Null();
            }

            if (value.IsDigital() ||(value.cmd.Length() == 1)) {
                for (S32 i = 0;i<v1.Length();i++) {
                    *(v1.CmdAt(i)) *= value.Float();
                }
                return v1;
            }
            else if (value.type == TYPE_CMD) {
                if (v1.Length() != value.cmd.Length()) {
                    LOG_ERR("error value %u",value.type);
                    return OpValue::Null();
                }
                for (S32 i = 0;i<v1.Length();i++) {
                    *(v1.CmdAt(i)) *= *(value.cmd.CmdAt(i));
                }
                return v1;
            }
            else {
                LOG_ERR("error value %u",value.type);
                return OpValue::Null();
            }
        }

        if (IsInteger() && value.IsInteger())
            return Integer()*value.Integer();
        else {
            F64 result = 0;
            if (IsInteger())
                result = Integer();
            else
                result = Float();

            if (value.IsInteger())
                result *=value.Integer();
            else
                result *=value.Float();
            return result;
        }
    }

    OpValue operator /(const OpValue &value) {
        if (!IsDigital() || !value.IsDigital()) {
            CmdValue v1 = cmd;
            if (type != TYPE_CMD) {
                LOG_ERR("error value %u",type);
                return OpValue::Null();
            }

            if (value.IsDigital() ||(value.cmd.Length() == 1)) {
                for (S32 i = 0;i<v1.Length();i++) {
                    *(v1.CmdAt(i)) /= value.Float();
                }
                return v1;
            }
            else if (value.type == TYPE_CMD) {
                if (v1.Length() != value.cmd.Length()) {
                    LOG_ERR("error value %u",value.type);
                    return OpValue::Null();
                }
                for (S32 i = 0;i<v1.Length();i++) {
                    *(v1.CmdAt(i)) /= *(value.cmd.CmdAt(i));
                }
                return v1;
            }
            else {
                LOG_ERR("error value %u",value.type);
                return OpValue::Null();
            }
        }

        if (IsInteger() && value.IsInteger()) {
            if (value.Integer() == 0) {
                LOG_ERR("/ divisor is 0");
                return Integer();
            }
            return Integer()/value.Integer();
        }
        else {
            F64 result = 0;
            if (IsInteger())
                result = Integer();
            else
                result = Float();

            if (value.IsInteger()){
                if (value.Integer() == 0) {
                    LOG_ERR("/ divisor is 0");
                    return result;
                }
                result /=value.Integer();
            }
            else {
                if (value.Float() == 0) {
                    LOG_ERR("/ divisor is 0");
                    return result;
                }
                result /=value.Float();
            }
            return result;
        }
    }

    OpValue operator %(const OpValue &value) {
        if (!IsDigital() || !value.IsDigital() ||
                value.IsFloat() || value.IsFloat()) {
            LOG_ERR("error value %u",type);
            return OpValue::Null();
        }

        if (value.Integer() == 0) {
            LOG_ERR("divisor is 0");
            return Integer();
        }
        if (IsInteger() && value.IsInteger())
            return Integer()%value.Integer();
        else {
            U32 result = Integer();

            result %= value.Integer();

            return result;
        }
    }

    TYPE_DEFINE type;
    union Udata{
        Udata() {
            s64 = 0;
        }
        ~Udata() {

        }

        S8   s8;
        U8   u8;
        S16  s16;
        U16  u16;
        S32  s32;
        U32  u32;
        S64  s64;
        U64  u64;
        F32  f32;
        F64  f64;
        S8*  str;
        void *pointer;
    };
    Udata data;
    CmdValue cmd;

    OpValue *next;

private:
    void release() {
        if (next) {
            delete next;
            next = NULL;
        }
        if (type == TYPE_STR){
            if (data.str)
                free(data.str);
            data.str = NULL;
        }
//        if(type == TYPE_CMD) {
//            if (data.cmd)
//                delete data.cmd;
//            data.cmd=NULL;
//        }
        type = TYPE_NONE;

    }

    void copydata(const OpValue & value) {
        switch (value.type) {
        case TYPE_U8:
            data.u8 = value.data.u8;
            break;
        case TYPE_S8:
            data.s8 = value.data.s8;
            break;
        case TYPE_U16:
            data.u16 = value.data.u16;
            break;
        case TYPE_S16:
            data.s16 = value.data.s16;
            break;
        case TYPE_U32:
            data.u32 = value.data.u32;
            break;
        case TYPE_S32:
            data.s32 = value.data.s32;
            break;
        case TYPE_U64:
            data.u64 = value.data.u64;
            break;
        case TYPE_S64:
            data.s64 = value.data.s64;
            break;
        case TYPE_F32:
            data.f32 = value.data.f32;
            break;
        case TYPE_F64:
            data.f64 = value.data.f64;
            break;
        case TYPE_STR:{
            if (data.str)
                free(data.str);
            int len = strlen(value.data.str);
            data.str = (S8*)malloc(len+1);
            strcpy(data.str,value.data.str);
            data.str[len] = '\0';
            break;
        }
        case TYPE_POINTER:
            data.pointer = value.data.pointer;
            break;
        case TYPE_CMD: {
            cmd = value.cmd;
        }
            break;
        }

    }

    OpValue *copynext(OpValue *p) {
        if (!p)
            return NULL;
        OpValue *b = new OpValue();
        b->type = p->type;
        b->copydata(*p);
        b->next = copynext(p->next);
        return b;
    }
};

#endif
