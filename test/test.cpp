/*************************************************************************
	> File Name: test.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年04月27日 星期五 10时47分28秒
 ************************************************************************/

#include <acquisition/acquisition.h>

void testTemplate(const char* templete,const unsigned char cmd[],int len);

void test() {
    unsigned char cmd[] = {0x89,0x03,0x04,0x43,0x0B,0xF3,0x33,0x92,0x98};
    int len = 9;
    const char * temp = "";
    testTemplate(temp,cmd,len);
}

void testTemplate(const char* templete,const unsigned char cmd[],int len)
{

}
