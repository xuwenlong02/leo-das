/*************************************************************************
	> File Name: defines.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月17日 星期三 15时36分49秒
 ************************************************************************/
#ifndef __GLOBAL_DEFINES__
#define __GLOBAL_DEFINES__

#include "macro.h"

typedef char              S8;
typedef unsigned char     U8;
typedef short             S16;
typedef unsigned short    U16;
typedef int               S32;
typedef unsigned int      U32;
typedef __uint64_t        U64;
typedef __int64_t         S64;
typedef float             F32;
typedef double            F64;

#ifdef __cplusplus
typedef bool              BOOL;
#define TRUE              true
#define FALSE             false
#else
typedef unsigned char     BOOL;
#define TRUE              1
#define FALSE             0
#endif

typedef void *            HANDLE;

typedef enum
{
    TYPE_NONE=0,
    TYPE_U8,
    TYPE_S8,
    TYPE_U16,
    TYPE_S16,
    TYPE_U32,
    TYPE_S32,
    TYPE_U64,
    TYPE_S64,
    TYPE_F32,
    TYPE_F64,
    TYPE_STR,
    TYPE_POINTER,
    TYPE_CMD

}TYPE_DEFINE;

typedef struct
{
    TYPE_DEFINE type;
    union{
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
        S8   str[8];
    }data;
}UnionData;

//环境变量
#define CONFIG_DIR_ENV    "DAS_CONFIG_DIR"
#define VERSION           "2.2.0"

#define MAX_ID_LEN              20
#define MAX_TYPE_LEN            20
#define MAX_VAR_LEN             32
#define MAX_CMD_LEN             256
#define MAX_COND_LEN            256

#define MAX_NAME_LEN            128
#define MAX_SECRETKEY_LEN       128

#define MAX_RECV_SIZE        512
#define MAX_NETADDR_LEN      128
#define MAX_SERIALADDR_LEN   30
#define MAX_REQUEST_SIZE     4096


#include "devid.h"

#endif//__GLOBAL_DEFINES__
