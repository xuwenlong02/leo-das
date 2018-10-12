/*************************************************************************
	> File Name: main.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月17日 星期一 14时03分04秒
 ************************************************************************/
#include "exchange.h"
#include <logger.h>
#include <utils/signals.h>

void sighandler(int sig);

int main(int argc, char *argv[])
{
//    LOG_ENABLE(false);

    Exchange ex;
    ex.StartServer();

    utils::WaitTerminationSignals(sighandler);

    return 0;
}

void sighandler(int sig)
{

}
