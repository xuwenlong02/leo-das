/*************************************************************************
	> File Name: watchdog.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月21日 星期五 10时45分12秒
 ************************************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#if defined(HTNICE_K4) || defined(HTNICE_OLDK4)
#define FEED_WATCHDOG	{char buf = '1'; write(fdw,&buf,1);}while(0)
#define CLOSE_WDT {char buf = 'V'; write(fdw,&buf,1);close(fdw);}while(0)
#elif defined(HTNICE_K2)
#include <linux/watchdog.h>
#define	WDIOC_SETMSTIMER	0xfedcba
#define	WDIOC_GETMSTIMER	0xfedcb9

int timer_val,wdt_val;
char buf;

#define REBOOTTIME	10	//¿ŽÃÅ¹·žŽÎ»Ê±Œä
#define ENABLE_WDT	timer_val = (REBOOTTIME+1)*1000; wdt_val = REBOOTTIME; ioctl(fdw,WDIOC_SETTIMEOUT,&wdt_val); ioctl(fdw,WDIOC_SETMSTIMER,&timer_val)	//¿ªÆô¿ŽÃÅ¹·
#define FEED_WATCHDOG	ioctl(fdw,WDIOC_KEEPALIVE,0)	//Î¹¹·
#define CLOSE_WDT buf = 'V'; write(fdw,&buf,1);close(fdw)	//¹Ø±Õ¿ŽÃÅ¹·
#else
#define FEED_WATCHDOG	{}while(0)
#define CLOSE_WDT {}while(0)
#endif

int fdw = -1;

void OpenDog() {
    if((fdw=open("/dev/watchdog",O_RDWR))==-1)
    {
        printf("Can not open /dev/watchdog\n");
//        exit(1);
        return;
    }
#ifdef HTNICE_K2
    ENABLE_WDT;
#endif
}

void FeedDog() {
    if (fdw!=-1) {
        FEED_WATCHDOG;
    }
}

void CloseDog() {
    CLOSE_WDT;
}
