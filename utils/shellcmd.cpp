/*************************************************************************
	> File Name: shellcmd.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月22日 星期一 14时40分20秒
 ************************************************************************/

#include "shellcmd.h"
#include <stdio.h>
#include <logger.h>
#include <errno.h>

ShellCmd::ShellCmd(shell_runcallback callback_):
    callback(callback_)
{
}

ShellCmd::ShellCmd()
{
}

S32 ShellCmd::RunCmd(const S8 *cmd, void *data)
{
    FILE *fp;
    char buffer[256] = {0};
    char shell[256] = {0};

    sprintf(shell,"sh %s",cmd);
    LOG_INFO("shell:%s",shell);

    fp = popen(shell,"r");
    if (!fp) {
        LOG_ERR("popen faild,%s",strerror(errno));
        return -1;
    }

    while(fgets(buffer,sizeof(buffer),fp) != NULL){
        LOG_INFO("%s",buffer);
        if (!callback(buffer,data))
            break;
    }
    return pclose(fp);
}

std::string ShellCmd::ShellGet(const S8 *cmd)
{
    FILE *fp;
    std::string result;
    char buffer[256] = {0};

    LOG_INFO("shell:%s",cmd);

    fp = popen(cmd,"r");

    if (!fp) {
        LOG_ERR("popen faild,%s",strerror(errno));
        return result;
    }
    while(fgets(buffer,sizeof(buffer),fp) != NULL) {
        LOG_DEBUG("%s",buffer);
        result += buffer;
    }
    pclose(fp);
    return result;
}

S32 ShellCmd::ShellSet(const S8 *cmd)
{
    FILE *fp;
    char buffer[256] = {0};

    LOG_INFO("shell:%s",cmd);

    fp = popen(cmd,"r");

    if (!fp) {
        LOG_ERR("popen faild,%s",strerror(errno));
        return -1;
    }

    while(fgets(buffer,sizeof(buffer),fp) != NULL) {
        LOG_INFO("%s",buffer);
    }
    return pclose(fp);
}

