#include "master.h"
#include "buttonrpc.hpp"
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unistd.h>

Master::Master()
{
    isInit = false;
    rpcServer_ = make_unique<buttonrpc>();
}

Master *Master::GetInstance()
{
    static Master instance;
    return &instance;
}

// 初始化
void Master::Init(int argc, char **argv, int reducenum = 5, short port = 5555)
{
    if (isInit)
    {
        std::cout << "this master is INITED, don't redo it !" << std::endl;
        return;
    }
    if (argc < 2)
    {
        std::cout
            << "please input your task filename, format: command file1 file2 "
               "..."
            << std::endl;
        exit(EXIT_FAILURE);
    }
    isInit = true;
    finishReduceTaskNum_ = 0;
    reduceNum_ = reducenum;

    mapTaskLists_.clear();
    fininshMapTasks_.clear();
    reduceTaskLists_.clear();
    reduceVec_.assign(reduceNum_, false);

    // 通过传入argc，argv来初始化map列表

    mapNum_ = argc - 1;
    // 初始化map任务列表
    for (int i = 1; i < argc; ++i)
    {
        mapTaskLists_.emplace_back(argv[i]);
    }

    // 初始化reduce任务
    for (int i = 0; i < reduceNum_; ++i)
    {
        reduceTaskLists_.push_back(i);
    }

    // 服务器模块初始化rpc服务器
    rpcServer_->as_server(port);
    rpcServer_->bind("getMapId", &Master::getMapId, this);
    rpcServer_->bind("assignMapTask", &Master::assignMapTask, this);
    rpcServer_->bind("setMapsuccess", &Master::setMapsuccess, this);
    rpcServer_->bind("assignReduceTask", &Master::assignReduceTask, this);
    rpcServer_->bind("getReduceNum", &Master::getReduceNum, this);
    rpcServer_->bind("setReducesuccess", &Master::setReducesuccess, this);
    rpcServer_->bind("isMapDone", &Master::isMapDone, this);
    rpcServer_->bind("Done", &Master::Done, this);
}

// 分配任务
std::string Master::assignMapTask()
{
    if (mapTaskLists_.empty())
    {
        return "empty";
    }
    std::string Taskname;
    {
        std::lock_guard<std::mutex> lock(taskLock_);
        Taskname = mapTaskLists_.front();
        mapTaskLists_.pop_front();
    }
    std::thread t1(std::bind(&Master::waitMapTask, this, Taskname));
    t1.detach();
    return Taskname;
}

// map任务定时，超时便重新加入工作队列
void Master::waitMapTask(std::string taskName)
{
    sleep(maxMapTime);
    if (!fininshMapTasks_.count(taskName))
    {
        std::lock_guard<std::mutex> lock(taskLock_);
        mapTaskLists_.emplace_front(taskName);
    }
}

// 设置map任务成功
bool Master::setMapsuccess(std::string taskName)
{
    if (!fininshMapTasks_.count(taskName))
    {
        std::lock_guard<std::mutex> lock(taskLock_);
        fininshMapTasks_.emplace(taskName);
        return true;
    }
    return false;
}

int Master::assignReduceTask()
{
    if (!isMapDone())
    {
        return -2;
    }
    if (reduceTaskLists_.empty())
    {
        return -1;
    }
    int k = reduceTaskLists_.front();
    {
        std::lock_guard<std::mutex> lock(taskLock_);
        reduceTaskLists_.pop_front();
    }
    std::thread t1(std::bind(&Master::waitReduceTask, this, k));
    t1.detach();
    return k;
}
bool Master::setReducesuccess(int k)
{
    if (!reduceVec_[k])
    {
        finishReduceTaskNum_++;
        reduceVec_[k] = true;
        return true;
    }
    return false;
}

void Master::waitReduceTask(int k)
{
    sleep(maxReduceTime);
    if (!reduceVec_[k])
    {
        lock_guard<mutex> lock(taskLock_);
        reduceTaskLists_.push_front(k);
    }
}

void Master::start()
{
    if (!isInit)
    {
        std::cout << "not Init, start fail!" << std::endl;
        exit(EXIT_FAILURE);
    }
    rpcServer_->run();
}
