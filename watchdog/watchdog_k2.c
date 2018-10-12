#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>
#include "check_process.h"

#define	WDIOC_SETMSTIMER	0xfedcba
#define	WDIOC_GETMSTIMER	0xfedcb9

int timer_val,wdt_val;
char buf;

#define REBOOTTIME	10	//看门狗复位时间
#define ENABLE_WDT	timer_val = (REBOOTTIME+1)*1000; wdt_val = REBOOTTIME; ioctl(fdw,WDIOC_SETTIMEOUT,&wdt_val); ioctl(fdw,WDIOC_SETMSTIMER,&timer_val)	//开启看门狗
#define FEED_WATCHDOG	ioctl(fdw,WDIOC_KEEPALIVE,0)	//喂狗
#define CLOSE_WDT buf = 'V'; write(fdw,&buf,1);close(fdw)	//关闭看门狗


int main(int argc, char **argv)
{       

	int fdw;
	
	if((fdw=open("/dev/watchdog",O_RDWR))==-1)
	{
		printf("Can not open /dev/watchdog\n");
		exit(1);
	}

	printf("enable wdt!\n");		
	ENABLE_WDT;

	while(1){
		sleep(1);
        if (isRunning("leo-das") == 0)
            FEED_WATCHDOG;
        else{
            printf("disable wdt!\n");
            CLOSE_WDT;
            return 0;
        }
	}

	printf("disable wdt!\n");
	CLOSE_WDT;      
	return 0;

}
