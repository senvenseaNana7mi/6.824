#include <bits/types/time_t.h>
#include <chrono>
#include<thread>
#include<iostream>
using namespace std;
#include<mutex>
#include<condition_variable>
#include<time.h>
#include<unistd.h>
std::mutex m_mutex;
std::condition_variable cond;

void func()
{
    time_t nowtm=time(NULL);
    tm* t=localtime(&nowtm);
    std::cout<<t->tm_min<<":"<<t->tm_sec<<endl;
}

void timefunc()
{
    while(true)
    {
        unique_lock<std::mutex> lock(m_mutex);
        // thread t1([]
        // {
        //     while(1);
        // });
        // t1.detach();
        cond.wait(lock);
        func();
    }

}

int main()
{
    timefunc();
    return 0;
}