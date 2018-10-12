/*************************************************************************
	> File Name: testshareptr.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月30日 星期日 10时28分44秒
 ************************************************************************/
#include <utils/shared_ptr.h>
#include <logger.h>

class testShare:public utils::RefrenceObj
{
public:
    testShare() {LOG_INFO("enter,%d",refenceCount());}
    virtual ~testShare() {LOG_INFO("exit,%d",refenceCount());}
};

int main(int argc,char *argv[])
{
    testShare *test = new testShare();
    utils::SharedPtr<testShare> test1 = test;
    utils::SharedPtr<testShare> test2 = test1;
    utils::SharedPtr<testShare> test3 = test;

    return 0;
}
