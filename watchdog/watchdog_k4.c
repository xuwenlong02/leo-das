#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "check_process.h"

#define FEED_WATCHDOG	{char buf = '1'; write(fdw,&buf,1);}while(0)	//Œππ∑
#define CLOSE_WDT {char buf = 'V'; write(fdw,&buf,1);close(fdw);}while(0)	//πÿ±’ø¥√≈π∑

int main(int argc, char **argv)
{       

	int fdw;
	
	if((fdw=open("/dev/watchdog",O_RDWR))==-1)
	{
		printf("Can not open /dev/watchdog\n");
		exit(1);
	}

	printf("enable wdt!\n");		

    while(1){
        sleep(1);
        if (isRunning("leo-das") == 0){
            FEED_WATCHDOG;
        }
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
