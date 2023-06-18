#pragma once 

#include <exception>
#include<mutex>
#include<semaphore.h>

class sem{
public:
    sem()
    {
        if(sem_init(&m_sem, 0, 0)!=0)
        {
            throw std::exception();
        }
    }
    sem(int num)
    {
        if(sem_init(&m_sem, 0, num)!=0)
        {
            throw std::exception();
        }
    }
    bool post()
    {
        return sem_post(&m_sem)==0;
    }
    bool wait()
    {
        return sem_wait(&m_sem)==0;
    }
    ~sem()
    {
        sem_destroy(&m_sem);
    }
private:
    sem_t m_sem;
};