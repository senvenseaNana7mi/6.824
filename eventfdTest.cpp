#include <cstdio>
#include <cstdlib>
#include<iostream>
using namespace std;
#include<sys/eventfd.h>
#include<thread>
#include<unistd.h>


int main()
{
    int evfd = eventfd(0, EFD_SEMAPHORE);
    for(int i =0;i<3;++i)
    {
        thread t1([evfd]()
        {
            sleep(5);
            eventfd_t value;
            int  ret = eventfd_read(evfd, &value);
            if(ret == -1)
            {
                perror("eventfd_read");
                exit(EXIT_FAILURE);
            }
            cout<< this_thread::get_id()<<" recv value: "<<value<<endl;
        });
        t1.detach();
    }
    thread t2([evfd]()
    {
        eventfd_t value = 1;
        eventfd_write(evfd, value);
    });
    t2.join();
    eventfd_t value;
    int  ret = eventfd_read(evfd, &value);
    if(ret!=-1)
    {
        cout<<value<<endl;
    }
    sleep(10);
    return 0;
}