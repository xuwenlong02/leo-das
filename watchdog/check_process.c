/*************************************************************************
	> File Name: check_process.c
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年07月27日 星期五 17时02分56秒
 ************************************************************************/
#include "check_process.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>

char* getPidFromStr(const char *str)
{
    static char sPID[16] = {0};
    int tmp = 0;
    int pos1 = 0;
    int pos2 = 0;
    int i = 0;
    int j = 0;

    for (i=0; i<strlen(str); i++) {
        if ( (tmp==0) && (str[i]>='0' && str[i]<='9') ) {
            tmp = 1;
            pos1 = i;
        }
        if ( (tmp==1) && (str[i]<'0' || str[i]>'9') ) {
            pos2 = i;
            break;
        }
    }
    for (j=0,i=pos1; i<pos2; i++,j++) {
        sPID[j] = str[i];
    }
    return sPID;
}


int isRunning(const char *proc_name)
{
#if 0
    int ret = 0;
    char *sCurrPid;

    sCurrPid = getPidFromStr(proc_name);

    printf("%s pid = %s\n",proc_name,sCurrPid);

//    sprintf(sCurrPid, "%d", pid);

    FILE *fstream=NULL;
    char buff[1024] = {0};
    if(NULL==(fstream=popen("ps -e -o pid,comm | grep a.out | grep -v PID | grep -v grep", "r")))
    {
        fprintf(stderr,"execute command failed: %s", strerror(errno));
        return -1;
    }
    while(NULL!=fgets(buff, sizeof(buff), fstream)) {
        char *oldPID = getPidFromStr(buff);
        if ( strcmp(sCurrPid, oldPID) != 0 ) {
            printf("程序已经运行，PID=%s\n", oldPID);
            ret = 1;
        }
    }
    pclose(fstream);

    return ret;
#else
    FILE *fp = NULL;
    int ret = 1;
    char buff[200] = {0};

    fp = popen("ps l|grep '/leo-das/bin/leo-das'|grep -v 'grep'|wc -l","r");
    if (!fp)
    {
        printf("shell failed!");
        return 1;
    }
    fgets(buff,sizeof(buff),fp);
    printf(buff);

    pclose(fp);
    ret = strncmp(buff,"1",1);
    printf("ret = %d\n",ret);
    return ret;
#endif
}
