/*************************************************************************
	> File Name: shellcmd.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月22日 星期一 14时40分36秒
 ************************************************************************/

#include <string>
#include <defines.h>

typedef bool (*shell_runcallback)(const char *line,void *data);

class ShellCmd
{
public:
    ShellCmd(shell_runcallback callback_);
    ShellCmd();
    S32 RunCmd(const S8 *cmd,void *data);
    std::string ShellGet(const S8 *cmd);
    S32 ShellSet(const S8 *cmd);
    static void result(const char *line);
private:
    shell_runcallback callback;
};
