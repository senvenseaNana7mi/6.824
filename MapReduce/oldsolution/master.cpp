#include "master.hpp"
#include <cstdio>
#include <exception>
#include <mutex>
#include <thread>

#define MAP_TASK_TIMEOUT 3
#define REDUCE_TASK_TIMEOUT 5

//用于定时，会创建线程睡眠，不明所以？？，根据是map还是reduce调用定时线程，定时不同的时间
void Master::waitTime(char* op)
{
    if(*op == 'm'){
        sleep(MAP_TASK_TIMEOUT);
    }else if(*op == 'r'){
        sleep(REDUCE_TASK_TIMEOUT);
    }
} 


Master::Master(int mapNum, int reduceNum)
    : m_done(false), m_mapNum(mapNum), m_reduceNum(reduceNum)
{
    m_list.clear();
    finishedMapTask.clear();
    finishedReduceTask.clear();
    runningMapWork.clear();
    runningReduceWork.clear();
    curMapIndex = 0;
    curReduceIndex = 0;
    if (m_mapNum <= 0 || m_reduceNum <= 0)
    {
        throw std::exception();
    }
    for (int i = 0; i < reduceNum; ++i)
    {
        reduceIndex.emplace_back(i);
    }
}

void Master::GetAllFile(char *file[], int argc)
{
    for (int i = 1; i < argc; ++i)
    {
        m_list.emplace_back(file[i]);
    }
    fileNum = argc - 1;
}

//map的worker只需要拿到对应的文件名就可以进行map
string Master::assignTask()
{
    if(isMapDone())
    {
        return "empty";
    }
    if(!m_list.empty())
    {
        char *task = nullptr;  
        {
            lock_guard<mutex> lock(m_assign_lock);
            //从工作队列中去除一个待map的文件名
            task = m_list.back(); 
            m_list.pop_back();
        }
        waitMap(string(task));            //调用waitMap将取出的任务加入正在运行的map任务队列并等待计时线程
        return string(task);
    }
    return "empty";
}


//定时处理任务
void Master::waitMapTask(Master *master)
{
    thread t1(waitTime,'m');
    t1.join();
    lock_guard<mutex> lock(master->m_assign_lock);
    //当前任务未处理完，重新加入队列
    if(!master->finishedMapTask.count(master->runningMapWork[master->curMapIndex]))
    {
        printf("filename : %s is timeout\n", master->runningMapWork[master->curMapIndex].c_str());
        const char *text = master->runningMapWork[master->curMapIndex].c_str(); //加入list，等待重新分配
        master->m_list.push_back((char*)text);
        master->curMapIndex++;
        return ;
    }
    printf("filename : %s is finished\n", master->runningMapWork[master->curMapIndex].c_str());
    master->curMapIndex++;
} 

//在分配任务时调用
void Master::waitMap(string filename)
{
    {
        lock_guard<mutex> lock(m_assign_lock);
        runningMapWork.push_back(filename);
    }
    thread t1(waitMapTask,this);
    t1.detach();
}



int Master::assignReduceTask()
{
    if(Done())return -1;
    if(!reduceIndex.empty())
    {
        int reduceIdx = 0;
        {
            lock_guard<mutex> lock(m_assign_lock);
            reduceIdx = reduceIndex.back();
            reduceIndex.pop_back();
        }
        waitReduce(reduceIdx);
        return reduceIdx;
    }
    return -1;
}

void Master::waitReduceTask(Master *master)
{
    thread t1(waitTime, 'r');
    t1.join();
    lock_guard<mutex> lock(master->m_assign_lock);
    //若超时后在对应的hashmap中没有该reduce任务完成的记录，将该任务重新加入工作队列
    if(!master->finishedReduceTask.count(master->runningReduceWork[master->curReduceIndex]))
    {
        for(auto a : master->m_list)printf(" before insert %s\n", a);
        master->reduceIndex.emplace_back(master->runningReduceWork[master->curReduceIndex]);
        printf(" reduce%d is timeout\n", master->runningReduceWork[master->curReduceIndex]);
        master->curReduceIndex++;
        for(auto a : master->m_list) printf(" after insert %s\n", a);
        return ;
    }
    printf("%d reduce is completed\n", master->runningReduceWork[master->curReduceIndex]);
    master->curReduceIndex++;
}

void Master::waitReduce(int reduceIdx)
{
    {    
        lock_guard<mutex> lock(m_assign_lock);
        runningReduceWork.push_back(reduceIdx);
    }
    thread t1(waitReduceTask, this);
    t1.detach();
}


void Master::setMapStat(string filename)
{
    lock_guard<mutex> lock(m_assign_lock);
    finishedMapTask[filename] = 1;  //通过调用worer的RPC调用修改map任务的完成状态
}

bool Master::isMapDone()
{
    lock_guard<mutex> lock(m_assign_lock);
    return finishedMapTask.size() == fileNum;
}


void Master::setReduceStat(int taskIndex)
{
    lock_guard<mutex> lock(m_assign_lock);
    finishedReduceTask[taskIndex] = 1; //通过worker的RPC调用修改reduce任务的完成状态
    m_assign_lock.unlock();
    return ;

}

bool Master::Done()
{
    lock_guard<mutex> lock(m_assign_lock);
    return finishedReduceTask.size()==m_reduceNum;
}

int main(int argc, char* argv[])
{
    if(argc < 2){
        cout<<"missing parameter! The format is ./Master pg*.txt"<<endl;
        exit(-1);
    }
    MprpcApplication::Init(argc, argv);
    RpcProvider provider;
    auto master = new Master;
    master->GetAllFile(argv+1, argc-1);
    provider.NotifyService(master);

    //启动rpc发布
    provider.Run();
}