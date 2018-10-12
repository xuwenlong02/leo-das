#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <error.h>
#include <net/route.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <resolv.h>
#include <ifaddrs.h>
#include "ifconfig.h"
 
int SetIfAddr(const char *ifname,const char *Ipaddr,const char *mask,const char *gateway)
{
    int fd;
    int rc;
    struct ifreq ifr; 
    struct sockaddr_in *sin;
    struct rtentry  rt;
    
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
    {
        perror("socket   error");
        return fd;
    }
    memset(&ifr,0,sizeof(ifr)); 
    strcpy(ifr.ifr_name,ifname); 
    sin = (struct sockaddr_in*)&ifr.ifr_addr;     
    sin->sin_family = AF_INET;  
   
    //ipaddr
    if((rc = inet_aton(Ipaddr,&(sin->sin_addr))) < 0)
    {     
        perror("inet_aton   error");     
        close(fd);
        return rc;
    }    
    
    if((rc = ioctl(fd,SIOCSIFADDR,&ifr)) < 0)
    {     
        perror("ioctl   SIOCSIFADDR   error");     
        close(fd);
        return rc;
    }

    //netmask
    if((rc = inet_aton(mask,&(sin->sin_addr))) < 0)
    {     
        perror("inet_pton   error");     
        close(fd);
        return rc;
    }    
    if((rc = ioctl(fd, SIOCSIFNETMASK, &ifr)) < 0)
    {
        perror("ioctl");
        close(fd);
        return rc;
    }

    //gateway
    memset(&rt, 0, sizeof(struct rtentry));
    memset(sin, 0, sizeof(struct sockaddr_in));
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    if((rc = inet_aton(gateway, &sin->sin_addr))<0)
    {
       printf ( "inet_aton error\n" );
    }
    memcpy ( &rt.rt_gateway, sin, sizeof(struct sockaddr_in));
    ((struct sockaddr_in *)&rt.rt_dst)->sin_family=AF_INET;
    ((struct sockaddr_in *)&rt.rt_genmask)->sin_family=AF_INET;
    rt.rt_flags = RTF_GATEWAY;
    if ((rc = ioctl(fd, SIOCADDRT, &rt))<0)
    {
        printf( "ioctl(SIOCADDRT) error in set_default_route\n");
        close(fd);
        return rc;
    }
    close(fd);
    return 0;
}

/*
 * 获取IP
 */
int GetIfAddr(const char* iface,char ipaddr[],char mask[])
{
    int sockfd;
    int rc;
    struct   sockaddr_in *sin;
    struct   ifreq ifr_ip;
    struct   rtentry rt;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
         printf("socket create failse...GetLocalIp!/n");
         return sockfd;
    }

    /* zero struct */
    memset(&ifr_ip, 0, sizeof(ifr_ip));
    strncpy(ifr_ip.ifr_name, iface, sizeof(ifr_ip.ifr_name) - 1);

    /* get ip */
    if((rc =  ioctl( sockfd, SIOCGIFADDR, &ifr_ip)) < 0 )
    {
        close(sockfd );
        return rc;
    }
    sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;
    strcpy(ipaddr,inet_ntoa(sin->sin_addr));

    /* get netmask */
    if( (rc = ioctl( sockfd, SIOCGIFNETMASK, &ifr_ip ) ) < 0 )
    {
        printf("mac ioctl error/n");
        close(sockfd );
        return rc;
    }
    sin = ( struct sockaddr_in * )&( ifr_ip.ifr_netmask );
    strcpy( mask, inet_ntoa( sin-> sin_addr));

    close(sockfd );

    return rc;
}

/*
 * 获取IP
 */
bool GetLocalIp(char ipaddr[])
{
    return GetLocalIp("eth0",ipaddr);
}

bool GetLocalIp(const char* iface,char ipaddr[])
{  
    int sock_get_ip;  
  
    struct   sockaddr_in *sin;  
    struct   ifreq ifr_ip;     
  
    if ((sock_get_ip=socket(AF_INET, SOCK_STREAM, 0)) == -1)  
    {  
         printf("socket create failse...GetLocalIp!/n");  
         return false;  
    }  
     
    memset(&ifr_ip, 0, sizeof(ifr_ip));     
    strncpy(ifr_ip.ifr_name, iface, sizeof(ifr_ip.ifr_name) - 1);
   
    if( ioctl( sock_get_ip, SIOCGIFADDR, &ifr_ip) < 0 )     
    {     
         return false;     
    }       
    sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;     
    strcpy(ipaddr,inet_ntoa(sin->sin_addr));         
      
    printf("local ip:%s /n",ipaddr);      
    close( sock_get_ip );  
      
    return true;  
} 

/*
 * 修改本机IP地址的函数
 */
int SetLocalIp(const char *iface, const char *ipaddr )
{  
    int sock_set_ip;  
      
    struct sockaddr_in sin_set_ip;  
    struct ifreq ifr_set_ip;  
  
    bzero( &ifr_set_ip,sizeof(ifr_set_ip));  
   
    if( ipaddr == NULL )  
        return -1;  
  
    if(sock_set_ip = socket( AF_INET, SOCK_STREAM, 0 ) == -1);  
    {  
        perror("socket create failse...SetLocalIp!/n");  
        return -1;  
    }  
   
    memset( &sin_set_ip, 0, sizeof(sin_set_ip));  
    strncpy(ifr_set_ip.ifr_name, iface, sizeof(ifr_set_ip.ifr_name)-1);
      
    sin_set_ip.sin_family = AF_INET;  
    sin_set_ip.sin_addr.s_addr = inet_addr(ipaddr);  
    memcpy( &ifr_set_ip.ifr_addr, &sin_set_ip, sizeof(sin_set_ip));  
  
    if( ioctl( sock_set_ip, SIOCSIFADDR, &ifr_set_ip) < 0 )  
    {  
        perror( "Not setup interface/n");  
        return -1;  
    }  
  
    //设置激活标志  
    ifr_set_ip.ifr_flags |= IFF_UP |IFF_RUNNING;  
  
    //get the status of the device  
    if( ioctl( sock_set_ip, SIOCSIFFLAGS, &ifr_set_ip ) < 0 )  
    {  
         perror("SIOCSIFFLAGS");  
         return -1;  
    }  
  
    close( sock_set_ip );  
    return 0;  
} 

/*
 *获取子网掩码的函数
 */
bool GetLocalNetMask(const char *ifname,char netmask_addr[])
{  
    int sock_netmask;
  
    struct ifreq ifr_mask;  
    struct sockaddr_in *net_mask;  
          
    sock_netmask = socket( AF_INET, SOCK_STREAM, 0 );  
    if( sock_netmask == -1)  
    {  
        perror("create socket failture...GetLocalNetMask/n");  
        return false;  
    }  
      
    memset(&ifr_mask, 0, sizeof(ifr_mask));     
    strncpy(ifr_mask.ifr_name, ifname, sizeof(ifr_mask.ifr_name )-1);     
  
    if( (ioctl( sock_netmask, SIOCGIFNETMASK, &ifr_mask ) ) < 0 )   
    {  
        printf("mac ioctl error/n");  
        return false;  
    }  
      
    net_mask = ( struct sockaddr_in * )&( ifr_mask.ifr_netmask );  
    strcpy( netmask_addr, inet_ntoa( net_mask -> sin_addr ) );  
      
    printf("local netmask:%s/n",netmask_addr);      
      
    close( sock_netmask );  
    return true;  
}  

/*
 * 修改子NETMASK的函数
 */
bool SetLocalNetMask(const char *iface,const char *szNetMask)
{  
    int sock_netmask;  
    char netmask_addr[32];     
  
    struct ifreq ifr_mask;  
    struct sockaddr_in *sin_net_mask;  
          
    sock_netmask = socket( AF_INET, SOCK_STREAM, 0 );  
    if( sock_netmask == -1)  
    {  
        perror("Not create network socket connect/n");  
        return false;  
    }  
      
    memset(&ifr_mask, 0, sizeof(ifr_mask));     
    strncpy(ifr_mask.ifr_name, iface, sizeof(ifr_mask.ifr_name )-1);
    sin_net_mask = (struct sockaddr_in *)&ifr_mask.ifr_addr;  
    sin_net_mask -> sin_family = AF_INET;  
    inet_pton(AF_INET, szNetMask, &sin_net_mask ->sin_addr);  
  
    if(ioctl(sock_netmask, SIOCSIFNETMASK, &ifr_mask ) < 0)   
    {  
        printf("sock_netmask ioctl error/n");  
        return false;  
    }  
	return true;
}  

/*
 * 获去GateWay
 */
bool GetGateWay(char gateway[])  
{  
    FILE *fp;  
    char buf[512];  
    char cmd[128];  
  
    strcpy(cmd, "route -n|grep 'UG'");
    fp = popen(cmd, "r");  
    if(NULL == fp)  
    {  
        perror("popen error");  
        return false;  
    }  

    size_t r=fread(buf,1,sizeof(buf),fp);
    sscanf(buf,"%*s%s",gateway);
    pclose(fp);  
      
    return true;  
}  

/*
 * 设置网关
 */
int SetGateWay(const char *szGateWay)  
{  
    int ret = 0;      
    char cmd[128];
    char strGW[50];

    GetGateWay(strGW);
      
    strcpy(cmd, "route del default gw ");  
    strcat(cmd, strGW);  
    ret = system(cmd);  
    if(ret < 0)  
    {  
        perror("route error");  
        return -1;  
    }  
    strcpy(cmd, "route add default gw ");  
    strcat(cmd, szGateWay);  
      
    ret = system(cmd);  
    if(ret < 0)  
    {  
        perror("route error");  
        return -1;  
    }  
  
    return ret;  
} 

/*
 * 获取系统中的网卡数量
 */
int GetNetCardCount(NetParm *ifnames)
{
    int nCount = 0;
    FILE* f = fopen("/proc/net/dev", "r");
    if (!f)
    {
        fprintf(stderr, "Open /proc/net/dev failed!errno:%d\n", errno);
        return nCount;
    }

    char szLine[512];
    char *p =fgets(szLine, sizeof(szLine), f);    /* eat line */
    p = fgets(szLine, sizeof(szLine), f);

    while(fgets(szLine, sizeof(szLine), f))
    {
        char szName[128] = {0};
        sscanf(szLine, "%s", szName);
        int nLen = strlen(szName);
        if (nLen <= 0)continue;
        if (szName[nLen - 1] == ':') szName[nLen - 1] = 0;
        if (strcmp(szName, "lo") == 0)continue;
        if (ifnames)
            strcpy(ifnames[nCount],szName);
        nCount++;
    }

    fclose(f);
    f = NULL;
    return nCount;
}

/*
 *获取本机MAC地址函数
 */
bool GetLocalMac(char mac_addr[30])  
{  
    int sock_mac;  
    struct ifreq ifr_mac;   
      
    sock_mac = socket( AF_INET, SOCK_STREAM, 0 );  
    if( sock_mac == -1)  
    {  
        perror("create socket falise...mac/n");  
        return false;  
    }  
      
    memset(&ifr_mac,0,sizeof(ifr_mac));     
    strncpy(ifr_mac.ifr_name, "eth0", sizeof(ifr_mac.ifr_name)-1);     
  
    if( (ioctl( sock_mac, SIOCGIFHWADDR, &ifr_mac)) < 0)  
    {  
        printf("mac ioctl error/n");  
        return false;  
    }  
      
    sprintf(mac_addr,"%02X:%02X:%02X:%02X:%02X:%02X",
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[0],  
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[1],  
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[2],  
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[3],  
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],  
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]);  
  
    printf("local mac:%s /n",mac_addr);      
      
    close( sock_mac );  
    return true;  
}  

#if 0
int main(int argc,const char* argv[])
{
	res_init();
	for (int i = 0; i < _res.nscount; i++){
		struct sockaddr* server = (struct sockaddr*)&_res.nsaddr_list[i];
		printf("Server:  %s\n", inet_ntoa(*(in_addr*)&(server->sa_data[2])));
	}
	
	struct ifaddrs        *ifc, *ifc1; 
    char                ip[64]; 
    char                nm[64]; 

    if (0 != getifaddrs(&ifc)) return(-1); 
    ifc1 = ifc; 
    printf("Iface\tIP address\tNetmask\n"); 
    for(; NULL != ifc; ifc = (*ifc).ifa_next) { 
        printf("%s", (*ifc).ifa_name); 
        if (NULL != (*ifc).ifa_addr) { 
                inet_ntop(AF_INET, &(((struct sockaddr_in*)((*ifc).ifa_addr))->sin_addr), ip, 64); 
                printf("\t%s", ip); 
        } else { 
                printf("\t\t"); 
        } 
        if (NULL != (*ifc).ifa_netmask) { 
                inet_ntop(AF_INET, &(((struct sockaddr_in*)((*ifc).ifa_netmask))->sin_addr), nm, 64); 
                printf("\t%s", nm); 
        } else { 
                printf("\t\t"); 
        } 
        printf("\n"); 
    } 
    freeifaddrs(ifc1); 

    printf("netcard count %d\n",GetNetCardCount());
	
	char macaddr[30];
	GetLocalMac(macaddr);
	printf("mac %s\n",macaddr);
	
	char gateway[30];
	GetGateWay(gateway);
	printf("gateway %s\n",gateway);
	
	char netmask[50];
    GetLocalNetMask("eth0",netmask);
    printf("netmask %s\n",netmask);
	
	char ipaddr[50];
	GetLocalIp(ipaddr);
	printf("ip address %s\n",ipaddr);
}
#endif

 
