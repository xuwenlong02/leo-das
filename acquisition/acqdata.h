/*************************************************************************
	> File Name: acqdata.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年03月13日 星期二 15时14分54秒
 ************************************************************************/
#ifndef __ACQDATA_H__
#define __ACQDATA_H__

#include <stdio.h>
#include <defines.h>
#include <time.h>
#include <json/json.h>
#include "modbus.h"

class AcqData
{
public:
    AcqData();
    AcqData(const OpValue& data_);
    AcqData& operator=(const OpValue& data_);


    void SetFlag(U8 flag_);
    void SetTime(time_t t);
    void SetId(U16 id);
//    void SetName(const char *name_);
//    void SetUnit(const char *unit_);
    time_t GetTime();
    U8 GetFlag();

    Json::Value ToJson();

private:
    void copydata(TYPE_DEFINE type, const OpValue::Udata &udata);

private:
    TYPE_DEFINE type;
    U8          flag;
    U16         itemid;

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
//        S8   str[MAX_VAR_LEN];
    };
    Udata  data;

//    S8     name[MAX_VAR_LEN];
//    S8     unit[MAX_VAR_LEN];
    time_t acqtime;

};

#endif //__ACQDATA_H__
