#include "logger.hpp"
#include <unistd.h>

int main()
{
    std::thread t1([]()
    {
        int i=0;
        while(i<10)
        {
            usleep(100);
            LOG_INFO("this is t1 i = %d",i);
            ++i;
        }
    });
    int j=0;
    
    while(j<10)
    {
        usleep(100);
        LOG_ERR("this is main  j = %d",j);
        ++j;
    }
    t1.join();
    return 0;
}