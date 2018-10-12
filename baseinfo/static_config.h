/*************************************************************************
	> File Name: config.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月23日 星期二 11时10分40秒
 ************************************************************************/
#ifndef __STATIC_CONFIG_H__
#define __STATIC_CONFIG_H__

#include <stdio.h>
#include <defines.h>
#include <transport/transport.h>

/*
 * 数据中心
 */
typedef struct{
    S8  netaddr[MAX_NETADDR_LEN];
    U16 port;
}datacenter_t;

typedef struct
{
    S8   id[MAX_ID_LEN];
    U32  heartbeat;
    S8   secretkey[MAX_SECRETKEY_LEN];
    datacenter_t datacenter;
}gateway_config_t;


#endif//__STATIC_CONFIG_H__
