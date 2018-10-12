/*************************************************************************
	> File Name: ifconfig.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月25日 星期四 15时24分45秒
 ************************************************************************/
#ifndef __IFCONFIG_H__
#define __IFCONFIG_H__
#include <defines.h>
#include<stdio.h>

typedef char NetParm[MAX_NETADDR_LEN];

int SetIfAddr(const char *ifname, const char *Ipaddr, const char *mask,const char *gateway);
int GetIfAddr(const char* iface,char ipaddr[],char mask[]);
/*
 * 获取IP
 */
bool GetLocalIp(const char *iface,char ipaddr[]);
bool GetLocalIp(char ipaddr[]);

/*
 * 修改本机IP地址的函数
 */
int SetLocalIp( const char *ipaddr );

/*
 *获取子网掩码的函数
 */
bool GetLocalNetMask(const char *ifname,char netmask_addr[]);

/*
 * 修改子NETMASK的函数
 */
bool SetLocalNetMask(const char *szNetMask);

/*
 * 获去GateWay
 */
bool GetGateWay(char gateway[]);

/*
 * 设置网关
 */
int SetGateWay(const char *szGateWay);

/*
 * 获取系统中的网卡数量
 */
int GetNetCardCount(NetParm *ifnames);

/*
 *获取本机MAC地址函数
 */
bool GetLocalMac(char mac_addr[30]);

#endif//__IFCONFIG_H__
