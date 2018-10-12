/*************************************************************************
	> File Name: acqdata.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年03月13日 星期二 15时15分52秒
 ************************************************************************/

#include <stdio.h>
#include <time.h>
#include <utils/custom.h>
#include "acqdata.h"

AcqData::AcqData()
{
    type = TYPE_NONE;
    flag = 0;
    itemid = 0;
    data.f64 = 0;
    acqtime = 0;//time(NULL);

}

AcqData::AcqData(const OpValue &data_) {
    acqtime = time(NULL);
    type = data_.type;
    flag = 0;
    copydata(data_.type,data_.data);
}

AcqData &AcqData::operator =(const OpValue &data_)
{
    flag = 0;
    type = data_.type;
    copydata(data_.type,data_.data);
    return *this;
}

void AcqData::SetFlag(U8 flag_)
{
    flag = flag_;
}

void AcqData::SetTime(time_t t)
{
    acqtime = t;
}

void AcqData::SetId(U16 id)
{
    itemid = id;
}

//void AcqData::SetName(const char *name_)
//{
//    strncpy(name,name_,MAX_VAR_LEN);
//}

//void AcqData::SetUnit(const char *unit_)
//{
//    strncpy(unit,unit_,MAX_VAR_LEN);
//}

time_t AcqData::GetTime()
{
    return acqtime;
}

U8 AcqData::GetFlag()
{
    return flag;
}

Json::Value AcqData::ToJson()
{
    Json::Value value;

    value["acqid"] = itemid;
//    value["name"] = name;
//    value["unit"] = unit;
//    value["time"] = utils::timetostr(acqtime);

    switch(type) {
    case TYPE_S8:
        value["value"]=data.s8;
        break;
    case TYPE_U8:
        value["value"] = data.u8;
        break;
    case TYPE_S16:
        value["value"] = data.s16;
        break;
    case TYPE_U16:
        value["value"] = data.u16;
        break;
    case TYPE_S32:
        value["value"] = data.s32;
        break;
    case TYPE_U32:
        value["value"] = data.u32;
        break;
    case TYPE_S64:
        value["value"] = data.s64;
        break;
    case TYPE_U64:
        value["value"] = data.u64;
        break;
    case TYPE_F32:
        value["value"] = data.f32;
        break;
    case TYPE_F64:
        value["value"] = data.f64;
        break;
//    case TYPE_STR:
//        value["value"] = data.str;
//        break;
    default:
        value["value"] = data.f64;
        break;
    }
    return value;
}

void AcqData::copydata(TYPE_DEFINE type,const OpValue::Udata &udata)
{
    switch (type) {
    case TYPE_U8:
        data.u8 = udata.u8;
        break;
    case TYPE_S8:
        data.s8 = udata.s8;
        break;
    case TYPE_U16:
        data.u16 = udata.u16;
        break;
    case TYPE_S16:
        data.s16 = udata.s16;
        break;
    case TYPE_U32:
        data.u32 = udata.u32;
        break;
    case TYPE_S32:
        data.s32 = udata.s32;
        break;
    case TYPE_U64:
        data.u64 = udata.u64;
        break;
    case TYPE_S64:
        data.s64 = udata.s64;
        break;
    case TYPE_F32:
        data.f32 = udata.f32;
        break;
    case TYPE_F64:
        data.f64 = udata.f64;
        break;
//    case TYPE_STR:{
//        strncpy(data.str,udata.str,MAX_VAR_LEN-1);
//        data.str[MAX_VAR_LEN] = '\0';
//        break;
//    }
    }
}
