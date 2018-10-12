/*************************************************************************
    > File Name: CustomServer.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月17日 星期一 14时00分18秒
 ************************************************************************/

#include <iostream>
#include <sys/select.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <string.h>
#include <event.h>
#include <stdlib.h>
#include <event2/rpc.h>
#include <event2/listener.h>
#include <event2/thread.h>

#include "customserver.h"
#include "custom.h"
#include "exchange.h"
#include <logger.h>

CustomServer::CustomServer(Exchange *ex):
    Server(),
    m_pEx(ex)
{
}

CustomServer::~CustomServer()
{
}

Exchange *CustomServer::Ex()
{
    return m_pEx;
}

tUser CustomServer::CreateUser()
{
    return new Custom(this);
}
