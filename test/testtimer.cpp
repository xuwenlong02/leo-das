/*************************************************************************
	> File Name: testtimer.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年09月30日 星期日 10时28分28秒
 ************************************************************************/
#include <utils/timer.h>
#include <utils/timer_task.h>
#include <logger.h>

class Test:public TimerTask
{
public:
    Test(const std::string &name,Milliseconds timeout,const TimerType timer_type) {
        time_ = 0;
        timer_ = new Timer(name,this);
        timeout_= timeout;
        timer_type_=timer_type;
    }

    ~Test() {
        delete timer_;
    }

    virtual void RunTimer() {
//        LOG_INFO("%d,%s",++time,timer_->timername().c_str());
        fprintf(stdout,"%d,%s:%s\n" ,
                ++time_,utils::nowtostr().c_str(),timer_->timer_name().c_str());
        fflush(stdout);
    }

    void Start() {
        time_ = 0;
        timer_->Start(timeout_,timer_type_);
    }

    void Stop() {
        timer_->Stop();
    }

private:
    Timer *timer_;
    int time_;
    Milliseconds timeout_;
    TimerType timer_type_;
};


#if 0
int main(int argc,char *argv[])
{
    Test *test1 = new Test("single",1000,kSingleShot);
    Test *test2 = new Test("period",500,kPeriodic);
//    test1->Start();
//    test2->Start();
    int timeout[3] = {0,1,2};
    for (int i = 0;i<16;i++) {
        test1->Start();
        sleep(timeout[i%3]);
//        test1->Stop();
    }

    for (int i = 0;i<5;i++) {
        test2->Start();
        sleep(i);
    }
    test2->Stop();
//    sleep(5);
//    if(!utils::WaitTerminationSignals(&sig_handler)){
//        LOG_ERR("failed to catch signal");
//    }

    delete test1;
    delete test2;
    return 0;
}
#endif
