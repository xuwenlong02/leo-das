/*************************************************************************
	> File Name: config.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月19日 星期五 11时50分51秒
 ************************************************************************/
#include <defines.h>
#include <vector>

using namespace std;

#define MAX_ID_LEN             20
#define MAX_TYPE_LEN             20
#define MAX_NETADDR_ADDR       128

typedef struct{
    S8  id[MAX_ID_LEN];
    S8  type[MAX_TYPE_LEN];
    S8  netaddr[MAX_NETADDR_ADDR];
    U16 port;
}datacenter_t;

typedef struct
{
    S8   id[MAX_ID_LEN];
    U32  heartbeat;
    datacenter_t datacenter;
}gateway_config_t;


extern BOOL read_config_baseinfo(gateway_config_t *config);
